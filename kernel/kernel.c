#include "kernel.h"

void kernel_main(void) {
    // Clear screen or print boot message
    char* vga = (char*) 0xB8000;
    const char* msg = "OS Prototype Loaded. Type away:";
    for (int i = 0; msg[i] != '\0'; i++) {
        vga[i * 2] = msg[i];
        vga[i * 2 + 1] = 0x0F;
    }

    // Initialize Interrupt Descriptor Table & Keyboard IRQ1
    idt_init();

    while (1) {
        __asm__ volatile("hlt"); // Halt CPU until next interrupt arrives
    }
}