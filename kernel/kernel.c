#include "kernel.h"

void mouse_init(void);

// Explicitly send characters inline to avoid pointer offset lookups
static inline void serial_putchar(char c) {
    __asm__ volatile ("outb %0, %1" : : "a"(c), "Nd"((uint16_t)0x3F8));
}

void serial_print(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        serial_putchar(str[i]);
    }
}

void kernel_main(void) {
    // Force greetings straight onto the stack cache layout
    char boot_msg[] = {'[', 'K', 'E', 'R', 'N', 'E', 'L', ']', ' ', 'B', 'o', 'o', 't', 'e', 'd', '!', '\n', '\0'};
    serial_print(boot_msg);

    char visual_msg[] = {'O', 'S', ' ', 'P', 'r', 'o', 't', 'o', 't', 'y', 'p', 'e', ' ', 
                         'L', 'o', 'a', 'd', 'e', 'd', '.', ' ', 'T', 'y', 'p', 'e', ' ', 
                         'a', 'w', 'a', 'y', ':', '\0'};

    char* vga = (char*) 0xB8000;
    for (int i = 0; visual_msg[i] != '\0'; i++) {
        vga[i * 2] = visual_msg[i];
        vga[i * 2 + 1] = 0x0F; // White on black
    }

    // Initialize structures safely
    idt_init();
    mouse_init();

    while (1) {
        __asm__ volatile("hlt");
    }
}
