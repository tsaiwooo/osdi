#include "reserve_memory.h"
//
void memory_reserve(unsigned long start, unsigned long end)
{
    // uart_printf("start = %d,end = %d\n", (unsigned long)start,
    //             (unsigned long)end);
    unsigned long size = (unsigned long)end - (unsigned long)start;
    uart_printf("start = %x, end = %x\n", start, end);
    uart_printf("size = %d\n", size);
    int cur_page_idx_start = (unsigned long)start / 4096, cur_page_idx, i;
    // uart_printf("start = %d cur_page_idx = %d\n", hextol(start),
    //             cur_page_idx_start);
    int need_pages = size / 4096;
    need_pages += (size % 4096) ? 1 : 0;
    for (i = 0; i < need_pages; ++i) {
        cur_page_idx = (cur_page_idx_start + i);
        // insert_page_in_frame_list(0, RESERVE_MEMORY, cur_page_idx);
        // switch_status_page_in_frame_list(0, cur_page_idx, RESERVE_MEMORY);
        int cur_order = 0;
        for (cur_order = 0; cur_order <= MAX_ORDER; ++cur_order) {
            // uart_printf("cur_page_idx = %d\n", cur_page_idx);
            // uart_printf("cur_order = %d,idx_order = %d, status = %d\n", cur_order,
            //             frame_array[cur_page_idx].order,
            //             frame_array[cur_page_idx].condition);
            int buddy_index = cur_page_idx ^ (1 << cur_order);
            if (frame_array[cur_page_idx].order == cur_order && frame_array[cur_page_idx].condition == FREE && buddy.free_area[cur_order].nr_free) {
                switch_status_page_in_frame_list(0, cur_page_idx, RESERVE_MEMORY);
                // uart_printf("in break condition\n");
                // if (find_page_in_freelists(cur_order, cur_page_idx)) {
                buddy.free_area[cur_order].nr_free--;
                delete_page_in_freelists(cur_order, cur_page_idx);
                break;
            }
            // if buddy is not in the current order free list
            // if (frame_array[buddy_index].order != -1 || frame_array[buddy_index].condition == FREE) {
            else if (frame_array[buddy_index].order != cur_order && frame_array[buddy_index].condition == FREE) {
                // if(frame_array[buddy_index].)
                frame* tmp = &frame_array[buddy_index];
                if (tmp->order > cur_order) {
                    // uart_printf("page = %d\n", tmp->index);
                    buddy.free_area[tmp->order].nr_free--;
                    delete_page_in_freelists(tmp->order, tmp->index);
                }
                tmp->order = cur_order;
                buddy.free_area[cur_order].nr_free++;
                tmp->prev = NULL;
                tmp->next = buddy.free_area[cur_order].list;
                // add free node in the free lists
                buddy.free_area[cur_order].list->prev = tmp;
                buddy.free_area[cur_order].list = tmp;
            }
            // choose the small one to be the cur_page_index
            if (i == 0)
                cur_page_idx = (cur_page_idx > buddy_index) ? buddy_index : cur_page_idx;
        }

        // handle MAX order
        // if (cur_order == MAX_ORDER) {
        //     buddy.free_area[MAX_ORDER].nr_free--;
        //     delete_page_in_freelists(cur_order, cur_page_idx);
        // }
        // uart_printf("page = %d\n", cur_page_idx);
    }
    uart_printf("===================================\nafter memory reserve\n");
    // for (int i = 0; i <= MAX_ORDER; ++i) {
    //   uart_printf("%d order free number = %d\n", i,
    //   buddy.free_area[i].nr_free);
    // }
    print_log();
    // int buddy_index = now_page_index ^ (1 << cur_order);
}
unsigned char* spin_start = &_spin_table_start;
unsigned char* spin_end = &_spin_table_end;
unsigned char* heap_start = &_heap_start;
unsigned char* heap_end = &_heap_end;
unsigned char* kernel_start = &_kernel_start;
unsigned char* kernel_end = &_kernel_end;
void mem_zone_reserve()
{
    memory_reserve((unsigned long)spin_start, (unsigned long)spin_end);
    memory_reserve((unsigned long)heap_start, (unsigned long)heap_end);
    memory_reserve((unsigned long)kernel_start, (unsigned long)kernel_end);
    // memory_reserve((unsigned long)CPIO_DEFAULT_PLACE,
    //                (unsigned long)CPIO_DEFAULT_PLACE_END);
    memory_reserve((unsigned long)dtb_place,
        (unsigned long)(dtb_place + dtb_len));
}