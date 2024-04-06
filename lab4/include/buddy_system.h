#ifndef _BUDDY_SYSTEM_
#define _BUDDY_SYSTEM_
#include "malloc.h"
#include "uart.h"
#define size_t unsigned long
#define NULL 0
#define MAX_ORDER 5
#define KB 1024
#define page_size 4 * KB
#define total_pages 128 * KB / page_size
#define ALLOCATED 10
#define TOO_LARGE 11
#define FREE 12
// define in linker script
extern char _buddy_start;
extern char _buddy_end;
extern char _buddy_size;
// char *buddy_start = (char *)&_buddy_start;
// char *buddy_end = (char *)&_buddy_end;
typedef struct Node Node;
struct Node {
  Node *next;
  int index;
};

typedef struct {
  Node *list;
  unsigned long nr_free;
  unsigned int order;
} free_area;

typedef struct {
  free_area free_area[MAX_ORDER + 1];
} buddy_zone;

typedef struct {
  int order;
  int condition;
} frame;
frame page_array[total_pages];
buddy_zone buddy;

void init_buddy();
void *kmalloc(size_t);
void kfree(char *);
int match_freelist(int, int);
#endif  // _BUDDY_STSTEM_