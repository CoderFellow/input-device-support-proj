[org 0x7c00]
bits 16

_start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Save the boot drive supplied by BIOS
    mov [BOOT_DRIVE], dl

    ; Reset disk system
    mov ah, 0
    mov dl, [BOOT_DRIVE]
    int 0x13
    jc .disk_error

    ; --- LOAD KERNEL INTO 0x9000 ---
    ; We need to load 76 sectors total. 
    ; Let's load them in safe, large contiguous chunks using es:bx = 0x0900:0x0000.
    
    mov ax, 0x0900
    mov es, ax
    xor bx, bx          ; ES:BX = 0x0900:0x0000 -> Linear 0x9000

    ; Chunk 1: Read Track 0, Head 0, Sectors 2 to 18 (17 sectors)
    mov ah, 0x02
    mov al, 17          
    mov ch, 0           
    mov dh, 0           
    mov cl, 2           
    mov dl, [BOOT_DRIVE]
    int 0x13
    jc .disk_error

    ; Advance buffer pointer by 17 sectors * 512 bytes = 8704 bytes (0x2200)
    mov bx, 0x2200

    ; Chunk 2: Read Track 0, Head 1, Sectors 1 to 18 (18 sectors)
    mov ah, 0x02
    mov al, 18          
    mov ch, 0           
    mov dh, 1           
    mov cl, 1           
    mov dl, [BOOT_DRIVE]
    int 0x13
    jc .disk_error

    ; Advance buffer pointer by another 18 sectors * 512 bytes = 9216 bytes (0x2400 -> total offset 0x4600)
    add bx, 0x2400

    ; Chunk 3: Read Track 1, Head 0, Sectors 1 to 18 (18 sectors)
    mov ah, 0x02
    mov al, 18          
    mov ch, 1           
    mov dh, 0           
    mov cl, 1           
    mov dl, [BOOT_DRIVE]
    int 0x13
    jc .disk_error

    ; Advance buffer pointer by another 18 sectors (0x2400 -> total offset 0x6A00)
    add bx, 0x2400

    ; Chunk 4: Read remaining sectors (76 total - 17 - 18 - 18 = 23 sectors remaining)
    ; Let's read Track 1, Head 1, Sectors 1 to 23
    mov ah, 0x02
    mov al, 23          
    mov ch, 1           
    mov dh, 1           
    mov cl, 1           
    mov dl, [BOOT_DRIVE]
    int 0x13
    jc .disk_error

    ; --- All 76 sectors loaded successfully! ---

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

    ; Far jump to protected mode entry
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

    ; Set up 32-bit stack
    mov esp, 0x90000
    mov ebp, esp

    ; Jump straight into the C kernel
    jmp 0x9000

; --- Variable storage ---
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