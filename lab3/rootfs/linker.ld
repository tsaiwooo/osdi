SECTIONS
{
    . = 0x10000;
    .text : { KEEP(*(.text.boot)) *(.text) }
    .rodata : { *(.rodata) }
    PROVIDE(_data = .);
    .data : { *(.data) }
    .bss (NOLOAD) : {
        . = ALIGN(16);
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }
    _end = .;

}
