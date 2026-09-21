; Multiboot constants for GRUB
MBALIGN     equ  1<<0             ; align loaded modules on page boundaries
MEMINFO     equ  1<<1             ; provide memory map
FLAGS       equ  MBALIGN | MEMINFO ; this is the Multiboot 'flag' field
MAGIC       equ  0x1BADB002       ; 'magic number' lets bootloader find the header
CHECKSUM    equ -(MAGIC + FLAGS)      ; checksum of above, to prove we are multiboot

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .bss
align 16
stack_bottom:
resb 16384 ; 16 Kilobytes for stack
stack_top:

section .text
global loader
extern kernel_main

loader:
    ; Setup the stack pointer
    mov esp, stack_top

    ; Call the C kernel entry point
    call kernel_main

    ; If kernel returns, hang the CPU
.hang:
    cli
    hlt
    jmp .hang