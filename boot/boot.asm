[org 0x7c00]        ; Tells NASM this file runs at 0x7c00 absolute address
bits 16             ; Start in 16-bit Real Mode

_start:
    ; Setup segment registers safely
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00

    ; Reset disk system (Drive 0)
    mov ah, 0
    int 0x13

    ; Read Kernel from Disk using BIOS INT 13h
    ; We load your compiled C kernel right to memory address 0x9000
    mov bx, 0x9000      ; Destination buffer ES:BX = 0x0000:0x9000
    mov ah, 0x02        ; Read sectors from drive command
    mov al, 30          ; Number of sectors to read
    mov ch, 0           ; Cylinder 0
    mov dh, 0           ; Head 0
    mov cl, 2           ; Sector 2 (Sector 1 is this bootloader)
    int 0x13
    jc .disk_error      ; If carry flag is set, disk read failed

    ; Enable A20 Gate
    in al, 0x92
    or al, 2
    out 0x92, al

    ; Load Global Descriptor Table (GDT)
    lgdt [gdt_descriptor]

    ; Switch to 32-bit Protected Mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Far jump to flush the CPU pipeline and enter 32-bit execution mode
    jmp CODE_SEG:init_pm

.disk_error:
    cli
    hlt
    jmp .disk_error

bits 32             ; Switch to 32-bit Protected Mode execution context
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Jump directly to the kernel entry code loaded at 0x9000
    ; (Where our separate linked kernel chunk will sit)
    jmp 0x9000

; --- Global Descriptor Table (GDT) ---
gdt_start:
gdt_null:
    dd 0
    dd 0
gdt_code:
    dw 0xffff
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00
gdt_data:
    dw 0xffff
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; --- Pad out to exactly 510 bytes, then add MBR signature ---
times 510-($-$$) db 0
dw 0xAA55
