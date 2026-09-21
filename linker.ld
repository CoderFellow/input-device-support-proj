ENTRY(loader)
OUTPUT_FORMAT(elf32-i386)

SECTIONS
{
    /* The kernel will be loaded at 1 Megabyte */
    . = 0x00100000;

    .text : {
        *(.multiboot)
        *(.text)
    }

    .rodata : {
        *(.rodata*)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(COMMON)
        *(.bss)
    }
}