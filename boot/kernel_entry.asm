bits 32
section .text

global _loader
extern _kernel_main

_loader:
    cli                 ; Clear hardware interrupts during environment setup
    mov esp, 0x90000    ; FIXED: Establish stack pointer cleanly at safe high memory
    mov ebp, esp        ; Clear base pointer frame tracking row
    call _kernel_main   ; Enter your working C main method loop

.hang:
    cli
    hlt
    jmp .hang
