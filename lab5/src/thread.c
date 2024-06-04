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
    thread_ = (thread*)kmalloc(PAGE_SIZE);
    // thread_ = (thread*)kmalloc(sizeof(thread));
    // POSIX clear
    for (int i = 0; i < SIG_NUMS; ++i) {
        thread_->POSIX.signal_handler[i] = 0;
    }
    // 要加 or 不加？
    thread_->POSIX.signal_handler[SIGKILL] = (uint64_t)exit;
    // thread_->POSIX.signal_handler[SIGKILL] = (uint64_t)signal_kill;
    thread_->POSIX.posix_stack = (uint64_t)kmalloc(PAGE_SIZE) + PAGE_SIZE;
    thread_->thread_id = thread_count++;
    thread_->status = RUN;
    thread_->prev = NULL;
    thread_->next = NULL;
    // give every thread a stack sapce
    // thread_->user_stack = (uint64_t)kmalloc(PAGE_SIZE) + (uint64_t)PAGE_SIZE;
    // thread_->sp_el0 = thread_->user_stack;
    // thread_->kernel_stack = (uint64_t)kmalloc(PAGE_SIZE) + (uint64_t)PAGE_SIZE;
    // thread_->sp_el1 = thread_->kernel_stack;
    thread_->kernel_stack = 0x80000;
    thread_->sp_el1 = thread_->kernel_stack;
    thread_->user_stack = 0x60000;
    thread_->sp_el0 = thread_->user_stack;
    thread_->function = (void*)0x80000;
    // stack pointer start address
    // thread_->fp = thread_->user_stack;
    // thread_->lr = (uint64_t)0x80000;
    // asm volatile("mov %0, lr" : "=r"(thread_->lr) :);
    // thread_->lr = (uint64_t)shell;
    thread_queue_insert((thread**)&thread_queue, (thread**)&thread_);
    kernel_thread = thread_;
    thread_->signal = false;
    thread_->POSIX.mask = false;
    asm volatile("msr tpidr_el1, %0" ::"r"(thread_queue));
    // asm volatile("msr tpidr_el1, %0" ::"r"(kernel_thread));
}

thread* thread_create(void (*function)())
{
    // thread* cur = (thread*)kmalloc(sizeof(thread));
    thread* thread_;
    thread_ = (thread*)kmalloc(PAGE_SIZE);
    // thread_ = (thread*)kmalloc(sizeof(thread));
    // POSIX clear
    for (int i = 0; i < SIG_NUMS; ++i) {
        thread_->POSIX.signal_handler[i] = 0;
    }
    thread_->POSIX.signal_handler[SIGKILL] = (uint64_t)exit_;
    // thread_->POSIX.signal_handler[SIGKILL] = (uint64_t)signal_kill;
    thread_->POSIX.posix_stack = (uint64_t)kmalloc(PAGE_SIZE) + (uint64_t)PAGE_SIZE;
    thread_->thread_id = thread_count++;
    thread_->status = RUN;
    thread_->prev = NULL;
    thread_->next = NULL;
    // give every thread a stack sapce
    thread_->user_stack = (uint64_t)kmalloc(PAGE_SIZE) + (uint64_t)PAGE_SIZE;
    thread_->sp_el0 = thread_->user_stack;
    thread_->kernel_stack = (uint64_t)kmalloc(PAGE_SIZE) + (uint64_t)PAGE_SIZE;
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
    // void (*el12el0)(uint64_t, uint64_t) = (void (*)(thread_->user_stack, function))from_el12el0;
    // thread_->lr = *(uint64_t)from_el12el0(thread_->user_stack, (uint64_t)function);
    thread_queue_insert((thread**)&thread_queue, (thread**)&thread_);
    // asm volatile("msr tpidr_el1, %0" ::"r"(thread_queue));

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
    thread* cur = get_current();
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
    thread* cur = (thread*)get_current();
    // if (!cur->is_user) {
    //     from_el12el0();
    //     cur->is_user = true;
    // }
    // thread* node = thread_queue_delete(thread_queue->next);
    // uart_printf("cur->id = %d\n", cur->thread_id);
    thread* next = cur->next;
    while (next->status == ZOMBIE)
        next = next->next;
    thread* node = thread_queue_delete((thread**)&thread_queue, (thread**)&cur);
    thread_queue_insert((thread**)&thread_queue, (thread**)&node);
    // uart_printf("exit1\n");
    // delay(10000000);
    // uart_printf("cur_id = %d, next_id = %d\n", cur->thread_id, next->thread_id);
    do_signal();
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
    signal(SIG_KILL, (uint64_t)&kill_);
    // int cnt = 1;
    int ret = 0;
    // if ((ret = fork()) == 0) { // child
    //     long long cur_sp;
    //     asm volatile("mov %0, sp" : "=r"(cur_sp));
    //     uart_printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
    //     ++cnt;

    //     if ((ret = fork()) != 0) {
    //         asm volatile("mov %0, sp" : "=r"(cur_sp));
    //         uart_printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
    //     } else {
    //         while (cnt < 5) {
    //             asm volatile("mov %0, sp" : "=r"(cur_sp));
    //             uart_printf("second child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
    //             delay(1000000);
    //             ++cnt;
    //         }
    //     }
    //     exit();
    // } else {
    //     long long cur_sp;
    //     asm volatile("mov %0, sp" : "=r"(cur_sp));
    //     uart_printf("parent here, pid %d, child %d, parent sp : 0x%x\n", getpid(), ret, cur_sp);
    // }

    if ((ret = fork()) == 0) { // child
        uart_printf("i am child\n");
        // while (1) {
        // uart_printf("child, id = %d\n", getpid());

        // delay(100000000);
        // for (int i = 0; i < 2; ++i) {
        //     fork();
        // }
        while (1) {
            // uart_printf("child, id = %d\n", getpid());
            // uart_printf("I am child in while\n");
            delay(10000000);
        }

        // test1(1, 2);

        // }
    } else {
        uart_printf("parent back, child id = %d\n", ret);

        // test1(4, 6);
        delay(100000000);
        kill(1, SIG_KILL);
        uart_printf("signal return\n");
        // while(1);
        //     delay(10000000);
        //     uart_printf("1111111\n");
        // }
    }
}
