SECTIONS
{
    /* mmap */
    . = 0xffff000000000000; /* kernel space */
    virtual_mem = .;
    . += 0x80000; /* kernel load address */
    _kernel_start = . ;
    /* . = 0x80000; */

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
    _code_end = .;
    /* _heap_start = . + 0x20000; */
    /* _heap_start = 0x100000; */
    . = ALIGN(0x1000);
    pg_dir = .;
    .data.pgd : {
         . += (515 * (1<<12)); 
    } /* PGD : 1 , PMD : 1 , PMD : 1, PTE : 512 */
    . = ALIGN(0x1000);
    _end = .;
 }
_code_size = _end - _kernel_start;
__bss_size = (__bss_end - __bss_start)>>3;
_user_stack =   0x50000;
_heap_start =   _end + 0x1000;
_heap_end =   _heap_start + 0x1001000;
/* for mem reserve use*/
_spin_table_start =   0x0;
_spin_table_end =   0x1000;
_buddy_start =   0x0;
_buddy_end =   0x3C000000;
_kernel_end =  _end;
