#ifndef _BUDDY_SYSTEM_
#define _BUDDY_SYSTEM_
#include "malloc.h"
#include "uart.h"
#define size_t unsigned long
#define NULL 0
#define MAX_ORDER 14
#define KB 1024
#define page_size 4096
#define range 0x3C000000
#define total_pages range / page_size
#define ALLOCATED 10
#define TOO_LARGE 11
#define FREE 12
#define DMA_ALLOCATE 13
#define RESERVE_MEMORY 14
// define in linker script
extern char _buddy_start;
extern char _buddy_end;
extern char _buddy_size;
// char *buddy_start = (char *)&_buddy_start;
// char *buddy_end = (char *)&_buddy_end;
/* buddy system data stucture */
typedef struct Node Node;
struct Node {
    Node* next;
    int index;
};

typedef struct frame frame;
struct frame {
    int index;
    short order;
    short condition;
    frame *next, *prev;
};
// 245760 pages, 1000 pages in one array
// record which page is used
frame* frame_array;

typedef struct {
    //   Node *list;
    frame* list;
    unsigned int nr_free;
    unsigned int order;
} free_area;

typedef struct {
    free_area free_area[MAX_ORDER + 1];
} buddy_zone;

buddy_zone buddy;
// frame *page_array[250];
/* buddy system data stucture */

/* DMA data stucture */
typedef struct DMA_node DMA_node;
struct DMA_node {
    char* address;
    DMA_node* next;
};

typedef struct DMA_area DMA_area;
struct DMA_area {
    int size;
    int free_number;
    DMA_node* head;
};

typedef struct DMA DMA;
struct DMA {
    // spilt to 16*2, 32*1, 64*1, 128*1, 256*1, 512*1, 1024*1, 2048*1
    DMA_area head[8];
    int page_index;
    DMA* next;
};
DMA* DMA_head;

/* DMA data stucture */
void init_buddy();
void* kmalloc(size_t);
void kfree(char*);
void init_DMA();
int DMA_order_funct(int);
void* DMA_malloc(int);
void DMA_free(char*);
void merge_page(int cur_order, int buddy_index, int cur_index);
// void insert_page_in_frame_list(int order, int condition, int index);
// frame *find_page_in_frame_list(int index);
// int find_page_in_freelists(int order, int index);
void delete_page_in_freelists(int order, int index);
// void delete_page_in_frame_list(int index);
void switch_status_page_in_frame_list(int order, int index, int status);
void print_log();
#endif // _BUDDY_STSTEM_