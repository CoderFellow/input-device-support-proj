ENTRY(_loader)

SECTIONS
{
    . = 0x9000;
    
    .text : {
        boot/kernel_entry.o(.text)
        *(.text)
    }

    .rodata : {
        *(.rodata)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(COMMON)
        *(.bss)
    }
}
