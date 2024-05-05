#include "system_call.h"

int getpid()
{
    int pid;
    // asm volatile("mov x8, %0" ::"r"(GETPID));
    register unsigned long x8 asm("x8") = GETPID;
    // uart_printf("after call svc\n");
    asm volatile("svc 0" ::"r"(x8));
    asm volatile("mov %0, x0" : "=r"(pid) :);
    // register int pid asm("x0");
    return pid;
}

size_t uart_read(char* buf, size_t size)
{
    // asm volatile("mov x8, %0" ::"r"(READ));
    // asm volatile("mov x0, %0\n\t"
    //              "mov x1, %1\n\t" ::"r"((void*)buf),
    //     "r"(size));
    register unsigned long x8 asm("x8") = READ;
    asm volatile("svc 0" ::"r"(x8));
    size_t return_size;
    asm volatile("mov %0, x0" : "=r"(return_size) :);
    return return_size;
}

size_t uart_write(char* buf, size_t size)
{
    // asm volatile("mov x8, %0" ::"r"(WRITE));
    register unsigned long x8 asm("x8") = WRITE;
    // asm volatile("mov x0, %0\n\t"
    //              "mov x1, %1\n\t" ::"r"((void*)buf),
    //     "r"(size));
    asm volatile("svc 0" ::"r"(x8));
    size_t return_size;
    asm volatile("mov %0, x0" : "=r"(return_size) :);
    return return_size;
}

int exec(const char* name, char** const argv)
{
    // asm volatile("mov x8, %0" ::"r"(EXEC));
    // asm volatile("mov x0, %0\n\t"
    //              "mov x1, %1\n\t" :
    //              : "r"(name), "r"(argv));
    // register const char* x0 asm("x0") = name;
    // register char** const x1 asm("r1") = argv;
    int return_val;
    // uart_printf("name = %s\n", name);
    register unsigned long x8 asm("x8") __attribute__((unused)) = EXEC;
    // asm volatile("svc 0" ::"r"(x0), "r"(x1), "r"(x8));
    asm volatile("svc 0");
    asm volatile("mov %0, x0" : "=r"(return_val) :);
    return return_val;
}

int fork()
{
    int pid;
    // asm volatile("mov x8, %0" ::"r"(FORK));
    register unsigned long x8 asm("x8") = FORK;
    asm volatile("svc 0" ::"r"(x8));
    asm volatile("mov %0, x0" : "=r"(pid) :);
    // uart_printf("pid = %d\n", pid);
    // if(pid != getpid()) return 0;
    return pid;
}

void exit()
{
    asm volatile("mov x8, %0" ::"r"(EXIT));
    asm volatile("svc 0");
}

void kill__(int pid,uint64_t cur)
{
    uart_printf("in kill function\n");
    // asm volatile("mov x8, %0" ::"r"(KILL));
    register unsigned long x8 asm("x8") __attribute__((unused)) = KILL;
    register unsigned long x0 asm("x0") __attribute__((unused)) = pid;

    // asm volatile("mov x0, %0" ::"r"(pid));
    // thread* cur = get_current();
    asm volatile("svc 0");
    uart_printf("exit1\n");
    if(((thread *)cur)->signal){
        uart_printf("if judge\n");
        call_signal(SIGRETURN);
        // ((thread *)cur)->signal = false;
    }
}

int mbox_call(unsigned char ch, unsigned int* mbox)
{
    register unsigned char x0 asm("x0") __attribute__((unused)) = ch;
    register unsigned int* x1 asm("r1") __attribute__((unused)) = mbox;
    register unsigned long x8 asm("x8") __attribute__((unused)) = MBOX;
    asm volatile("svc 0");
    register int val asm("x0");
    return val;
}

void test_handler()
{
    uart_printf("tset kill function>_<\n");
    delay(10000000);
    uart_printf("hehehehehe\n");
    call_signal(SIGRETURN);
}
int signal(int SIGNAL,uint64_t signal_handler)
{
    register unsigned long x8 asm("x8") __attribute__((unused)) = SIGREG;
    asm volatile("svc 0");
    return 1;

}

void kill(int pid,int SIGNAL)
{
    register unsigned long x8 asm("x8") __attribute__((unused)) = SIG_KILL;
    asm volatile("svc 0");
}

void call_signal(int SIGNAL)
{
    register unsigned long x8 asm("x8") __attribute__((unused)) = SIG_CALL;
    asm volatile("svc 0");
}