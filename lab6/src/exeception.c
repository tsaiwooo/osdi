#include "exception.h"

void exeception_handle()
{
    unsigned long long spsr_el1, elr_el1, esr_el1;
    unsigned int number;
    asm volatile("mrs %0, elr_el1\n\t"
                 "mrs %1, spsr_el1\n\t"
                 "mrs %2, esr_el1\n\t"
                 "mov %3, x0\n\t"
                 : "=r"(elr_el1), "=r"(spsr_el1), "=r"(esr_el1), "=r"(number)
                 :
                 :);
    uart_printf("spsr_el1 = 0x%x\n", spsr_el1);
    uart_printf("elr_el1 = 0x%x\n", elr_el1);
    uart_printf("esr_el1 = 0x%x\n", esr_el1);
    uart_printf("exeception number = %d\n", number);
    // if(number==8 || number==5){
    //     unsigned int count, freq;
    //     asm volatile("mrs  %[result], cntpct_el0": [result]"=r"(count));
    //     asm volatile("mrs  %[result], cntfrq_el0": [result]"=r"(freq));
    //     int time = (count) / freq;
    //     uart_printf("[%d] core timer interrupt\n", time);
    //     set_expired_time(2);
    //     // asm volatile("b ")
    // }else{
    while (1)
        ;
    // }
}

void svc_handler(int number, uint64_t ptr, uint64_t esr)
{
    // core_timer_disable();
    int svc_type = esr >> 26;
    // uart_printf("svc_type = %d\n", svc_type);
    switch (svc_type) {
    case 0x15: {
        // disable_interrupt();
        trap_frame* trap_ptr = (trap_frame*)ptr;

        // uart_printf("number = %d\n", trap_ptr->x[8]);
        int type = 1;
        asm volatile("mov   %0, x8" : "=r"(type) :);
        // uart_printf("original pid = %d\n", trap_ptr->x0);
        // uart_printf("in svc handler and tpye = %d\n", type);
        switch (type) {
        case GETPID: {
            thread* cur = (thread*)get_current();
            // thread* cur = (thread*)TO_KA((uint64_t)get_current());
            ((trap_frame*)trap_ptr)->x[0] = cur->thread_id;
            // asm volatile("mov x0, %0" : : "r"(trap_ptr->x0));
            // uart_printf("id = %d\n", cur->thread_id);
            break;
        }
        case READ:
            // sys_read((trap_frame *)ptr);
            sys_read(ptr);
            break;
        case WRITE:
            sys_write(ptr);
            break;
        case EXEC:
            exec_function(ptr);
            break;
        case FORK:
            fork_exec(ptr);
            break;
        case EXIT:
            exit_();
            break;
        case MBOX: {
            thread* cur = get_current();
            unsigned int* mbox_addr = (unsigned int*)(trap_ptr->x[1] - 0xffffffffe000 + cur->mm.user_stack_pages[0]);
            int val = sys_mailbox((unsigned char)trap_ptr->x[0], (unsigned int*)TO_KA((uint64_t)mbox_addr));
            // unsigned int *tmp_buf = (unsigned int *)TO_KA((uint64_t)kmalloc(PAGE_SIZE));
            // for(int i=0; i<144; ++i){
            //     ((char *)tmp_buf)[i] = ((char *)mbox)[i];
            // }
            // int val = sys_mailbox(ch, tmp_buf);

            // for(int i=0; i<144; ++i){
            //     ((char *)mbox)[i] = ((char *)tmp_buf)[i];
            // }

            register int x0 asm("x0") __attribute__((unused)) = val;
            // sys_mbox((unsigned char)trap_ptr->x[0], (unsigned int*)trap_ptr->x[1]);
            break;
        }
        case KILL:
            kill_(ptr);
            break;
        case SIGREG:
            signal_(ptr);
            break;
        case SIG_KILL:
            // disable_interrupt();
            // enable_interrupt();
            sig_handler(ptr);
            break;
        case SIG_CALL:
            sig_call(ptr);
            break;
        default:
            uart_printf("no support type system call = %d\n", type);
            break;
        }
        break;
    }
    default:
        uart_printf("Exception occurse\n");
        uart_printf("ESR: 0x%x\n", esr);
        while (1)
            ;
        break;
    }
    // core_timer_enable();
}

void enable_interrupt() { asm volatile("msr DAIFClr, 0xf\n\t"); }

void disable_interrupt() { asm volatile("msr DAIFSet, 0xf\n\t"); }

void exec_function(uint64_t ptr)
{
    trap_frame* trap_ptr = (trap_frame*)ptr;
    char* function;
    // char* function_ptr = function;
    // char** argv = NULL;
    function = (char*)(trap_ptr->x[0]);
    // argv = (char**)trap_ptr->x[1];
    // asm volatile("mov %0, x0" : "=r"((char*)function) :);
    // asm volatile("mov %0, x1" : "=r"(argv) :);

    char* data;
    struct cpio_newc_header* header = (struct cpio_newc_header*)TO_KA((uint64_t)CPIO_DEFAULT_PLACE);
    char* filename;
    unsigned long filesize = 0;
    // uart_printf("%d filename = %s\n", trap_ptr->x[8], (char*)trap_ptr->x[0]);
    while (header != 0) {
        int finish = cpio_parse_header(header, (const char**)&filename, &filesize, &data, &header);
        if (finish)
            break;
        if (!strcmp(function, filename)) {
            // for (int i = 0; i < (filesize / PAGE_SIZE + 1); ++i) {
            //     char* user_code = kmalloc(PAGE_SIZE);
            //     // memcpy((void*)TO_KA((uint64_t)(user_code)), (void*)(&data + i * PAGE_SIZE), PAGE_SIZE);
            //     for (unsigned long x = 0; x < PAGE_SIZE; x++) {
            //         user_code[x] = *data++;
            //         // uart_printf("%d %d\n", user_code[x], *(data - 1));
            //     }
            //     uart_printf("start_addr = %x\n", i * PAGE_SIZE);
            //     map_page(TO_PA((uint64_t)user_code), i << 12);
            // }
            // for (unsigned long i = 0; i < filesize; i++) {
            //     user_addr[i] = *data++;
            // }
            // thread* new_process = (thread*)TO_PA((uint64_t)thread_create((void*)0));

            uart_printf("size = %d\n", filesize);
            // thread* cur = (thread*)TO_KA((uint64_t)get_current());
            thread* cur = (thread*)get_current();
            cur->mm.pgd = (uint64_t)kmalloc(PAGE_SIZE);
            cur->mm.kernel_pages[PGD] = cur->mm.pgd;
            cur->mm.kernel_pages_count = 1;
            for (int i = 0; i < (filesize / PAGE_SIZE + 1); ++i) {
                char* user_code = (char*)TO_KA((uint64_t)kmalloc(PAGE_SIZE));
                // memcpy((void*)TO_KA((uint64_t)(user_code)), (void*)(&data + i * PAGE_SIZE), PAGE_SIZE);
                for (unsigned long x = 0; x < PAGE_SIZE; x++) {
                    user_code[x] = *data++;
                    // uart_printf("%d %d\n", user_code[x], *(data - 1));
                }
                uart_printf("start_addr = %x\n", i * PAGE_SIZE);
                map_page(cur, TO_PA((uint64_t)user_code), i << 12, map_code);
            }
            // uart_printf("user_st = %x, pgd = %x\n", cur->user_stack, cur->mm.pgd);
            cur->user_stack = (uint64_t)kmalloc(PAGE_SIZE);
            map_page(cur, (uint64_t)cur->user_stack, 0xffffffffe000, map_code);
            cur->user_stack = 0xfffffffff000;
            // map_page(cur, cur->POSIX.posix_stack, 0xffffffffd000, map_stack);
            // cur->POSIX.posix_stack = 0xffffffffd000;
            asm volatile(
                "mov x3, #0\n\t"
                "msr spsr_el1, x3\n\t"
                "msr elr_el1, %[data]\n\t"
                // "msr sp_el0, %[st]\n\t"
                "msr sp_el0, %[stack_ptr]\n\t"
                "dsb     ish\n\t" // ensure write has completed
                "msr     ttbr0_el1, %[pgd]\n\t" // switch translation based address.
                "tlbi    vmalle1is\n\t" // invalidate all TLB entries
                "dsb     ish\n\t" // ensure completion of TLB invalidatation
                "ldr x4, =exception_vector_table\n\t"
                "msr vbar_el1,x4\n\t"
                "mov    sp, %[kernel_ptr]\n\t"
                "eret"
                :
                // : [data] "r"(user_addr), [stack_ptr] "r"(user_sp));
                // : [data] "r"(0), [stack_ptr] "r"(cur->user_stack), [pgd] "r"(cur->mm.pgd));
                : [data] "r"(0), [stack_ptr] "r"(cur->user_stack), [pgd] "r"(cur->mm.pgd), [kernel_ptr] "r"(cur->kernel_stack));
        }
    }

    asm volatile("mov x0, %0" : : "r"(-1));
}

void fork_exec(uint64_t ptr)
{
    trap_frame* trap_ptr = (trap_frame*)ptr;
    // thread* parent = (thread*)TO_KA((uint64_t)get_current());
    thread* parent = (thread*)get_current();
    // uint64_t function_addr;
    // asm volatile("mov %0, lr" : "=r"(function_addr) :);
    thread* child = (thread*)thread_create((void*)0);
    // thread* child = (thread*)TO_KA((uint64_t)thread_create((void*)0));
    // uart_printf("exit2\n");
    // POSIX
    for (int i = 0; i <= SIG_NUMS; ++i) {
        child->POSIX.signal_handler[i] = parent->POSIX.signal_handler[i];
    }
    // stack
    // for (int i = 1; i <= PAGE_SIZE; ++i) {
    //     // user_stack in user space -> change to kernel space
    //     *(char*)TO_KA((uint64_t)(child->user_stack - i)) = *(char*)TO_KA((uint64_t)(parent->user_stack - i));
    // }
    // kernel stack複製不過去
    // uart_printf("child_id = %d, kernel_stack = %x, parent_kernel_stack = %x\n",child->thread_id,TO_PA(child->kernel_stack),parent->kernel_stack);
    for (int i = 1; i <= PAGE_SIZE; ++i) {
        *(char*)TO_KA((uint64_t)(child->kernel_stack - i)) = *(char*)TO_KA((uint64_t)(parent->kernel_stack - i));
        // memcpy((void *)TO_KA(child->kernel_stack), (void *)TO_KA(parent->kernel_stack), PAGE_SIZE);
        // uart_printf("src = %x, dst = %x\n", *(char*)TO_KA((uint64_t)(parent->kernel_stack - i)), *(char*)TO_KA((uint64_t)(child->kernel_stack - i)));
    }
    child->x19 = parent->x19;
    child->x20 = parent->x20;
    child->x21 = parent->x21;
    child->x22 = parent->x22;
    child->x23 = parent->x23;
    child->x24 = parent->x24;
    child->x25 = parent->x25;
    child->x26 = parent->x26;
    child->x27 = parent->x27;
    child->x28 = parent->x28;
    // uint64_t user_fp_offset = parent->user_stack - parent->fp;
    // child->fp = child->user_stack - user_fp_offset;
    // uint64_t kernel_fp_offset = parent->kernel_stack - trap_ptr->x[29];

    // child->fp = parent->fp;
    child->mm.kernel_pages_count = 4;
    child->mm.user_pages_count = 4;
    copy_vm(child, parent);
    if (child->thread_id == 1) {
        for (int i = 0; i < ((0x40000000 - 0x3C000000) / 4096); ++i) {
            uart_printf("i = %d\n", i);
            // char* gpu_mem = kmalloc(PAGE_SIZE);
            // map_page(child, (uint64_t)gpu_mem, 0x3C000000 + i * 0x1000, 3);
            map_page(child, (uint64_t)(0x3C000000LL + i * 0x1000), (uint64_t)(0x3C000000LL + i * 0x1000), 3);
            // map_page(parent, (uint64_t)(0x3C000000LL + i * 0x1000), (uint64_t)(0x3C000000LL + i * 0x1000), 3);
            // clean_page_table((uint64_t)0x3C000000LL + i * 0x1000);
        }
    }
    child->mm.user_stack_pages[0] = child->user_stack;
    child->mm.user_stack_count = 4;
    map_page(child, child->user_stack, 0xffffffffe000, map_stack);
    // map_page(child, child->POSIX.posix_stack, 0xffffffffd000, map_stack);
    // child->POSIX.posix_stack = 0xffffffffd000;

    // change pgd before return to user
    asm volatile("mov x19, %0" ::"r"(child->mm.pgd));
    child->lr = (uint64_t)return_to_user;

    child->function = parent->function;
    // set sp in right place
    // uint64_t user_offset = parent->user_stack - trap_ptr->sp_el0;
    // child->sp_el0 = child->user_stack - user_offset;
    uint64_t kernel_offset = TO_KA(parent->kernel_stack) - TO_KA(ptr);

    child->sp_el1 = child->kernel_stack - kernel_offset;
    child->sp_el0 = parent->sp_el0;
    // uart_printf("child_id = %d, kernel_stack = %x\n",child->thread_id,TO_PA(child->sp_el1));
    // uart_printf("child_id = %d, kernel_stack = %x, offset = %d, parent_kernel_stack = %x, trap_ptr = %x\n",child->thread_id,TO_PA(child->kernel_stack),kernel_offset,parent->kernel_stack,ptr);
    // uint64_t user_offset = trap_ptr->x[29] - trap_ptr->sp_el0;

    // child frame
    trap_frame* child_trapframe = (trap_frame*)child->sp_el1;
    child_trapframe->x[0] = 0;
    // child_trapframe->x[29] = child->fp;
    child_trapframe->sp_el0 = trap_ptr->sp_el0;
    // child->fp = child->sp_el0;
    for (int i = 1; i < 31; ++i) {
        child_trapframe->x[i] = trap_ptr->x[i];
    }
    // child_trapframe->x[29] = child->fp;
    child_trapframe->elr_el1 = trap_ptr->elr_el1;
    child_trapframe->spsr_el1 = trap_ptr->spsr_el1;
    // return value
    trap_ptr->x[0] = child->thread_id;
}

void sys_read(uint64_t ptr)
{
    trap_frame* trap_ptr = (trap_frame*)ptr;
    char* buf = (char*)trap_ptr->x[0];
    int size = (int)trap_ptr->x[1];
    *IRQs1 |= AUX_INT;
    enable_interrupt();
    int i;
    for (i = 0; i < size; ++i)
        buf[i] = uart_getc();
    buf[size] = '\0';
    // disable_interrupt();
    // *IRQ_disable1 |= AUX_INT;
    trap_ptr->x[0] = size;
    disable_interrupt();
    // uart_printf("read buf = %s\n", buf);
}

void sys_write(uint64_t ptr)
{
    trap_frame* trap_ptr = (trap_frame*)ptr;
    char* buf = (char*)trap_ptr->x[0];
    int size = (int)trap_ptr->x[1];
    enable_interrupt();
    for (int i = 0; i < size; ++i) {
        uart_send(buf[i]);
    }
    // disable_interrupt();
    trap_ptr->x[0] = size;
    disable_interrupt();
    // *IRQ_disable1 |= AUX_INT;
}

void from_el12el0()
{
    thread* cur = get_current();
    uint64_t sp = cur->user_stack;
    sp -= sp % 0x100;
    asm volatile(
        "mrs x3, currentEl\n\t"
        "msr sp_el0, %0\n\t"
        "msr spsr_el1, %1\n\t"
        "msr elr_el1, %2\n\t"
        // "mov x30, %3\n\t"
        "eret\n\t"
        :
        : "r"(cur->sp_el0), "r"(0), "r"(cur->function));
}
// 目前無法在各個thread跳到el0，會一直跳svc

void exit_()
{
    // thread* cur = (thread*)TO_PA((uint64_t)get_current());
    thread* cur = (thread*)get_current();
    // uart_printf("the kill id = %d\n", cur->thread_id);
    // cur->status = ZOMBIE;
    if (cur->thread_id) {
        // thread* delete = thread_queue_delete((thread**)&thread_queue, (thread**)&cur);
        // thread_queue_insert((thread**)&zombie_queue, (thread**)&delete);
        cur->status = ZOMBIE;
        /* new add */
        kfree((char*)(cur->user_stack - PAGE_SIZE));
        kfree((char*)(cur->kernel_stack - PAGE_SIZE));
        kfree((char*)(cur->POSIX.posix_stack - PAGE_SIZE));
        kfree((char*)cur);
        /* new add */
        schedule();
    } else {
        uart_printf("kernel cannot leave\n");
    }
    // kill_zombies();
}

void kill_(uint64_t ptr)
{
    trap_frame* trap_ptr = (trap_frame*)ptr;
    thread* cur = (thread*)TO_KA((uint64_t)get_current());

    int kill_id = trap_ptr->x[0];
    if (kill_id == cur->thread_id || kill_id == 0) {
        uart_printf("cannot kill\n");
        return;
    }

    thread* kill_thread = (thread*)TO_KA((uint64_t)thread_queue_find(kill_id));
    if (!kill_thread->thread_id) {
        uart_printf("this thread is null\n");
        return;
    }
    uart_printf("kill id = %d\n", kill_thread->thread_id);
    kill_thread->status = ZOMBIE;
    kill_thread = thread_queue_delete((thread**)&thread_queue, (thread**)&kill_thread);
    kfree((char*)(kill_thread->user_stack - PAGE_SIZE));
    kfree((char*)(kill_thread->kernel_stack - PAGE_SIZE));
    kfree((char*)(kill_thread->POSIX.posix_stack - PAGE_SIZE));
    kfree((char*)kill_thread);
}

void sys_mbox(unsigned char ch, unsigned int* mbox)
{
    // unsigned int *mbox_addr = (unsigned int *)(((struct trapframe *)trapframe)->x[1] - 0xffffffffe000 + cur->m_stack);
    // int val = sys_mailbox(ch, mbox);
    unsigned int* tmp_buf = (unsigned int*)TO_KA((uint64_t)kmalloc(PAGE_SIZE));
    for (int i = 0; i < 144; ++i) {
        ((char*)tmp_buf)[i] = ((char*)mbox)[i];
    }
    int val = sys_mailbox(ch, tmp_buf);

    for (int i = 0; i < 144; ++i) {
        ((char*)mbox)[i] = ((char*)tmp_buf)[i];
    }

    register int x0 asm("x0") __attribute__((unused)) = val;
}

void signal_(uint64_t ptr)
{
    thread* cur = (thread*)get_current();
    trap_frame* trap_ptr = (trap_frame*)ptr;
    int sig_type = (int)trap_ptr->x[0];
    uint64_t signal_handler = (uint64_t)trap_ptr->x[1];
    cur->POSIX.signal_handler[sig_type] = (uint64_t)signal_handler;
    // if(sig_type == SIGKILL || sig_type == SIGDEF){
    //     cur->POSIX.signal_handler[SIGKILL] = (uint64_t)signal_handler;
    // }
    cur->POSIX.signal_type = (unsigned int)sig_type;
    // cur->POSIX.posix_stack = (uint64_t)kmalloc(PAGE_SIZE) + PAGE_SIZE;
    // cur->POSIX.posix_stack += PAGE_SIZE;
}

void sig_handler(uint64_t ptr)
{
    // thread* cur = get_current();
    trap_frame* trap_ptr = (trap_frame*)ptr;
    // unsigned int sig_type = trap_ptr->x[1];

    // cur->signal = true;
    thread* find_thread = thread_queue_find(trap_ptr->x[0]);
    if (!find_thread->thread_id)
        return;
    find_thread->signal = 1;
    // uint64_t signal_handler = find_thread->POSIX.signal_handler[sig_type];
    // cur->POSIX.signal_sp_el1 = ptr;
    // -------  problem  --------- //
    // 1. 沒有用到自己的stack
    // 2. 沒有分清楚到底誰有註冊誰沒註冊，需要改一下判斷
    // if (sig_type == SIGKILL) {
    //     asm volatile(
    //         "mov x10, %0\n\t"
    //         "mov x0, %1\n\t"
    //         "mov x1, %2\n\t"
    //         "mov sp, %3\n\t"
    //         "blr x10\n\t"
    //         "mov sp, %4\n\t" ::"r"(signal_handler),
    //         "r"(ptr), "r"(cur->POSIX.posix_stack), "r"(cur->POSIX.posix_stack), "r"(cur->POSIX.signal_sp_el1));
    //     return_to_user();
    // }
}

void sig_call(uint64_t ptr)
{
    thread* cur = (thread*)TO_PA((uint64_t)get_current());
    trap_frame* trpa_ptr = (trap_frame*)ptr;
    int sig_case = trpa_ptr->x[0];

    switch (sig_case) {
    case SIGRETURN: {
        kfree((char*)(cur->POSIX.posix_stack - PAGE_SIZE));
        // asm volatile(
        // "mrs x3, currentEl\n\t"
        // "msr sp_el0, %0\n\t"
        // "msr spsr_el1, %1\n\t"
        // "msr elr_el1, %2\n\t"
        // // restore sp_el1
        // "msr sp_el1, %3\n\t"
        // // "mov x30, %3\n\t"
        // // "eret\n\t"
        // :
        // // : "r"(cur->sp_el0),  "r"(cur->lr), "r"(cur->sp_el1));
        // : "r"(cur->POSIX.signal_sp_el0), "r"(cur->POSIX.signal_spsr_el1), "r"(cur->POSIX.signal_RA), "r"(cur->POSIX.signal_sp_el1));
        // asm volatile(
        // "mrs x3, currentEl\n\t"
        // "msr sp_el0, %0\n\t"
        // "mov x4, 0\n\t"
        // "msr spsr_el1, x4\n\t"
        // // restore sp_el1
        // "msr elr_el1, %1\n\t"
        // "mov sp, %2\n\t"
        // // "eret"
        // :
        // // : "r"(cur->sp_el0),  "r"(cur->lr), "r"(cur->sp_el1));
        // : "r"(cur->POSIX.signal_sp_el0) , "r"(cur->POSIX.signal_RA), "r"(cur->POSIX.signal_sp_el1));
        // asm volatile("msr sp_el1, %0"::"r"(cur->POSIX.signal_sp_el1));

        // asm volatile("mrs %0, sp_el1":"=r"(ptr):);
        // ptr->elr_el1 = cur->POSIX.signal_RA ;
        // ptr->sp_el0 = cur->POSIX.signal_sp_el0;
        // return_to_user();
        // SIG_return(cur->POSIX.signal_sp_el1, cur->POSIX.signal_Rsp, cur->POSIX.signal_sp_el0);
        cur->POSIX.mask = false;
        asm volatile("mov sp, %0" ::"r"(cur->POSIX.signal_Rsp));
        return_to_user();

        // return_to_user();
        break;
    }
    default:
        break;
    }
}

void signal_kill(uint64_t ptr, uint64_t sp)
{
    trap_frame* trap_ptr = (trap_frame*)ptr;
    thread* cur = (thread*)TO_PA((uint64_t)get_current());
    int kill_id = trap_ptr->x[0];
    if (kill_id == cur->thread_id || kill_id == 0) {
        uart_printf("cannot kill\n");
        return;
    }

    thread* kill_thread = thread_queue_find(kill_id);
    if (!kill_thread->thread_id) {
        uart_printf("this thread is null\n");
        return;
    }
    uart_printf("kill id = %d\n", kill_thread->thread_id);
    kill_thread->status = ZOMBIE;
    kill_thread = thread_queue_delete((thread**)&thread_queue, (thread**)&kill_thread);
    kfree((char*)(kill_thread->user_stack - PAGE_SIZE));
    kfree((char*)(kill_thread->kernel_stack - PAGE_SIZE));
    kfree((char*)(kill_thread->POSIX.posix_stack - PAGE_SIZE));
    kfree((char*)kill_thread);
}

void do_signal()
{
    thread* cur = (thread*)get_current();
    if (cur->signal) {
        // cur->signal = false;

        uint64_t signal_handler = cur->POSIX.signal_handler[SIGKILL];
        if (signal_handler == (uint64_t)exit_) {
            uart_printf("in signal check id = %d\n", cur->thread_id);
            cur->signal = false;
            exit();
        } else {
            // -------  problem  --------- //
            // 1. 沒有用到自己的stack
            // 2. 沒有分清楚到底誰有註冊誰沒註冊，需要改一下判斷
            // asm volatile(
            //     "mov x10, %0\n\t"
            //     "blr x10\n\t"
            //     "mov sp, %1\n\t" ::"r"(signal_handler),
            //     "r"(cur->POSIX.signal_sp_el1));
            // return_to_user();
            enable_interrupt();
            // asm volatile(
            //     "mov x11, sp\n\t"
            //     "mov sp, %0\n\t"
            //     "mov x10, %1\n\t"
            //     "blr x10\n\t"
            //     "mov sp, x11\n\t" ::"r"(cur->POSIX.posix_stack),
            //     "r"(signal_handler));
            // asm volatile("mov %0, sp" : "=r"(cur->POSIX.signal_Rsp) :);
            // cur->POSIX.signal_Rsp = ptr;
            cur->POSIX.mask = true;
            cur->signal = false;
            asm volatile(
                "mov x11, sp\n\t"
                "msr sp_el0, %0\n\t"
                "mov x4, 0\n\t"
                "msr spsr_el1, x4\n\t"
                "msr elr_el1, %1\n\t"
                "mov x30, %2\n\t"
                "eret" ::"r"(cur->POSIX.posix_stack),
                "r"(signal_handler), "r"(signal_return));
            uart_printf("signal return\n");
        }
    }
}

void signal_return()
{
    register unsigned long x0 asm("x0") __attribute__((unused)) = SIGRETURN;
    register unsigned long x8 asm("x8") __attribute__((unused)) = SIG_CALL;

    asm volatile("svc 0" ::);
}

void save_rsp(uint64_t a, uint64_t ptr)
{
    thread* cur = (thread*)TO_PA((uint64_t)get_current());
    if (!cur->POSIX.mask) {
        cur->POSIX.signal_Rsp = ptr;
    }
}

void copy_vm(thread* dst, thread* src)
{
    for (int i = 1; i < 4; ++i) {
        dst->mm.kernel_pages[i] = (uint64_t)kmalloc(PAGE_SIZE);
        dst->mm.user_stack_pages[i] = (uint64_t)kmalloc(PAGE_SIZE);
    }
    char* dst_pgd = (char*)TO_KA(dst->mm.pgd);
    char* src_pgd = (char*)TO_KA(src->mm.pgd);
    for (int i = 0; i < PAGE_SIZE - 20; ++i) {
        *(dst_pgd + i) = *(src_pgd + i);
    }

    char* dst_stack = (char*)TO_KA(dst->mm.user_stack_pages[0]);
    char* src_stack = (char*)TO_KA(src->mm.user_stack_pages[0]);
    for (int i = 0; i < PAGE_SIZE; ++i) {
        *(dst_stack + i) = *(src_stack + i);
    }
    dst->mm.user_stack_count = 4;
    // for (int i = 0; i < 4; ++i) {
    //     // for (int j = 0; i < PAGE_SIZE; ++i) {
    //     //     *(char*)TO_KA((uint64_t)dst->mm.kernel_pages[i] + j) = *(char*)TO_KA((uint64_t)src->mm.kernel_pages[i] + j);
    //     // }
    //     memcpy((void*)TO_KA((uint64_t)dst->mm.kernel_pages[i]), (void*)TO_KA((uint64_t)src->mm.kernel_pages[i]), PAGE_SIZE);
    // }
    // for (int i = 1; i < 4; ++i) {
    //     // for (int j = 0; i < PAGE_SIZE; ++i) {
    //     //     *(char*)TO_KA((uint64_t)(dst->mm.user_stack_pages[i]) + j) = *(char*)TO_KA((uint64_t)(src->mm.user_stack_pages[i]) + j);
    //     // }
    //     memcpy((void*)TO_KA((uint64_t)dst->mm.user_stack_pages[i]), (void*)TO_KA((uint64_t)src->mm.user_stack_pages[i]), PAGE_SIZE);
    // }
}