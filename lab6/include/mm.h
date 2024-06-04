#ifndef __MM_H_
#define __MM_H_
#include "buddy_system.h"
#include "exception.h"
#include "mmu.h"
#include "system_call.h"
#include "thread.h"
#include "uart.h"
typedef struct thread thread;
#define VIRTUAL_MEM_BASE 0xFFFF000000000000
#define TO_KA(x) (x | 0xFFFF000000000000)
#define TO_PA(x) (x & 0x0000FFFFFFFFFFFF)
#define PHYS_MEMORY_SIZE 0x40000000

#define PAGE_MASK 0xfffffffffffff000

#define PAGE_SHIFT 12
#define TABLE_SHIFT 9
#define SECTION_SHIFT (PAGE_SHIFT + TABLE_SHIFT)
#define PAGE_SIZE 4096
#define SECTION_SIZE (1 << SECTION_SHIFT)
#define PTRS_PER_TABLE (1 << TABLE_SHIFT)

#define PGD_SHIFT PAGE_SHIFT + 3 * TABLE_SHIFT
#define PUD_SHIFT PAGE_SHIFT + 2 * TABLE_SHIFT
#define PMD_SHIFT PAGE_SHIFT + TABLE_SHIFT
#define PTE_SHIFT PAGE_SHIFT
#define PAGE_MASK 0xfffffffffffff000
#define CODE_MASK 0x0000000000000fff

#define DIS 0

// #define MAX_PROCESS_PAGES 128
#define uint64_t unsigned long long

extern void set_pgd(uint64_t pgd);
extern uint64_t get_pgd();

void map_page(thread* cur, uint64_t page, uint64_t va, int type);
uint64_t map_table(uint64_t* table, uint64_t shift, uint64_t va, int* new_table);
void map_table_entry(uint64_t* pte, uint64_t va, uint64_t pa);
void clean_page_table(uint64_t table);
#endif // __MM_H_