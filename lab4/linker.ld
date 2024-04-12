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
_heap_start = 0x200000;
_heap_end = 0x400000;
/* for mem reserve use*/
_spin_table_start = 0x0;
_spin_table_end = 0x1000;
_buddy_start = 0x0;
_buddy_end = 0x3C000000;
_kernel_start = 0x60000;
_kernel_end = _end;
