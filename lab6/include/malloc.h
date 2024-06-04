#ifndef _MALLOC_H_
#define _MALLOC_H_
#define size_t unsigned long
#include "mm.h"
#include "uart.h"
extern unsigned char _heap_start;
extern unsigned char* top;
void* simple_malloc(size_t size);
#endif // _MALLOC_H_