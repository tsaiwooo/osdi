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
            // disable_interrupt();
            // trap_frame* trap_ptr = (trap_frame*)ptr;
            sys_mbox((unsigned char)trap_ptr->x[0], (unsigned int*)trap_ptr->x[1]);
            // enable_interrupt();
            break;
        }
        case KILL:
            kill_(ptr);
            break;
        case SIGREG:
            signal_(ptr);
            break;
        case SIG_KILL:
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
    struct cpio_newc_header* header = CPIO_DEFAULT_PLACE;
    char* filename;
    unsigned long filesize = 0;
    // uart_printf("%d filename = %s\n", trap_ptr->x[8], (char*)trap_ptr->x[0]);
    while (header != 0) {
        int finish = cpio_parse_header(header, (const char**)&filename, &filesize, &data, &header);
        if (finish)
            break;
        if (!strcmp(function, filename)) {
            char* user_addr = (char*)kmalloc(sizeof(char) * filesize);
            // char* user_addr = (char*)0x80000;
            // char* user_sp = (char*)0x60000;
            // char* user_sp = (char*)kmalloc(PAGE_SIZE);
            // user_sp += PAGE_SIZE;
            for (unsigned long i = 0; i < filesize; i++) {
                user_addr[i] = *data++;
            }
            // thread* new = thread_create((void*)user_addr);
            // switch_to(get_current(), new);
            // uart_printf("after set\n");
            asm volatile(
                "mov x3, #0\n\t"
                "msr spsr_el1, x3\n\t"
                "msr elr_el1, %[data]\n\t"
                // "msr sp_el0, %[stack_ptr]\n\t"
                "eret"
                :
                // : [data] "r"(user_addr), [stack_ptr] "r"(user_sp));
                : [data] "r"(user_addr));
        }
    }

    asm volatile("mov x0, %0" : : "r"(-1));
}

void fork_exec(uint64_t ptr)
{
    trap_frame* trap_ptr = (trap_frame*)ptr;
    thread* parent = get_current();
    // uint64_t function_addr;
    // asm volatile("mov %0, lr" : "=r"(function_addr) :);
    thread* child = thread_create((void*)0);
    // uart_printf("exit2\n");
    // POSIX
    for (int i = 0; i <= SIG_NUMS; ++i) {
        child->POSIX.signal_handler[i] = parent->POSIX.signal_handler[i];
    }
    // stack
    for (int i = 1; i <= PAGE_SIZE; ++i) {
        *(char*)(child->user_stack - i) = *(char*)(parent->user_stack - i);
    }
    // uart_printf("exit3\n");
    for (int i = 1; i <= PAGE_SIZE; ++i) {
        *(char*)(child->kernel_stack - i) = *(char*)(parent->kernel_stack - i);
    }
    // uart_printf("exit4\n");
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
    // child->lr = (uint64_t)test_function;
    // uint64_t user_fp_offset = parent->user_stack - parent->fp;
    // child->fp = child->user_stack - user_fp_offset;
    // uint64_t kernel_fp_offset = parent->kernel_stack - trap_ptr->x[29];

    // child->fp = parent->fp;
    child->lr = (uint64_t)return_to_user;

    child->function = parent->function;
    // set sp in right place
    // user_offset < 0
    uint64_t user_offset = parent->user_stack - trap_ptr->sp_el0;
    child->sp_el0 = child->user_stack - user_offset;
    uint64_t kernel_offset = parent->kernel_stack - ptr;
    child->sp_el1 = child->kernel_stack - kernel_offset;
    // uint64_t user_offset = trap_ptr->x[29] - trap_ptr->sp_el0;

    // child frame
    trap_frame* child_trapframe = (trap_frame*)child->sp_el1;
    child_trapframe->x[0] = 0;
    // child_trapframe->x[29] = child->fp;
    child_trapframe->sp_el0 = child->sp_el0;
    // child_trapframe->elr_el1 = trap_ptr->elr_el1;
    // child_trapframe->spsp_el1 = trap_ptr->spsp_el1;
    child->fp = child->sp_el0;
    child_trapframe->x[29] = child->fp;

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
    enable_uart_r_interrupt();

    int i;
    for (i = 0; i < size; ++i) {
        // uart_printf("i = %d\n", i);
        buf[i] = async_getc();
    }
    buf[i] = '\0';
    trap_ptr->x[0] = size;
    disable_uart_r_interrupt();
    disable_interrupt();
    *IRQ_disable1 |= AUX_INT;
    // uart_printf("read buf = %s\n", buf);
}

void sys_write(uint64_t ptr)
{
    trap_frame* trap_ptr = (trap_frame*)ptr;
    char* buf = (char*)trap_ptr->x[0];
    int size = (int)trap_ptr->x[1];
    int i;
    *IRQs1 |= AUX_INT;
    for (i = 0; i < size; ++i) {
        async_send(buf[i]);
    }
    trap_ptr->x[0] = i;
    disable_uart_r_interrupt();
    disable_interrupt();
    *IRQ_disable1 |= AUX_INT;
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
    thread* cur = get_current();
    cur->status = ZOMBIE;
    if (cur->thread_id) {
        // thread* delete = thread_queue_delete((thread**)&thread_queue, (thread**)&cur);
        // thread_queue_insert((thread**)&zombie_queue, (thread**)&delete);
        cur->status = ZOMBIE;
    }
    schedule();
    // kill_zombies();
}

void kill_(uint64_t ptr)
{
    trap_frame* trap_ptr = (trap_frame*)ptr;
    thread* cur = get_current();

    int kill_id = trap_ptr->x[0];
    if (kill_id == cur->thread_id || kill_id == 0) {
        uart_printf("cannout kill\n");
        return;
    }

    thread* kill_thread = thread_queue_find(kill_id);
    if(!kill_thread->thread_id){
        uart_printf("this thread is null\n");
        return;
    }
    uart_printf("kill id = %d\n", kill_thread->thread_id);
    kill_thread->status = ZOMBIE;
    kill_thread = thread_queue_delete((thread**)&thread_queue, (thread**)&kill_thread);
    kfree((char*)(kill_thread->user_stack - PAGE_SIZE));
    kfree((char*)(kill_thread->kernel_stack - PAGE_SIZE));
    kfree((char*)kill_thread);

    // asm volatile("ret");
    // if (kill_thread->thread_id) {
    //     kill_thread->status = ZOMBIE;
    //     uart_printf("status = %d\n", kill_thread->status);
    // }
    // if (kill_thread) {
    //     thread* delete = thread_queue_delete((thread**)&thread_queue, (thread**)&kill_thread);
    //     thread_queue_insert((thread**)&zombie_queue, (thread**)&delete);
    // }
    // kill_zombies();
}

void sys_mbox(unsigned char ch, unsigned int* mbox)
{
    // trap_frame* trap_ptr = (trap_frame*)ptr;
    // unsigned char ch = (unsigned char)(trap_ptr->x[0]);
    // unsigned int* mbox = (unsigned int*)(trap_ptr->x[1]);

    int val = sys_mailbox(ch, mbox);
    register int x0 asm("x0") __attribute__((unused)) = val;
}

void signal_(uint64_t ptr)
{
    thread* cur = get_current();
    trap_frame* trap_ptr = (trap_frame*)ptr;
    int sig_type = (int)trap_ptr->x[0];
    uint64_t signal_handler = (uint64_t )trap_ptr->x[1];
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
    thread *cur = get_current();
    trap_frame *trap_ptr = (trap_frame *)ptr;
    unsigned int sig_type = trap_ptr->x[1];

    cur->signal = true;
    uint64_t signal_handler = cur->POSIX.signal_handler[sig_type];
    // store the return address before enter signal handler, include original sp_el0
    // sp_el1 which before this exeception happend
    // cur->POSIX.signal_spsr_el1 = trap_ptr->spsr_el1;
    // cur->POSIX.signal_sp_el0 = trap_ptr->sp_el0;
    cur->POSIX.signal_sp_el1 = ptr;
    // cur->POSIX.signal_RA = trap_ptr->elr_el1;
    // goto signal handler, and give kill_id store at x0
    // new method
    // asm volatile("msr sp_el1, %0"::"r"(cur->POSIX.posix_stack));
    // register unsigned long sp asm("sp") __attribute__((unused)) = cur->POSIX.posix_stack;
    // signal_kill(ptr,cur->POSIX.posix_stack);
    // -------  problem  --------- //
    // 1. 沒有用到自己的stack
    // 2. 沒有分清楚到底誰有註冊誰沒註冊，需要改一下判斷
    if(sig_type == SIGKILL ){
        // void (*handler)(uint64_t,uint64_t) = (void (*)(uint64_t,uint64_t))signal_handler;
        // pass trap_frame address and POSIX stack address
        // asm volatile("mov x1, %0"::"r"(cur->POSIX.signal_sp_el1));
        // asm volatile("mov x0, %0"::"r"(ptr));
        // (*handler)(ptr,cur->POSIX.posix_stack);
        
        asm volatile(
            "mov x10, %0\n\t"
            "mov x0, %1\n\t"
            "mov x1, %2\n\t"
            "mov sp, %3\n\t"
            "blr x10\n\t"
            "mov sp, %4\n\t"
            ::"r"(signal_handler), "r"(ptr), "r"(cur->POSIX.posix_stack), "r"(cur->POSIX.posix_stack),"r"(cur->POSIX.signal_sp_el1)
        );
        return_to_user();
    }
}

void sig_call(uint64_t ptr)
{
    thread *cur = get_current();
    trap_frame *trpa_ptr = (trap_frame *)ptr;
    int sig_case = trpa_ptr->x[0];

    switch (sig_case)
    {
    case SIGRETURN:{
        kfree((char *)(cur->POSIX.posix_stack - PAGE_SIZE));
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
        SIG_return(cur->POSIX.signal_sp_el1,cur->POSIX.signal_RA,cur->POSIX.signal_sp_el0);
        break;
    }
    default:
        break;
    }
}

void signal_kill(uint64_t ptr,uint64_t sp)
{
    trap_frame* trap_ptr = (trap_frame*)ptr;
    thread* cur = get_current();
    int kill_id = trap_ptr->x[0];
    if (kill_id == cur->thread_id || kill_id == 0) {
        uart_printf("cannout kill\n");
        return;
    }

    
    thread* kill_thread = thread_queue_find(kill_id);
    if(!kill_thread->thread_id){
        uart_printf("this thread is null\n");
        return;
    }
    uart_printf("kill id = %d\n", kill_thread->thread_id);
    kill_thread->status = ZOMBIE;
    kill_thread = thread_queue_delete((thread**)&thread_queue, (thread**)&kill_thread);
    kfree((char*)(kill_thread->user_stack - PAGE_SIZE));
    kfree((char*)(kill_thread->kernel_stack - PAGE_SIZE));
    kfree((char*)kill_thread);
}