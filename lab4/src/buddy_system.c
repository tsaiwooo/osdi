#include "buddy_system.h"
char *buddy_start = (char *)&_buddy_start;
char *buddy_end = (char *)&_buddy_end;

void init_buddy() {
  // init total page array
  for (int i = 0; i < total_pages; ++i) {
    page_array[i].condition = FREE;
    page_array[i].order = -1;
  }
  // init free list of any order
  for (int i = 0; i < MAX_ORDER; ++i) {
    buddy.free_area[i].nr_free = 0;
    buddy.free_area[i].order = i;
    buddy.free_area[i].list = NULL;
  }

  Node *tmp = simple_malloc(sizeof(Node));
  tmp->index = 0;
  tmp->next = NULL;
  buddy.free_area[MAX_ORDER].list = tmp;
  buddy.free_area[MAX_ORDER].nr_free = 1;
  buddy.free_area[MAX_ORDER].order = MAX_ORDER;
}

void *kmalloc(size_t size) {
  uart_printf("before malloc, the free number of each lists\n");
  for (int i = 0; i <= MAX_ORDER; ++i) {
    uart_printf("%d order free number = %d\n", i, buddy.free_area[i].nr_free);
  }
  if (size > (page_size * (1 << MAX_ORDER))) {
    uart_printf("size is not enough\n");
  }
  // find the fit order
  int order = 0;
  while ((page_size * (1 << order)) < size) {
    // order *= 2;
    order++;
  }

  // go to find the free space in the free list according to its order
  if (buddy.free_area[order].nr_free == 0) {
    int order_gap = 1;
    while (buddy.free_area[order + order_gap].nr_free == 0) {
      order_gap++;
    }
    // uart_printf("order_gap = %d , cur_order = %d\n", order_gap,
    //             order + order_gap);

    // split and set buddy
    int now_page_index = (buddy.free_area[order + order_gap].list)->index;
    page_array[now_page_index].condition = ALLOCATED;
    page_array[now_page_index].order = order;
    // current free block--, and list set to next free block
    buddy.free_area[order + order_gap].nr_free--;
    buddy.free_area[order + order_gap].list =
        buddy.free_area[order + order_gap].list->next;
    // handle the split page
    for (int cur_order = (order + order_gap); cur_order > order; --cur_order) {
      int buddy_index = now_page_index ^ (1 << (cur_order - 1));
      // split into two blocks and add it to free lists according to its order
      Node *tmp = simple_malloc(sizeof(Node)),
           *cur = buddy.free_area[cur_order - 1].list;
      // page array change the order according to the page
      page_array[buddy_index].order = cur_order - 1;
      buddy.free_area[cur_order - 1].nr_free++;
      tmp->index = buddy_index;
      tmp->next = NULL;
      if (buddy.free_area[cur_order - 1].list == NULL) {
        buddy.free_area[cur_order - 1].list = tmp;
        // uart_printf("head index = %d\n",
        //             buddy.free_area[cur_order - 1].list->index);
      } else {
        while (cur->next != NULL) {
          cur = cur->next;
        }
        cur->next = tmp;
      }
    }
    uart_printf(
        "after malloc, the free number of each lists and now_page_idx = %d\n",
        now_page_index);
    for (int i = 0; i <= MAX_ORDER; ++i) {
      uart_printf("%d order free number = %d\n", i, buddy.free_area[i].nr_free);
    }
    // uart_printf("now_page_idx = %d\n", now_page_index * page_size);
    return buddy_start + now_page_index * page_size;
  } else {
    buddy.free_area[order].nr_free--;
    Node *node;
    Node *cur = buddy.free_area[order].list;
    node = cur;
    page_array[cur->index].condition = ALLOCATED;
    page_array[cur->index].order = order;
    // uart_printf("in else %d order %d index\n", order, cur->index);
    // find the free node index
    // if(cur->index == cur->index){
    //   buddy.free_area[order].list = buddy.free_area[order].list->next;
    // }else{
    // while(cur->index!=)
    // }
    cur = cur->next;
    buddy.free_area[order].list = cur;
    uart_printf("after malloc, the free number of each lists = %d\n",
                node->index);
    for (int i = 0; i <= MAX_ORDER; ++i) {
      uart_printf("%d order free number = %d\n", i, buddy.free_area[i].nr_free);
    }
    // uart_printf("now_page_idx = %x\n", node->index * page_size);
    return buddy_start + node->index * page_size;
  }
}

void kfree(char *address) {
  int cur_index = (address - buddy_start) / (page_size);
  int cur_order = page_array[cur_index].order;
  page_array[cur_index].condition = FREE;
  // when free, add 1 in current order
  buddy.free_area[cur_order].nr_free++;
  while (cur_order < MAX_ORDER) {
    int buddy_index = (cur_index ^ (1 << (cur_order)));
    uart_printf("current index = %d, buddy_index = %d\n", cur_index,
                buddy_index);
    // // buddy is allocated, do nothing
    // if (page_array[buddy_index].condition != FREE ||
    //     !match_freelist(cur_order, buddy_index)) {

    if (page_array[buddy_index].condition != FREE ||
        page_array[buddy_index].order != cur_order) {
      page_array[cur_index].order = cur_order;
      // insert it to list
      // buddy.free_area[cur_order].nr_free++;
      Node *tmp = simple_malloc(sizeof(Node));
      tmp->index = cur_index;
      tmp->next = NULL;
      Node *cur = buddy.free_area[cur_order].list;
      if (cur == NULL) {
        buddy.free_area[cur_order].list = tmp;
      } else {
        while (cur->next != NULL) {
          cur = cur->next;
        }
        cur->next = tmp;
      }
      break;
    } else {
      // free nodes in current order--
      buddy.free_area[cur_order].nr_free -= 2;
      Node *cur = buddy.free_area[cur_order].list;
      // delete free node in cur_order
      // if head match, head = head->next
      if (cur->index == buddy_index) {
        buddy.free_area[cur_order].list = buddy.free_area[cur_order].list->next;
      } else {
        Node *pre;
        // not head, then pre->next = cur->next
        while (cur != NULL) {
          if (cur->index == buddy_index) {
            pre->next = cur->next;
            break;
          }
          pre = cur;
          cur = cur->next;
        }
      }
      // dont add to next order, it may be merged in next order
      // next order free nodes++, it's wrong, so don't add 1
      // find merge, add to next
      buddy.free_area[cur_order + 1].nr_free++;
      // add new node to next order
      Node *next_order_cur = buddy.free_area[cur_order + 1].list;
      Node *next_tmp = simple_malloc(sizeof(Node));
      next_tmp->index = cur_index;
      next_tmp->next = NULL;
      if (next_order_cur == NULL) {
        buddy.free_area[cur_order + 1].list = next_tmp;
      } else {
        while (next_order_cur->next != NULL) {
          next_order_cur = next_order_cur->next;
        }
        next_order_cur->next = next_tmp;
      }
    }
    cur_order++;
  }
  uart_printf("===================================\nafter free function\n");
  for (int i = 0; i <= MAX_ORDER; ++i) {
    uart_printf("%d order free number = %d\n", i, buddy.free_area[i].nr_free);
  }
}

// int match_freelist(int order, int buddy_index) {
//   Node *cur = buddy.free_area[order].list;
//   while (cur) {
//     if (cur->index == buddy_index) {
//       return 1;
//     }
//   }
//   return 0;
// }