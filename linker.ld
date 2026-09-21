SECTIONS
{
    . = 0x9000;
    
    .text : {
        boot/kernel_entry.o(.ktext)
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
        boot/kernel_entry.o(.kbss)
        *(.bss)
    }
}
