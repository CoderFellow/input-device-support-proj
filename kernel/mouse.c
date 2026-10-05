#include "kernel.h"

static uint8_t mouse_cycle = 0;
static int8_t mouse_byte[3];
static int mouse_x = 40;
static int mouse_y = 12;

// Wait for PS/2 controller to be ready to write
void mouse_wait_write(void) {
    int timeout = 100000;
    while (timeout--) {
        if ((inb(0x64) & 2) == 0) return;
    }
}

// Wait for PS/2 controller to have data ready to read
void mouse_wait_read(void) {
    int timeout = 100000;
    while (timeout--) {
        if ((inb(0x64) & 1) == 1) return;
    }
}

void mouse_write(uint8_t write) {
    mouse_wait_write();
    outb(0x64, 0xD4); // Tell controller we are sending data to the mouse port
    mouse_wait_write();
    outb(0x60, write);
}

uint8_t mouse_read(void) {
    mouse_wait_read();
    return inb(0x60);
}

void mouse_draw_cursor(void) {
    char* vga = (char*) 0xB8000;
    // Highlight character cell at mouse location (Invert VGA attribute byte)
    int pos = (mouse_y * 80 + mouse_x) * 2 + 1;
    vga[pos] = 0x70; // Inverted colors (black on white)
}

void mouse_handler_main(void) {
    uint8_t status = inb(0x64);
    if (status & 0x01) {
        uint8_t data = inb(0x60);
        
        // Ensure packet alignment bit (Bit 3 of Byte 0 must be 1)
        if (mouse_cycle == 0 && !(data & 0x08)) {
            outb(0x20, 0x20);
            outb(0xA0, 0x20);
            return;
        }

        mouse_byte[mouse_cycle++] = data;

        if (mouse_cycle == 3) {
            mouse_cycle = 0;

            int rel_x = mouse_byte[1];
            int rel_y = mouse_byte[2];

            if (mouse_byte[0] & 0x10) rel_x |= 0xFFFFFF00; // Sign extend X
            if (mouse_byte[0] & 0x20) rel_y |= 0xFFFFFF00; // Sign extend Y

            mouse_x += rel_x / 2;
            mouse_y -= rel_y / 2; // Invert Y for screen coordinates

            // Clamp coordinates to text screen bounds (80x25)
            if (mouse_x < 0) mouse_x = 0;
            if (mouse_x >= 80) mouse_x = 79;
            if (mouse_y < 0) mouse_y = 0;
            if (mouse_y >= 25) mouse_y = 24;

            mouse_draw_cursor();
        }
    }

    // Send EOI to both Master and Slave PICs
    outb(0xA0, 0x20);
    outb(0x20, 0x20);
}

void mouse_install(void) {
    uint8_t status;

    // Enable Auxiliary Device (Mouse)
    mouse_wait_write();
    outb(0x64, 0xA8);

    // Read Comptroller Command Byte
    mouse_wait_write();
    outb(0x64, 0x20);
    mouse_wait_read();
    
    // Enable Bit 1 (IRQ12) AND preserve Bit 0 (IRQ1 Keyboard)
    status = (inb(0x60) | 0x03); 

    // Write updated Command Byte back
    mouse_wait_write();
    outb(0x64, 0x60);
    mouse_wait_write();
    outb(0x60, status);

    // Set default mouse settings & enable packet streaming
    mouse_write(0xF6);
    mouse_read(); // ACK

    mouse_write(0xF4);
    mouse_read(); // ACK
}

// ISR Assembly Stub
__asm__ (
    ".global _mouse_handler_stub\n"
    "_mouse_handler_stub:\n"
    "    pushal\n"
    "    call _mouse_handler_main\n"
    "    popal\n"
    "    iret\n"
);