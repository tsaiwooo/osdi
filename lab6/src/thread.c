#include "thread.h"

void test_function()
{
    // thread* current_thread = (thread*)get_current();
    for (int i = 0; i < 10; ++i) {
        // uart_printf("Thread id: %d %d\n", current_thread->thread_id, i);
        uart_printf("Thread id: %d %d\n", getpid(), i);
        delay(100000000);
        schedule();
    }
    exit();
}

void init_thread()
{
    // thread* thread_;
    // for (int i = 0; i < MAX_THREADS; ++i) {
    // }
    thread_count = 0;
    thread* thread_;
    thread_ = (thread*)TO_KA((uint64_t)kmalloc(PAGE_SIZE));
    // thread_ = (thread*)kmalloc(sizeof(thread));
    // POSIX clear
    for (int i = 0; i < SIG_NUMS; ++i) {
        thread_->POSIX.signal_handler[i] = 0;
    }
    // 要加 or 不加？
    thread_->POSIX.signal_handler[SIGKILL] = (uint64_t)exit;
    // thread_->POSIX.signal_handler[SIGKILL] = (uint64_t)signal_kill;
    thread_->POSIX.posix_stack = (uint64_t)kmalloc(PAGE_SIZE) + PAGE_SIZE;
    // map_page(thread_, thread_->POSIX.posix_stack, 0xffffffffd000, map_stack);
    // thread_->POSIX.posix_stack = 0xffffffffe000;

    thread_->thread_id = thread_count++;
    thread_->status = RUN;
    thread_->prev = NULL;
    thread_->next = NULL;
    // give every thread a stack sapce
    // thread_->user_stack = (uint64_t)kmalloc(PAGE_SIZE) + (uint64_t)PAGE_SIZE;
    // thread_->sp_el0 = thread_->user_stack;
    // thread_->kernel_stack = (uint64_t)kmalloc(PAGE_SIZE) + (uint64_t)PAGE_SIZE;
    // thread_->sp_el1 = thread_->kernel_stack;
    thread_->kernel_stack = VIRTUAL_MEM_BASE | 0x80000;
    thread_->sp_el1 = thread_->kernel_stack;
    // thread_->user_stack = 0x60000;
    // thread_->sp_el0 = thread_->user_stack;
    thread_->function = (void*)0x80000;
    thread_queue_insert((thread**)&thread_queue, (thread**)&thread_);
    kernel_thread = thread_;
    thread_->signal = false;
    thread_->POSIX.mask = false;
    // virtual memory
    // extern uint64_t pg_dir;
    // asm volatile("msr tpidr_el1, %0" ::"r"(thread_queue));
    // thread_->mm.pgd = TO_KA(pg_dir);
    thread_->mm.pgd = (uint64_t)kmalloc(PAGE_SIZE);
    clean_page_table((uint64_t)thread_->mm.pgd);

    // asm volatile("mrs %0, ttbr0_el1":"=r"(thread_->mm.pgd));
    // uart_printf(".............................pgd = %x\n", pg_dir);
    thread_->mm.kernel_pages[PGD] = thread_->mm.pgd;
    thread_->mm.kernel_pages_count = 1;

    // char* aa = kmalloc(PAGE_SIZE);
    // thread_->kernel_stack = TO_KA((uint64_t)kmalloc(PAGE_SIZE)) + PAGE_SIZE;
    // thread_->sp_el1 = thread_->kernel_stack;
    thread_->user_stack = (uint64_t)kmalloc(PAGE_SIZE);
    // char* user_st = kmalloc(PAGE_SIZE);
    // map_page(thread_, (uint64_t)user_st, 0x0000ffffffffd000);
    // user_st = kmalloc(PAGE_SIZE);
    // map_page(thread_, (uint64_t)user_st, 0x0000ffffffffc000);
    // user_st = kmalloc(PAGE_SIZE);
    // map_page(thread_, (uint64_t)user_st, 0x0000ffffffffb000);
    thread_->mm.user_stack_count = 1;
    map_page(thread_, thread_->user_stack, 0xffffffffe000, map_stack);
    thread_->mm.user_stack_pages[0] = thread_->user_stack;
    thread_->user_stack = 0xfffffffff000;
    thread_->sp_el0 = thread_->user_stack;
    // mailbox
    // for (int i = 0; i < ((0x40000000 - 0x3C000000) / 0x1000) ; ++i) {
    //     // uart_printf("i = %d\n", i);
    //     // char* gpu_mem = kmalloc(PAGE_SIZE);
    //     // map_page(child, (uint64_t)gpu_mem, 0x3C000000 + i * 0x1000, 3);
    //     uart_printf("i = %d\n",i);
    //     map_page(thread_, (uint64_t)(0x3C000000 + i * 0x1000),(uint64_t)( 0x3C000000 + i * 0x1000), 3);
    // }
    // asm volatile("msr ttbr0_el1, %0" : : "r"(thread_->mm.pgd));

    // extern unsigned char _code_end;
    // set_pgd(thread_->mm.pgd);
    // for (int i = 0; i < ((uint64_t)_kernel_end - (uint64_t)&_kernel_start) / PAGE_SIZE; ++i) {
    //     char* user_code = kmalloc(PAGE_SIZE);
    //     memcpy((void*)TO_KA((uint64_t)(user_code)), (void*)((uint64_t)&_kernel_start + i * PAGE_SIZE), PAGE_SIZE);
    //     uart_printf("start_addr = %x\n", ((uint64_t)&_kernel_start + i * PAGE_SIZE));
    //     map_page(TO_PA((uint64_t)user_code), i << 12);
    // }

    // int pages = (TO_PA((uint64_t)&_code_end) - (uint64_t)0x7f000) / PAGE_SIZE;
    // for (int i = 127; i < 127 + pages; ++i) {
    //     uart_printf("start_addr = %x\n", ((uint64_t)&_kernel_start + i * PAGE_SIZE));
    //     map_page(TO_PA((uint64_t)0x7f000) + (i - 127) * 4096, i << 12);
    // }
    // map_page(thread_->user_stack, 0xfffffffff000);
    // thread_->user_stack = 0xfffffffff000;

    asm volatile("msr tpidr_el1, %0" ::"r"(thread_queue));
}

thread* thread_create(void (*function)())
{
    // thread* cur = (thread*)kmalloc(sizeof(thread));
    thread* thread_;
    thread_ = (thread*)TO_KA((uint64_t)kmalloc(PAGE_SIZE));
    // thread_ = (thread*)kmalloc(sizeof(thread));
    // POSIX clear
    for (int i = 0; i < SIG_NUMS; ++i) {
        thread_->POSIX.signal_handler[i] = 0;
    }
    thread_->POSIX.signal_handler[SIGKILL] = (uint64_t)exit_;
    // thread_->POSIX.signal_handler[SIGKILL] = (uint64_t)signal_kill;
    thread_->POSIX.posix_stack = (uint64_t)kmalloc(PAGE_SIZE) + PAGE_SIZE;
    // map_page(thread_, thread_->POSIX.posix_stack, 0xffffffffd000, map_stack);
    // thread_->POSIX.posix_stack = 0xffffffffe000;

    thread_->thread_id = thread_count++;
    thread_->status = RUN;
    thread_->prev = NULL;
    thread_->next = NULL;
    // give every thread a stack sapce
    thread_->kernel_stack = TO_KA((uint64_t)kmalloc(PAGE_SIZE)) + (uint64_t)PAGE_SIZE;
    thread_->sp_el1 = thread_->kernel_stack;
    thread_->function = function;
    // stack pointer start address
    thread_->fp = thread_->user_stack;

    // uart_printf("thread = %x\n", thread_);
    // uart_printf("thread user stack  %x\n", thread_->user_stack);
    // uart_printf("thread kernel stack %x\n", thread_->kernel_stack);
    // uart_printf("thread posix stack %x\n", thread_->POSIX.posix_stack);
    thread_->lr = (uint64_t)function;
    // thread_->lr = (uint64_t)from_el12el0;
    thread_->signal = false;
    thread_->POSIX.mask = false;
    // virtual memory
    thread_->mm.pgd = (uint64_t)kmalloc(PAGE_SIZE);
    clean_page_table((uint64_t)thread_->mm.pgd);
    thread_->mm.kernel_pages[PGD] = thread_->mm.pgd;
    thread_->mm.kernel_pages_count = 1;

    thread_->user_stack = (uint64_t)kmalloc(PAGE_SIZE);
    // thread_->user_stack = 0x60000;
    thread_->mm.user_stack_pages[0] = thread_->user_stack;
    thread_->mm.user_stack_count = 1;
    // map_page((thread*)TO_KA((uint64_t)thread_), thread_->user_stack, 0xffffffffe000, map_stack);
    // thread_->sp_el0 = 0xfffffffff000;
    // map_page(thread_->kernel_stack, 0xffffffffffffc000);
    thread_queue_insert((thread**)&thread_queue, (thread**)&thread_);

    return thread_;
}

void thread_queue_insert(thread** tail, thread** node)
{
    // uart_printf("init id = %d\n", (*node)->thread_id);
    if (!(*node))
        return;
    if (!(*tail)) {
        (*tail) = *node;
        (*tail)->prev = (*tail);
        (*tail)->next = (*tail);
        // uart_printf("in if init id = %d\n", (*node)->thread_id);

    } else {
        (*tail)->next->prev = *node;
        (*node)->next = (*tail)->next;
        (*node)->prev = (*tail);
        (*tail)->next = *node;
        (*tail) = *node;
        // uart_printf("in else init id = %d\n", (*node)->thread_id);
        // uart_printf("tail id = %d\n", thread_queue->thread_id);
    }
}

void degub_info()
{
    int count = MAX_THREADS;
    while (count--) {
        uart_printf("id = %d\n", thread_queue->next->thread_id);
        thread_queue = thread_queue->next;
    }
}

// delete has bugs, link
thread* thread_queue_delete(thread** tail, thread** cur)
{
    // thread* node = cur;
    // uart_printf("delete node id = %d\n", cur->thread_id);
    if ((*cur) == NULL)
        return NULL;
    // if ((*cur) == (*tail))
    //     return NULL;
    if ((*cur) == (*cur)->next) {
        (*tail) = NULL;
    } else {
        thread* next = (*cur)->next;
        (*cur)->prev->next = (*cur)->next;
        // (*cur)->prev = next->prev;
        next->prev = (*cur)->prev;

        // next->prev = node->prev;

        // thread_queue->next = node->next;
    }
    (*tail) = (*cur)->prev;
    (*cur)->prev = NULL;
    (*cur)->next = NULL;
    return (*cur);
}

thread* thread_queue_find(int id)
{
    thread* cur = (thread*)TO_KA((uint64_t)get_current());
    thread* tail = thread_queue;
    if (tail->thread_id == id)
        return tail;
    while (cur != tail) {
        // uart_printf("id = %d\n", cur->thread_id);
        if (cur->thread_id == id) {
            // uart_printf("in\n");
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}

void schedule()
{
    thread* cur = (thread*)TO_KA((uint64_t)get_current());
    // if (!cur->is_user) {
    //     from_el12el0();
    //     cur->is_user = true;
    // }
    // thread* node = thread_queue_delete(thread_queue->next);
    // uart_printf("cur->id = %d\n", cur->thread_id);
    thread* next = (thread*)TO_KA((uint64_t)cur->next);
    while (next->status == ZOMBIE)
        next = (thread*)TO_KA((uint64_t)next->next);
    if (next == cur) {
        // do_signal();
        return;
    }
    thread* node = thread_queue_delete((thread**)&thread_queue, (thread**)&cur);
    thread_queue_insert((thread**)&thread_queue, (thread**)&node);
    // uart_printf("exit1\n");
    // delay(10000000);
    // uart_printf("cur_id = %d, next_id = %d\n", cur->thread_id, next->thread_id);
    do_signal();
    set_pgd(next->mm.pgd);
    switch_to(cur, next);
    // if (cur->signal) {
    // }
    // switch_to(cur, thread_queue->next->next);
}

void idle()
{
    while (1) {
        kill_zombies();
        schedule();
    }
}

void kill_zombies()
{
    // thread* cur = zombie_queue;
    thread* cur = get_current();

    // int total_threads = thread_count;
    while (cur != thread_queue) {
        // if (cur->status == ZOMBIE) {
        // thread_queue_delete((thread**)&zombie_queue, (thread**)&cur);
        if (cur->status == ZOMBIE) {
            thread* kill_thread = thread_queue_delete((thread**)&thread_queue, (thread**)&cur);
            thread_queue_insert((thread**)&zombie_queue, (thread**)&kill_thread);
        }
        // }
        cur = cur->next;
    }
    // disable_interrupt();
    thread* zombie_cur = zombie_queue;
    while (zombie_cur) {
        kfree((char*)zombie_cur->kernel_stack);
        kfree((char*)zombie_cur->user_stack);
        thread* tmp = zombie_cur;
        zombie_cur = zombie_cur->next;
        kfree((char*)tmp);
    }
    // enable_interrupt();
}

void scheduler_timer()
{
    unsigned long cntfrq_el0;
    asm volatile("mrs %0, cntfrq_el0" : "=r"(cntfrq_el0));
    asm volatile("msr cntp_tval_el0, %0" : : "r"(cntfrq_el0 >> 5));
}

void test1(int a, int b)
{
    static int c = 1;
    while (1) {
        c += b;
        delay(10000000);
        uart_printf("cur_thread = %d, c = %d\n", getpid(), c);
    }
}

void fork_test()
{
    uart_printf("\nFork Test, pid %d\n", getpid());
    // signal(SIG_KILL, (uint64_t)&kill_);
    int cnt = 2;
    int ret = 0;
    if ((ret = fork()) == 0) { // child
        long long cur_sp;
        asm volatile("mov %0, sp" : "=r"(cur_sp));
        uart_printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
        ++cnt;

        if ((ret = fork()) != 0) {
            asm volatile("mov %0, sp" : "=r"(cur_sp));
            uart_printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
        } else {
            while (cnt < 5) {
                asm volatile("mov %0, sp" : "=r"(cur_sp));
                uart_printf("second child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
                delay(1000000);
                ++cnt;
            }
        }
        exit();
    } else {
        long long cur_sp;
        asm volatile("mov %0, sp" : "=r"(cur_sp));
        uart_printf("parent here, pid %d, child %d, parent sp : 0x%x\n", getpid(), ret, cur_sp);
    }

    // if ((ret = fork()) == 0) { // child
    //     uart_printf("i am child\n");
    //     // while (1) {
    //     // uart_printf("child, id = %d\n", getpid());

    //     // delay(100000000);
    //     // for (int i = 0; i < 2; ++i) {
    //     //     fork();
    //     // }
    //     while (1) {
    //         // uart_printf("child, id = %d\n", getpid());
    //         // uart_printf("I am child in while\n");
    //         delay(10000000);
    //     }

    //     // test1(1, 2);

    //     // }
    // } else {
    //     uart_printf("parent back, child id = %d\n", ret);

    //     // test1(4, 6);
    //     delay(100000000);
    //     kill(1, SIG_KILL);
    //     uart_printf("signal return\n");
    //     // while(1);
    //     //     delay(10000000);
    //     //     uart_printf("1111111\n");
    //     // }
    // }
}
