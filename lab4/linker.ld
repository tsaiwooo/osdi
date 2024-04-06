SECTIONS
{
    . = 0x80000;
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
    /* _heap_start = . + 0x20000; */
    /* _heap_start = 0x100000; */
    _end = .;

   /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}
__bss_size = (__bss_end - __bss_start)>>3;
_heap_start = 0x100000;

_buddy_start = 0x3000000;
_buddy_end = 0x3020000;
/* 128KB size */
/* split to 32 pages */
_buddy_size = (_buddy_end - _buddy_start) >> 12;