#ifndef __RESERVE_MEMORY_H_
#define __RESERVE_MEMORY_H_
#include "buddy_system.h"
#include "cpio.h"
#include "malloc.h"
#include "my_string.h"
#include "uart.h"
extern unsigned char _spin_table_start;
extern unsigned char _spin_table_end;
extern unsigned char _heap_start;
extern unsigned char _heap_end;
extern unsigned char _kernel_start;
extern unsigned char _kernel_end;
extern void *CPIO_DEFAULT_PLACE;
void *CPIO_DEFAULT_PLACE_END;
extern char *dtb_place;
extern unsigned long dtb_len;
void memory_reserve(unsigned long start, unsigned long end);
void mem_zone_reserve();
#endif  // __RESERVE_MEMORY_H_