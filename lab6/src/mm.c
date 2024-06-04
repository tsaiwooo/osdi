#include "mm.h"

void map_page(thread* cur, uint64_t page, uint64_t va, int type)
{
    // thread* cur = (thread*)TO_KA((uint64_t)get_current());
    if (!cur->mm.pgd) {
        cur->mm.pgd = (uint64_t)kmalloc(PAGE_SIZE);
        cur->mm.kernel_pages[PGD] = cur->mm.pgd;
        cur->mm.kernel_pages_count = 1;
    }
    uart_printf("pgd = %x", cur->mm.pgd);
    int new_table;
    uint64_t pud = map_table((uint64_t*)TO_KA(cur->mm.pgd), PGD_SHIFT, va, &new_table);
    uart_printf(" pud = %x", pud);
    if (new_table && type == map_code) {
        cur->mm.kernel_pages[cur->mm.kernel_pages_count++] = TO_PA(pud);
    } else if (new_table && type == map_stack) {
        cur->mm.user_stack_pages[cur->mm.user_stack_count++] = TO_PA(pud);
    }
    uint64_t pmd = map_table((uint64_t*)TO_KA(pud), PUD_SHIFT, va, &new_table);
    uart_printf(" pmd = %x", pmd);
    if (new_table && type == map_code) {
        cur->mm.kernel_pages[cur->mm.kernel_pages_count++] = TO_PA(pmd);
    } else if (new_table && type == map_stack) {
        cur->mm.user_stack_pages[cur->mm.user_stack_count++] = TO_PA(pmd);
    }
    uint64_t pte = map_table((uint64_t*)TO_KA(pmd), PMD_SHIFT, va, &new_table);
    uart_printf(" pte = %x", pte);
    if (new_table && type == map_code) {
        cur->mm.kernel_pages[cur->mm.kernel_pages_count++] = TO_PA(pte);
    } else if (new_table && type == map_stack) {
        cur->mm.user_stack_pages[cur->mm.user_stack_count++] = TO_PA(pte);
    }
    map_table_entry((uint64_t*)TO_KA(pte), va, page);
    struct user_page p = { page, va };
    cur->mm.user_pages[cur->mm.user_pages_count++] = p;
}

uint64_t map_table(uint64_t* table, uint64_t shift, uint64_t va, int* new_table)
{
    uint64_t index = va >> shift;
    index = index & (PTRS_PER_TABLE - 1);
    if (!table[index]) {
        *new_table = 1;
        uint64_t next_level_table = (uint64_t)kmalloc(PAGE_SIZE);
        while(next_level_table >= (uint64_t)0x3C000000){
            next_level_table = (uint64_t)kmalloc(PAGE_SIZE);
        }
        clean_page_table(next_level_table);
        // uint64_t next_level_table = (uint64_t)kmalloc(PAGE_SIZE);
        uint64_t entry = next_level_table | PD_TABLE;
        table[index] = entry;
        return next_level_table;
    } else {
        *new_table = 0;
    }
    return table[index] & PAGE_MASK;
    // return TO_PA(table[index] & PAGE_MASK);
}

void map_table_entry(uint64_t* pte, uint64_t va, uint64_t pa)
{
    uint64_t index = va >> PAGE_SHIFT;
    index = index & (PTRS_PER_TABLE - 1);
    uint64_t entry = pa | PTE_FLAGS;
    pte[index] = entry;
    uart_printf(" index = %d, pte entry : %x\n", index, pte[index]);
}

void clean_page_table(uint64_t table)
{
    char *start_addr = (char *)TO_KA(table);
    for(int i=0; i<4096; ++i){
        start_addr[i] = 0;
    }
}