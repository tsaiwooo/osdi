#include "malloc.h"

unsigned char *top = (unsigned char *)&_heap_start;

void *simple_malloc(size_t size) {
  char *cur = (char *)top;
  // uart_printf("cur address = %x\n", cur);
  // size = size + 0x10 - size%0x10; // ALIGN 16
  size = (size + 0xF) & ~0xF;  // 將size上調至最接近的16的倍數

  top += size;
  //   top += 10;
  return cur;
}