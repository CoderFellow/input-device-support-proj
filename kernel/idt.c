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

extern void keyboard_handler_stub(void);
extern void idt_load(void);

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low = (base & 0xFFFF);
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector = sel;
    idt[num].zero = 0;
    idt[num].flags = flags;
}

void pic_remap(void) {
    // ICW1: Initialize PICs
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    // ICW2: Remap offsets (Master -> 0x20, Slave -> 0x28)
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    // ICW3: Setup cascading
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    // ICW4: Environment info
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    // Mask interrupts except keyboard (IRQ1) and cascade
    outb(0x21, 0xFD); // 1111 1101 (IRQ1 enabled)
    outb(0xA1, 0xFF);
}

void idt_init(void) {
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (uint32_t)&idt;

    pic_remap();

    // Map keyboard handler to IRQ1 (Vector 0x21)
    idt_set_gate(0x21, (uint32_t)keyboard_handler_stub, 0x08, 0x8E);

    // Load IDT
    __asm__ volatile("lidt (%0)" : : "r" (&idtp));
    __asm__ volatile("sti"); // Enable interrupts
}