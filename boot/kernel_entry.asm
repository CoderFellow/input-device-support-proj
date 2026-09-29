bits 32
section .text

global _loader
extern _kernel_main

_loader:
    cli                 ; Clear hardware interrupts during environment setup
    
    ; --- VISUAL PROOF: Write bright green 'K' at top-left of VGA screen ---
    mov byte [0xB8000], 'K'
    mov byte [0xB8001], 0x0A

    mov esp, 0x90000    ; Establish stack pointer cleanly
    mov ebp, esp        
    call _kernel_main   ; Enter your C main method

.hang:
    cli
    hlt
    jmp .hang