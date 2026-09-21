bits 32
section .ktext

global _loader
extern _kernel_main

_loader:
    mov esp, stack_top   ; Initialize your kernel stack pointer
    call _kernel_main    ; Call your main C kernel loop

.hang:
    cli
    hlt
    jmp .hang

section .kbss
align 16
stack_bottom:
    resb 16384
stack_top:
