#include "kernel.h"

// Set 1 US QWERTY Scancode to ASCII lookup table
const char scancode_ascii[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    // Rest zeroes for unmapped function keys
};

void keyboard_handler_main(void) {
    uint8_t status = inb(0x64);
    // Check if output buffer is full and data is from keyboard
    if (status & 0x01) {
        uint8_t scancode = inb(0x60);
        
        // Check if key release (break code has high bit set: e.g., > 0x80)
        if (!(scancode & 0x80)) {
            char c = scancode_ascii[scancode];
            if (c != 0) {
                // Echo key to VGA text buffer or debug output location
                char* vga = (char*) 0xB8000;
                static int pos = 0;
                vga[pos * 2] = c;
                vga[pos * 2 + 1] = 0x07; // Light grey on black
                pos = (pos + 1) % 2000;
            }
        }
    }
    
    // Send End of Interrupt (EOI) to Master PIC
    outb(0x20, 0x20);
}

// Assembly wrapper stub for the ISR
// Assembly wrapper stub for the ISR
__asm__ (
    ".global _keyboard_handler_stub\n"
    "_keyboard_handler_stub:\n"
    "    pushal\n"
    "    call _keyboard_handler_main\n"
    "    popal\n"
    "    iret\n"
);