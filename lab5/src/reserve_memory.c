#include "reserve_memory.h"
//
void memory_reserve(char *start, char *end) {
  // uart_printf("start = %d,end = %d\n", (unsigned long)start,
  //             (unsigned long)end);
  unsigned long size = (unsigned long)end - (unsigned long)start;
  uart_printf("start = %x, end = %x\n", start, end);
  uart_printf("size = %d\n", size);
  int cur_page_idx_start = (unsigned long)start / 4096, cur_page_idx = 0;
  uart_printf("start = %d cur_page_idx = %d\n", hextol(start),
              cur_page_idx_start);
  int need_pages = size / 4096;
  need_pages += (size % 4096) ? 1 : 0;
  for (int i = 0; i < need_pages; ++i) {
    cur_page_idx = (cur_page_idx_start + i);
    // insert_page_in_frame_list(0, RESERVE_MEMORY, cur_page_idx);
    switch_status_page_in_frame_list(0, cur_page_idx, RESERVE_MEMORY);
    int cur_order = 0;
    for (cur_order = 0; cur_order < MAX_ORDER; ++cur_order) {
      if (find_page_in_freelists(cur_order, cur_page_idx)) {
        buddy.free_area[cur_order].nr_free--;
        delete_page_in_freelists(cur_order, cur_page_idx);
        break;
      }
      int buddy_index = cur_page_idx ^ (1 << cur_order);

      Node *tmp = simple_malloc(sizeof(Node));
      buddy.free_area[cur_order].nr_free++;
      tmp->index = buddy_index;
      // add free node in the free lists
      tmp->next = buddy.free_area[cur_order].list;

      buddy.free_area[cur_order].list = tmp;
      // choose the small one to be the cur_page_index
      cur_page_idx = (cur_page_idx > buddy_index) ? buddy_index : cur_page_idx;
      // uart_printf("cur_order = %d, buddy_idx = %d\n", cur_order,
      // buddy_index);
    }

    // handle MAX order
    if (cur_order == MAX_ORDER) {
      buddy.free_area[MAX_ORDER].nr_free--;
      delete_page_in_freelists(cur_order, cur_page_idx);
      // Node *cur_max = buddy.free_area[MAX_ORDER].list;
      // Node *pre = cur_max;
      // if (cur_max->index == cur_page_idx) {
      //   buddy.free_area[MAX_ORDER].list =
      //   buddy.free_area[MAX_ORDER].list->next;
      // } else {
      //   while (cur_max->index != cur_page_idx) {
      //     pre = cur_max;
      //     cur_max = cur_max->next;
      //   }
      //   pre->next = cur_max->next;
      // }
    }
  }
  uart_printf("===================================\nafter memory reserve\n");
  for (int i = 0; i <= MAX_ORDER; ++i) {
    uart_printf("%d order free number = %d\n", i, buddy.free_area[i].nr_free);
  }
  // int buddy_index = now_page_index ^ (1 << cur_order);
}
char *spin_start = &_spin_table_start;
char *spin_end = &_spin_table_end;
char *heap_start = &_heap_start;
char *heap_end = &_heap_end;
char *kernel_start = &_kernel_start;
char *kernel_end = &_kernel_end;
void mem_zone_reserve() {
  char *cpio_start = CPIO_DEFAULT_PLACE;
  char *cpio_end = CPIO_DEFAULT_PLACE_END;
  // char *heap_start = "200000";
  // char *heap_end = "300000";
  memory_reserve((unsigned long)spin_start, (unsigned long)spin_end);
  memory_reserve((unsigned long)heap_start, (unsigned long)heap_end);
  memory_reserve((unsigned long)kernel_start, (unsigned long)kernel_end);
  memory_reserve((unsigned long)CPIO_DEFAULT_PLACE,
                 (unsigned long)CPIO_DEFAULT_PLACE_END);
  memory_reserve((unsigned long)dtb_place,
                 (unsigned long)(dtb_place + dtb_len));
}