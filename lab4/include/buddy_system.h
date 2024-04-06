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
#define DMA_ALLOCATE 13
// define in linker script
extern char _buddy_start;
extern char _buddy_end;
extern char _buddy_size;
// char *buddy_start = (char *)&_buddy_start;
// char *buddy_end = (char *)&_buddy_end;
/* buddy system data stucture */
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
/* buddy system data stucture */

/* DMA data stucture */
typedef struct DMA_node DMA_node;
struct DMA_node {
  char *address;
  DMA_node *next;
};

typedef struct DMA_area DMA_area;
struct DMA_area {
  int size;
  int free_number;
  DMA_node *head;
};

typedef struct DMA DMA;
struct DMA {
  // spilt to 16*2, 32*1, 64*1, 128*1, 256*1, 512*1, 1024*1, 2048*1
  DMA_area head[8];
  int page_index;
  DMA *next;
};
DMA *DMA_head;

/* DMA data stucture */
void init_buddy();
void *kmalloc(size_t);
void kfree(char *);
void init_DMA();
int DMA_order_funct(int);
void *DMA_malloc(int);
void DMA_free(char *);
void merge_page(int cur_order, int buddy_index, int cur_index);

#endif  // _BUDDY_STSTEM_