[org 0x7c00]
bits 16

_start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    mov [BOOT_DRIVE], dl

    ; --- Read kernel sectors using standard CHS (AH = 0x02) ---
    ; We need to load 76 sectors starting from Cylinder 0, Head 0, Sector 2
    mov bx, 0x9000          ; Destination memory offset
    mov dh, 0               ; Head 0
    mov ch, 0               ; Cylinder 0
    mov cl, 2               ; Sector 2 (Sector 1 is our boot sector)
    mov al, 76              ; Read 76 sectors total

.read_loop:
    mov ah, 0x02            ; BIOS read sector function
    mov dl, [BOOT_DRIVE]
    int 0x13
    jc .disk_error          ; If carry flag set, read failed

    ; Enable A20 Gate
    in al, 0x92
    or al, 2
    out 0x92, al

    ; Switch to Protected Mode
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp CODE_SEG:init_pm

.disk_error:
    ; Visual error marker: print red 'E' at 0xB8000
    mov byte [0x8000], 'E'  ; Wait, let's keep it safe without memory warnings:
    cli
    hlt
    jmp .disk_error

BOOT_DRIVE db 0

; --- Protected Mode Setup ---
align 16
bits 32
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, 0x90000
    mov ebp, esp

    ; Jump to C kernel entry at 0x9000
    jmp 0x9000

; --- GDT ---
align 4
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

times 510-($-$$) db 0
dw 0xAA55