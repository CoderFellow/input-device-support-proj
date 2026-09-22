#include "kernel.h"

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

__asm__ (
    ".global _keyboard_handler_stub\n"
    "_keyboard_handler_stub:\n"
    "    pushal\n"
    "    call _keyboard_handler_main\n"
    "    popal\n"
    "    iret\n"
);
