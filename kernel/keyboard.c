#include "kernel.h"
// Add this declaration if not already present
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

extern void keyboard_handler_stub(void);

const char scancode_ascii[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

void keyboard_handler_main(void) {
    uint8_t status = inb(0x64);
    if (status & 0x01) {
        uint8_t scancode = inb(0x60);
        
        if (!(scancode & 0x80)) {
            char c = scancode_ascii[scancode];
            if (c != 0) {
                char* vga = (char*) 0xB8000;
                
                // FIXED: Start tracking text position right after the greeting message
                // Length of "OS Prototype Loaded. Type away:" is 31 characters
                static int pos = 31; 
                
                vga[pos * 2] = c;
                vga[pos * 2 + 1] = 0x0F; // High-intensity White on Black text
                pos = (pos + 1) % 2000;
            }
        }
    }
    outb(0x20, 0x20);
}


void keyboard_install(void) {
    // Map IRQ1 (keyboard) to offset 0x21 or your IDT vector entry
    // Assuming your IDT setup maps IRQ1 to vector 33 (0x21)
    idt_set_gate(33, (uint32_t)keyboard_handler_stub, 0x08, 0x8E);

    // Enable keyboard interrupt on the PIC (Clear mask for IRQ 1)
    uint8_t mask = inb(0x21);
    outb(0x21, mask & ~(1 << 1));
}

__asm__ (
    ".global _keyboard_handler_stub\n"
    "_keyboard_handler_stub:\n"
    "    pushal\n"
    "    call _keyboard_handler_main\n"
    "    popal\n"
    "    iret\n"
);
