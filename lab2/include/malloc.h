#ifndef _MALLOC_H_
#define _MALLOC_H_
#define size_t unsigned long
extern char _heap_start;
extern char *top;
void *simple_malloc(size_t size);
#endif // _MALLOC_H_