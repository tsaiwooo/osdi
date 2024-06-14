SECTIONS
{
    . = 0x60000;
    __relocoate_place = .;
    . = 0x80000;
    __code_start = .;
    .text : { KEEP(*(.text.boot)) *(.text) }
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
__bss_size = (__bss_end - __bss_start)>>3;
__code_size = _end - __code_start;