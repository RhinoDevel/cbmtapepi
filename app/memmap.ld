
/* RhinoDevel, Marcel Timm, 2019aug22 */

/* See: https://sourceware.org/binutils/docs-2.29/ld/Scripts.html#Scripts */

ENTRY(_start) /* Just necessary for intermediately build ELF output file. */

SECTIONS
{
    . = 0x8000; /* (sets the location counter, which is initially 0) */
    __start = .; /* (creates symbol, usable by source files) */

    /* Code segment: */
    __text_start = .;
    .text : /* (output section name) */
    {
        /* Make sure, that .text.boot is placed at beginning of output file: */
        KEEP(*(.text.boot)) /* [KEEP() protects from link-time garbage coll.] */

        *(.text) /* (use all .text input sections from all input files) */
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

    __heap = .;

    __end = .;
}
