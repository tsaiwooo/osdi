#ifndef __THREAD_H_
#define __THREAD_H_
#include "buddy_system.h"
#include "cpio.h"
#include "exception.h"
#include "function.h"
#include "malloc.h"
#include "mm.h"
#include "posix.h"
#include "reserve_memory.h"
#include "uart.h"
#define NULL 0
#define uint64_t unsigned long long
#define MAX_THREADS 3
#define PAGE_SIZE 4096
#define true 1
#define false 0
#define MAX_PROCESS_PAGES 1024

#define map_stack 1
#define map_code 2
enum status {
    RUN,
    IDLE,
    ZOMBIE,
    POSIX
};

typedef struct user_page user_page;
typedef struct mm_struct mm_struct;

struct user_page {
    uint64_t phys_addr;
    uint64_t virt_addr;
};

enum page_state {
    PGD,
    PUD,
    PMD,
    PTE
};

struct mm_struct {
    uint64_t pgd;
    int user_pages_count;
    struct user_page user_pages[MAX_PROCESS_PAGES];
    int kernel_pages_count;
    uint64_t kernel_pages[MAX_PROCESS_PAGES];
    uint64_t user_stack_pages[MAX_PROCESS_PAGES];
    int user_stack_count;
};

typedef struct thread thread;
struct thread {
    uint64_t x19;
    uint64_t x20;
    uint64_t x21;
    uint64_t x22;
    uint64_t x23;
    uint64_t x24;
    uint64_t x25;
    uint64_t x26;
    uint64_t x27;
    uint64_t x28;
    // x29
    uint64_t fp;
    // x30
    uint64_t lr;
    uint64_t sp_el0;
    uint64_t sp_el1;
    uint64_t user_stack;
    uint64_t kernel_stack;
    posix POSIX;
    int thread_id;
    void (*function)(void);
    enum status status;
    thread* next;
    thread* prev;
    unsigned short signal;
    struct mm_struct mm;
};

typedef struct trap_frame trap_frame;
struct trap_frame {
    uint64_t x[31];
    uint64_t spsr_el1;
    uint64_t elr_el1;
    uint64_t sp_el0;
};

extern volatile uint64_t _code_size;
extern unsigned char _kernel_start;

void scheduler_timer();
// store the tail of the queue
thread *thread_queue, *zombie_queue, *kernel_thread;
unsigned int thread_count;
void degub_info();
extern void switch_to(thread* prev, thread* next);
extern void* get_current();
extern void return_to_user();
extern void set_pgd(uint64_t pgd);
thread* thread_create(void (*function)());
void schedule();
void idle();
void init_thread();
void thread_queue_insert(thread** tail, thread** node);
thread* thread_queue_delete(thread** tail, thread** cur);
thread* thread_queue_find(int);
void fork_test();
void test_function();
void kill_zombies();

#endif // __THREAD_H_