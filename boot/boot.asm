section .text
bits 32

; Multiboot constants
MBALIGN     equ  1<<0
MEMINFO     equ  1<<1
FLAGS       equ  MBALIGN | MEMINFO
MAGIC       equ  0x1BADB002
CHECKSUM    equ  -(MAGIC + FLAGS)

; Multiboot header at the very start of .text (ensures QEMU finds it)
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

global loader
extern _kernel_main

loader:
    mov esp, stack_top
    call _kernel_main

.hang:
    cli
    hlt
    jmp .hang

section .bss
align 16
stack_bottom:
resb 16384
stack_top: