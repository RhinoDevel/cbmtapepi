ENTRY(_start)

SECTIONS
{
    /* Starts at LOADER_ADDR: */
    . = 0x8000;
    __start = .;

    /* Code segment: */
    __text_start = .;
    .text :
    {
        KEEP(*(.text.boot))
        *(.text)
    }
    . = ALIGN(4096); /* Aligns to page size. */
    __text_end = .;

    /* Read-only data segment: */
    __rodata_start = .;
    .rodata :
    {
        *(.rodata)
    }
    . = ALIGN(4096); /* Aligns to page size. */
    __rodata_end = .;

    /* (Initialized) read-write data segment                               */
    /* (to hold explicitly initialized global and static local variables): */
    __data_start = .;
    .data :
    {
        *(.data)
    }
    . = ALIGN(4096); /* Aligns to page size. */
    __data_end = .;


    /* Uninitialized read-write data segment                                 */
    /* (to hold NOT explicitly initialized global & static local variables): */
    __bss_start = .;
    .bss :
    {
        bss = .;
        *(.bss)
    }
    . = ALIGN(4096); /* Aligns to page size. */
    __bss_end = .;
    __end = .;
}
