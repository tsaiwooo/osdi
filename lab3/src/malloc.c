#include "malloc.h"

char *top = &_heap_start;

void *simple_malloc(size_t size)
{
    // reserve space for header(size,分配狀態等等)
    char *cur = top;
    // size = size + 0x10 - size%0x10; // ALIGN 16
    // ((malloc_header *)top)->size = size;
    top += size;
    return cur;
}