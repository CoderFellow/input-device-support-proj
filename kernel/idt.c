#include "kernel.h"

struct idt_entry {
    uint16_t base_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct idt_entry idt[256];
struct idt_ptr idtp;

// FIXED: Remove the manual underscores. The C compiler automatically appends them!
extern void keyboard_handler_stub(void);
extern void mouse_handler_stub(void);

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low = (base & 0xFFFF);
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector = sel;
    idt[num].zero = 0;
    idt[num].flags = flags;
}

void pic_remap(void) {
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20); // Master vector offset 0x20
    outb(0xA1, 0x28); // Slave vector offset 0x28
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    
    // FIXED: Active-low bit mask update
    outb(0x21, 0xF9); // Unmask IRQ1 (Keyboard) and IRQ2 (Cascade) -> 1111 1001b
    outb(0xA1, 0xEF); // Unmask IRQ12 strictly (PS/2 Mouse line)   -> 1110 1111b
}


// Create a basic safety loop trap
void generic_exception_handler(void) {
    serial_print("[PANIC] Unhandled Exception or Interrupt triggered!\n");
    while(1) { __asm__ volatile("cli; hlt"); }
}

void idt_init(void) {
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (uint32_t)&idt;

    // 1. Clear out table memory safely
    for(int i = 0; i < 256; i++) {
        idt_set_gate(i, (uint32_t)generic_exception_handler, 0x08, 0x8E);
    }

    // 2. Remap the PIC channels
    pic_remap();

    // 3. Override your explicit hardware driver entries
    idt_set_gate(0x21, (uint32_t)keyboard_handler_stub, 0x08, 0x8E);
    idt_set_gate(0x2C, (uint32_t)mouse_handler_stub, 0x08, 0x8E);

    __asm__ volatile("lidt (%0)" : : "r" (&idtp));
    __asm__ volatile("sti"); // Now safe from basic unhandled vector crashes!
}
