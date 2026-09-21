#include "kernel.h"

// Send a character to COM1 serial port
static inline void serial_putchar(char c) {
    __asm__ volatile ("outb %0, %1" : : "a"(c), "Nd"((uint16_t)0x3F8));
}

void serial_print(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        serial_putchar(str[i]);
    }
}

void kernel_main(void) {
    serial_print("[KERNEL] Booted successfully!\n");

    // VGA text buffer output
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