#ifndef __THREAD_H_
#define __THREAD_H_
#include "buddy_system.h"
#include "exception.h"
#include "function.h"
#include "malloc.h"
#include "posix.h"
#include "tmpfs.h"
#include "uart.h"
#include "vfs.h"
#define NULL 0
#define uint64_t unsigned long long
#define MAX_THREADS 3
#define PAGE_SIZE 4096
#define MAX_FD_SIZE 16
#define true 1
#define false 0
enum status {
    RUN,
    IDLE,
    ZOMBIE,
    POSIX
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
    struct file* fd[MAX_FD_SIZE];
};

typedef struct trap_frame trap_frame;
struct trap_frame {
    uint64_t x[31];
    uint64_t spsr_el1;
    uint64_t elr_el1;
    uint64_t sp_el0;
};

void scheduler_timer();
// store the tail of the queue
thread *thread_queue, *zombie_queue, *kernel_thread;
unsigned int thread_count;
void degub_info();
extern void switch_to(thread* prev, thread* next);
extern void* get_current();
extern void return_to_user();
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