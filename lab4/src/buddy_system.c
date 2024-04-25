#include "buddy_system.h"
char* buddy_start = (char*)&_buddy_start;
char* buddy_end = (char*)&_buddy_end;

void init_buddy()
{
    //   init total page array
    frame_array = simple_malloc(sizeof(frame) * total_pages);
    for (int i = 0; i < total_pages; ++i) {
        frame_array[i].condition = FREE;
        frame_array[i].order = -1;
        frame_array[i].index = i;

        // devise
        frame_array[i].next = NULL;
        frame_array[i].prev = NULL;
    }
    // init free list of any order
    for (int i = 0; i < MAX_ORDER; ++i) {
        buddy.free_area[i].nr_free = 0;
        buddy.free_area[i].order = i;
        buddy.free_area[i].list = NULL;
    }
    // devise Node to frame
    //   Node *tmp__ = simple_malloc(sizeof(Node));
    //   frame *tmp__ = simple_malloc(sizeof(frame));
    frame* tmp__ = &frame_array[0];
    //   tmp__->index = 0;
    tmp__->order = MAX_ORDER;
    tmp__->next = NULL;
    tmp__->prev = NULL;
    buddy.free_area[MAX_ORDER].list = tmp__;
    frame* cur_max = buddy.free_area[MAX_ORDER].list;
    //   buddy.free_area[MAX_ORDER].nr_free = total_pages / (1 << 15);
    buddy.free_area[MAX_ORDER].nr_free = (0x3C000000) / ((1 << 14) * page_size);
    //   uart_printf("max size = %d\n", buddy.free_area[MAX_ORDER].nr_free);
    for (int i = 1; i < buddy.free_area[MAX_ORDER].nr_free; ++i) {
        // tmp__ = simple_malloc(sizeof(frame));
        // uart_printf("index = %d\n", (1 << 15) * i);
        // tmp__ = &frame_array[(1 << 15) * page_size * i];
        tmp__ = &frame_array[(1 << 14) * i];
        tmp__->order = MAX_ORDER;
        // tmp__->index = (1 << 15) * i;
        tmp__->next = NULL;
        tmp__->prev = cur_max;
        cur_max->next = tmp__;
        cur_max = cur_max->next;
    }
    //   for (int i = 0; i < buddy.free_area[MAX_ORDER].nr_free; ++i) {
    //     uart_printf("index = %x\n", buddy.free_area[MAX_ORDER].list->index);
    //     buddy.free_area[MAX_ORDER].list =
    //     buddy.free_area[MAX_ORDER].list->next;
    //   }
    buddy.free_area[MAX_ORDER].order = MAX_ORDER;
}

void* kmalloc(size_t size)
{
    uart_printf("before malloc, the free number of each lists\n");
    // for (int i = 0; i <= MAX_ORDER; ++i) {
    //     uart_printf("%d order free number = %d\n", i, buddy.free_area[i].nr_free);
    // }
    print_log();
    // size < page size -> using dma
    if (size <= (page_size / 2)) {
        return DMA_malloc(size);
    }

    // find the fit order
    int order = 0;
    while ((page_size * (1 << order)) < size) {
        order++;
    }
    // uart_printf("order = %d\n", order);
    // go to find the free space in the free list according to its order
    if (buddy.free_area[order].nr_free == 0) {
        int order_gap = 1;
        while (buddy.free_area[order + order_gap].nr_free == 0) {
            order_gap++;
        }

        // uart_printf("order_gap = %d , cur_order = %d\n", order_gap,
        //             order + order_gap);

        // split and set buddy
        int now_page_index = buddy.free_area[order + order_gap].list->index;
        switch_status_page_in_frame_list(order, now_page_index, ALLOCATED);
        // insert_page_in_frame_list(order, ALLOCATED, now_page_index);
        // page_array[now_page_index].condition = ALLOCATED;
        // page_array[now_page_index].order = order;
        // current free block--, and list set to next free block
        buddy.free_area[order + order_gap].nr_free--;
        buddy.free_area[order + order_gap].list = buddy.free_area[order + order_gap].list->next;

        buddy.free_area[order + order_gap].list->prev = NULL;
        // handle the split page
        for (int cur_order = (order + order_gap); cur_order > order; --cur_order) {
            int buddy_index = now_page_index ^ (1 << (cur_order - 1));
            //   uart_printf("cur_order = %d start order = cur_order-1, nr_free =
            //   %d\n",
            //               cur_order, buddy.free_area[cur_order - 1].nr_free);
            //   uart_printf("cur_order = %d start order = 2, nr_free = %d\n",
            //   cur_order,
            //               buddy.free_area[2].nr_free);
            // split into two blocks and add it to free lists according to its order
            // devise it to frame
            frame* tmp = &frame_array[buddy_index];
            //   Node *tmp = simple_malloc(sizeof(Node));

            // page array change the order according to the page
            //   page_array[buddy_index].order = (cur_order - 1);
            buddy.free_area[cur_order - 1].nr_free++;
            //   tmp->index = buddy_index;
            tmp->order = (cur_order - 1);
            //   uart_printf("buddy_idx  = %d, buddy_order = %d\n", buddy_index,
            //               frame_array[buddy_index].order);
            tmp->next = NULL;
            tmp->prev = NULL;
            //   if (buddy.free_area[cur_order - 1].list == NULL) {
            if (buddy.free_area[cur_order - 1].nr_free == 1) {
                buddy.free_area[cur_order - 1].list = tmp;
            } // 某個nr_free的數量不對,導致linked list這邊有錯誤
            else {
                tmp->next = buddy.free_area[cur_order - 1].list;
                buddy.free_area[cur_order - 1].list->prev = tmp;
                buddy.free_area[cur_order - 1].list = tmp;
            }
        }
        uart_printf("after malloc, the free number of each lists \n");
        print_log();
        // for (int i = 0; i <= MAX_ORDER; ++i) {
        //   uart_printf("%d order free number = %d ", i,
        //   buddy.free_area[i].nr_free); frame *cur = buddy.free_area[i].list;
        //   while (cur) {
        //     uart_printf("  %d ", cur->index);
        //   }
        //   uart_printf("\n");
        // }
        // uart_printf("now_page_idx = %d\n", now_page_index * page_size);
        // return now_page_index * page_size;
        return buddy_start + now_page_index * page_size;
    } else {
        buddy.free_area[order].nr_free--;
        frame* node;
        frame* cur = buddy.free_area[order].list;
        node = cur;
        switch_status_page_in_frame_list(order, cur->index, ALLOCATED);
        // insert_page_in_frame_list(order, ALLOCATED, cur->index);
        buddy.free_area[order].list = buddy.free_area[order].list->next;
        buddy.free_area[order].list->prev = NULL;
        // page_array[cur->index].condition = ALLOCATED;
        // page_array[cur->index].order = order;
        uart_printf("after malloc, the free number of each lists, cur page = %d\n",
            node->index);
        // for (int i = 0; i <= MAX_ORDER; ++i) {
        //   uart_printf("%d order free number = %d\n", i,
        //   buddy.free_area[i].nr_free);
        // }
        print_log();
        // uart_printf("now_page_idx = %x\n", node->index * page_size);
        // return node->index * page_size;
        return buddy_start + node->index * page_size;
    }
}

void kfree(char* address)
{
    int cur_index = (address - buddy_start) / (page_size);
    //   int cur_order = page_array[cur_index].order;
    frame* cur_page = &frame_array[cur_index];
    //   frame *cur_page = find_page_in_frame_list(cur_index);
    int cur_order = cur_page->order;
    //   if (page_array[cur_index].condition == DMA_ALLOCATE) {
    //     DMA_free(address);
    //   }
    if (cur_page->condition == DMA_ALLOCATE) {
        DMA_free(address);
    }
    //   page_array[cur_index].condition = FREE;
    cur_page->condition = FREE;
    // when free, add 1 in current order
    buddy.free_area[cur_order].nr_free++;
    //   delete_page_in_frame_list(cur_index);
    switch_status_page_in_frame_list(cur_order, cur_index, FREE);
    //   frame_array[cur_index].condition = FREE;
    //   frame_array[cur_index].order = -1;
    // insert it to list, devise it to frame
    frame* tmp = &frame_array[cur_index];
    //   Node *tmp = simple_malloc(sizeof(Node));
    //   tmp->index = cur_index;
    tmp->prev = NULL;
    tmp->next = buddy.free_area[cur_order].list;
    buddy.free_area[cur_order].list = tmp;
    while (cur_order < MAX_ORDER) {
        int buddy_index = (cur_index ^ (1 << (cur_order)));
        frame* buddy_page = &frame_array[buddy_index];
        // if buddy_page is in frame list or buddy is not in free list
        if (buddy_page->condition != FREE || buddy_page->order != cur_order) {
            uart_printf(
                "cur_order = %d, buddy_page = %d, cur_page = %d, in not merge\n",
                cur_order, buddy_index, cur_index);
            // if (buddy_page->condition != FREE || buddy_page->order != cur_order) {
            //   page_array[cur_index].order = cur_order;
            //   insert it to list
            //   Node *tmp = simple_malloc(sizeof(Node));
            //   tmp->index = cur_index;
            //   tmp->next = buddy.free_area[cur_order].list;
            //   buddy.free_area[cur_order].list = tmp;
            break;
        } else {
            // free nodes in current order-=2,itself and its buddy
            delete_page_in_freelists(cur_order, buddy_index);
            delete_page_in_freelists(cur_order, cur_index);
            merge_page(cur_order, buddy_index, cur_index);
            //   buddy.free_area[cur_order].nr_free -= 2;
            //   Node *cur = buddy.free_area[cur_order].list;
            //   // delete free node in cur_order
            //   // if head match, head = head->next
            //   if (cur->index == buddy_index) {
            //     buddy.free_area[cur_order].list =
            //     buddy.free_area[cur_order].list->next;
            //   } else {
            //     Node *pre;
            //     // not head, then pre->next = cur->next
            //     while (cur != NULL) {
            //       if (cur->index == buddy_index) {
            //         pre->next = cur->next;
            //         break;
            //       }
            //       pre = cur;
            //       cur = cur->next;
            //     }
            //   }
            //   // find merge, add to next
            //   buddy.free_area[cur_order + 1].nr_free++;
            //   // add new node to next order
            //   Node *next_order_cur = buddy.free_area[cur_order + 1].list;
            //   Node *next_tmp = simple_malloc(sizeof(Node));
            //   next_tmp->index = cur_index;
            //   next_tmp->next = NULL;
            //   if (next_order_cur == NULL) {
            //     buddy.free_area[cur_order + 1].list = next_tmp;
            //   } else {
            //     while (next_order_cur->next != NULL) {
            //       next_order_cur = next_order_cur->next;
            //     }
            //     next_order_cur->next = next_tmp;
            //   }
        }
        // determine which one is the main indexm which new
        cur_index = (cur_index > buddy_index) ? buddy_index : cur_index;
        cur_order++;
    }
    uart_printf("===================================\nafter free function\n");
    //   for (int i = 0; i <= MAX_ORDER; ++i) {
    //     uart_printf("%d order free number = %d\n", i,
    //     buddy.free_area[i].nr_free);
    //   }
    print_log();
}

void init_DMA()
{
    char* addr = kmalloc(page_size);
    DMA* cur = DMA_head;
    DMA* DMA_tmp = simple_malloc(sizeof(DMA));
    DMA_node* tmp = simple_malloc(sizeof(DMA_node));
    DMA_node* tmp2 = simple_malloc(sizeof(DMA_node));
    // record which page is allocated
    DMA_tmp->page_index = (addr - buddy_start) / (page_size);
    //   frame *dma_page = find_page_in_frame_list(DMA_tmp->page_index);
    frame* dma_page = &frame_array[DMA_tmp->page_index];
    dma_page->condition = DMA_ALLOCATE;
    dma_page->order = 0;
    //   page_array[DMA_tmp->page_index].condition = DMA_ALLOCATE;
    //   page_array[DMA_tmp->page_index].order = 0;
    uart_printf("addr = %x, In DMA the page is %d\n", addr, DMA_tmp->page_index);
    DMA_tmp->head[0].size = 16;
    DMA_tmp->head[0].free_number = 2;
    tmp->address = addr + 16 * 0;
    tmp2->address = addr + 16 * 1;
    tmp->next = tmp2;
    tmp2->next = NULL;
    DMA_tmp->head[0].head = tmp;
    for (int i = 1; i < 8; ++i) {
        tmp = (DMA_node*)simple_malloc(sizeof(DMA_node));
        DMA_tmp->next = NULL;
        DMA_tmp->head[i].free_number = 1;
        tmp->address = addr + (1 << (i + 4));
        tmp->next = NULL;
        DMA_tmp->head[i].head = tmp;
        DMA_tmp->head[i].size = (1 << (i + 4));
    }
    if (!DMA_head) {
        DMA_head = DMA_tmp;
    } else {
        while (cur->next != NULL) {
            cur = cur->next;
        }
        cur->next = DMA_tmp;
    }
}

int DMA_order_funct(int size)
{
    int nearest_order = 0;
    for (nearest_order = 0; (1 << nearest_order) < size; nearest_order++) {
        ;
    }
    if (nearest_order < 4)
        return 0;
    return nearest_order - 4;
}

void* DMA_malloc(int size)
{
    const int DMA_order = DMA_order_funct(size);
    int DMA_order_tmp = DMA_order;
    //   uart_printf("DMA_order = %d\n", DMA_order);
    DMA* cur = DMA_head;
    // find the free space over every node in each page
    while (cur->head[DMA_order].free_number == 0 && cur != NULL) {
        DMA_order_tmp++;
        if (DMA_order_tmp > 7) {
            cur = cur->next;
            DMA_order_tmp = DMA_order;
        }
    }
    // represents the free spcae is not enough, so allocate a new page for it, and
    // find this page in the list
    if (!cur) {
        init_DMA();
        // cur = cur->next;
        cur = DMA_head;
        while (cur->next) {
            cur = cur->next;
        }
    }
    cur->head[DMA_order_tmp].free_number--;
    char* addr = cur->head[DMA_order_tmp].head->address;
    cur->head[DMA_order_tmp].head = cur->head[DMA_order_tmp].head->next;
    //   uart_printf("%d order, at address %x\n", DMA_order_tmp, addr);
    return (void*)addr;
}

void DMA_free(char* addr)
{
    int cur_index = (addr - buddy_start) / (page_size);
    int DMA_order = DMA_order_funct((addr - buddy_start));
    DMA* cur = DMA_head;

    while (cur->page_index != cur_index && cur) {
        cur = cur->next;
    }
    // the address doesn't exist
    if (!cur)
        return;
    DMA_node* tmp = simple_malloc(sizeof(DMA_node));
    tmp->address = addr;
    tmp->next = NULL;
    cur->head[DMA_order].free_number++;
    DMA_node* cur_node = cur->head[DMA_order].head;
    if (!cur_node) {
        cur->head[DMA_order].head = tmp;
    } else {
        cur->head[DMA_order].head->next = tmp;
    }
}

void merge_page(int cur_order, int buddy_index, int cur_index)
{
    buddy.free_area[cur_order].nr_free -= 2;
    // find merge, add to next
    buddy.free_area[cur_order + 1].nr_free++;
    // add new node to next order
    // devise it to frame
    frame* next_tmp = &frame_array[cur_index > buddy_index ? buddy_index : cur_index];
    //   Node *next_tmp = simple_malloc(sizeof(Node));
    //   next_tmp->index = cur_index;
    // free index choose the small one, so it can be compared, devise it ot frame
    //   next_tmp->index = cur_index > buddy_index ? buddy_index : cur_index;
    // insert from head
    next_tmp->prev = NULL;
    next_tmp->next = buddy.free_area[cur_order + 1].list;
    buddy.free_area[cur_order + 1].list = next_tmp;
    next_tmp->condition = FREE;
    next_tmp->order = cur_order + 1;
}

void delete_page_in_freelists(int order, int index)
{
    frame* cur = &frame_array[index];
    if (!cur->prev && !cur->next) {
        buddy.free_area[order].list = NULL;
    } else if (index == buddy.free_area[order].list->index) {
        buddy.free_area[order].list = buddy.free_area[order].list->next;
        if (buddy.free_area[order].list)
            buddy.free_area[order].list->prev = NULL;
    } else if (!cur->next) {
        cur->prev->next = NULL;
        cur->prev = NULL;
    } else {
        frame* pre = cur->prev;
        pre->next = cur->next;
        cur->next->prev = pre;
    }
}

void switch_status_page_in_frame_list(int order, int index, int status)
{
    frame_array[index].order = order;
    frame_array[index].condition = status;
}

void print_log()
{
    for (int i = 0; i <= MAX_ORDER; ++i) {
        uart_printf("%d order free number = %d ", i, buddy.free_area[i].nr_free);
        frame* cur = buddy.free_area[i].list;
        int numbers = buddy.free_area[i].nr_free;
        while (cur && numbers) {
            // while (cur) {
            numbers--;
            uart_printf("  %d ", cur->index);
            cur = cur->next;
        }
        uart_printf("\n");
    }
}