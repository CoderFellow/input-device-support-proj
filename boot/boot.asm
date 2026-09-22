[org 0x7c00]
bits 16

_start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00

    ; 🚀 FIXED: Save the active boot drive number provided by the BIOS in DL
    mov [BOOT_DRIVE], dl

    ; Reset disk system cleanly using the native drive number
    mov ah, 0
    mov dl, [BOOT_DRIVE]
    int 0x13
    jc .disk_error

    ; --- TRACK 0, HEAD 0: Read Sectors 2 to 18 (17 Sectors) ---
    mov bx, 0x9000      ; Destination memory buffer address
    mov ah, 0x02        ; Read sectors command
    mov al, 17          ; Read the remaining 17 sectors on this track
    mov ch, 0           ; Cylinder 0
    mov dh, 0           ; Head 0
    mov cl, 2           ; Start right at Sector 2
    mov dl, [BOOT_DRIVE] ; Pass the validated boot drive identifier back
    int 0x13
    jc .disk_error

    ; Advance destination pointer: 17 sectors * 512 bytes = 8704 bytes (0x2200)
    add bx, 0x2200

    ; --- TRACK 0, HEAD 1: Read Sectors 1 to 18 (18 Sectors) ---
    mov ah, 0x02
    mov al, 18          ; Read full track limit
    mov ch, 0           ; Cylinder 0
    mov dh, 1           ; Switch to Head 1
    mov cl, 1           ; Start at Sector 1
    mov dl, [BOOT_DRIVE]
    int 0x13
    jc .disk_error

    ; Advance destination pointer: 18 sectors * 512 bytes = 9216 bytes (0x2400)
    add bx, 0x2400

    ; --- TRACK 1, HEAD 0: Read Sectors 1 to 18 (18 Sectors) ---
    mov ah, 0x02
    mov al, 18
    mov ch, 1           ; Advance to Cylinder Track 1
    mov dh, 0           ; Reset to Head 0
    mov cl, 1           ; Start at Sector 1
    mov dl, [BOOT_DRIVE]
    int 0x13
    jc .disk_error

    ; --- All 53 sectors (approx 27KB) loaded successfully into RAM ---

    ; Enable A20 Gate safely
    in al, 0x92
    or al, 2
    out 0x92, al

    ; Load global descriptor table
    lgdt [gdt_descriptor]

    ; Switch to 32-bit Protected Mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Far jump to flush the CPU pipeline cache constraints
    jmp CODE_SEG:init_pm

.disk_error:
    cli
    hlt
    jmp .disk_error

align 16
bits 32
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Jump directly to the loaded C kernel entry stub code at 0x9000
    jmp 0x9000

; --- Variable storage safe from instruction alignment breaks ---
BOOT_DRIVE db 0

; --- Global Descriptor Table (GDT) ---
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
