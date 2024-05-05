#ifndef __EXCEPTION_H_
#define __EXCEPTION_H_
#include "core_timer.h"
#include "cpio.h"
#include "mailbox.h"
#include "shell.h"
#include "system_call.h"
#include "thread.h"
#include "uart.h"
#include "posix.h"

#define uint64_t unsigned long long
enum svc_type {
    GETPID,
    READ,
    WRITE,
    EXEC,
    FORK,
    EXIT,
    MBOX,
    KILL,
    SIGREG,
    SIG_KILL,
    SIG_CALL
};

enum signal_type{
    SIGKILL = 9,
    SIGDEF,
    SIGRETURN
};

extern void SIG_return(uint64_t sp,uint64_t elr_el1,uint64_t sp_el0);
extern void handler(uint64_t,uint64_t,uint64_t);
void from_el12el0();
void exeception_handle();
void enable_interrupt();
void disable_interrupt();
void irq_handle();
void svc_handler();

void exec_function();
void fork_exec();
void sys_read(uint64_t);
// void sys_read(trap_frame* );
// void sys_write(trap_frame* );
void sys_write(uint64_t);
void fork_exec(uint64_t);
void sys_mbox(unsigned char, unsigned int*);
void exit_();
void kill_(uint64_t);
void signal_(uint64_t);
void sig_handler(uint64_t);
void sig_call(uint64_t);
void signal_kill(uint64_t ptr,uint64_t sp);
#endif // __EXCEPTION_H_