#include "kernel.h"

static int mouse_x = 40;
static int mouse_y = 12;
static uint8_t mouse_cycle = 0;
static char mouse_packet[3];

// Draws a simple on-screen cursor tracking variable changes
void draw_mouse_cursor(int x, int y) {
    char* vga = (char*) 0xB8000;
    // Restore the screen character block layout or render a special color block
    int index = (y * 80 + x) * 2;
    vga[index + 1] = 0x70; // Inverse color background creates an active block cursor pointer
}

void mouse_wait(uint8_t a_type) {
    uint32_t timeout = 100000;
    if (a_type == 0) {
        while (timeout--) { if ((inb(0x64) & 1) == 1) return; }
    } else {
        while (timeout--) { if ((inb(0x64) & 2) == 0) return; }
    }
}

void mouse_write(uint8_t a_write) {
    mouse_wait(1);
    outb(0x64, 0xD4);
    mouse_wait(1);
    outb(0x60, a_write);
}

void mouse_init(void) {
    uint8_t status;
    mouse_wait(1);
    outb(0x64, 0xA8); // Enable auxiliary mouse device channel
    
    mouse_wait(1);
    outb(0x64, 0x20);
    mouse_wait(0);
    status = (inb(0x60) | 2);
    mouse_wait(1);
    outb(0x64, 0x60);
    mouse_wait(1);
    outb(0x60, status);
    
    mouse_write(0xF4); // Enable packet stream data streaming
    mouse_wait(0);
    inb(0x60); // Read acknowledge
}

void mouse_handler_main(void) {
    uint8_t status = inb(0x64);
    if (status & 0x01) { // Data available
        if (status & 0x20) { // Mouse data bit set
            mouse_packet[mouse_cycle++] = inb(0x60);
            if (mouse_cycle == 3) {
                mouse_cycle = 0;
                
                // Parse mouse movement changes
                int rel_x = (int)mouse_packet[1];
                int rel_y = (int)mouse_packet[2];
                
                if (mouse_packet[0] & 0x10) rel_x |= 0xFFFFFF00; // Sign extend X
                if (mouse_packet[0] & 0x20) rel_y |= 0xFFFFFF00; // Sign extend Y
                
                mouse_x += rel_x / 2;
                mouse_y -= rel_y / 2; // Invert axis logic
                
                // Screen boundaries constraint check
                if (mouse_x < 0) mouse_x = 0;
                if (mouse_x > 79) mouse_x = 79;
                if (mouse_y < 0) mouse_y = 0;
                if (mouse_y > 24) mouse_y = 24;
                
                draw_mouse_cursor(mouse_x, mouse_y);
            }
        }
    }
    outb(0xA0, 0x20); // Send EOI to Slave PIC
    outb(0x20, 0x20); // Send EOI to Master PIC
}

__asm__ (
    ".global _mouse_handler_stub\n"
    "_mouse_handler_stub:\n"
    "    pushal\n"
    "    call _mouse_handler_main\n"
    "    popal\n"
    "    iret\n"
);
