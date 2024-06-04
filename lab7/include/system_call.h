#ifndef __SYSTEM_CALL_H_
#define __SYSTEM_CALL_H_
#include "buddy_system.h"
#include "exception.h"
#include "thread.h"
#include "uart.h"
#define uint64_t unsigned long long
enum vfs {
    SYS_OPEN = 11,
    SYS_CLOSE,
    SYS_WRITE,
    SYS_READ,
    SYS_MKDIR,
    SYS_MOUNT,
    SYS_CHDIR
};
int getpid();
int fork();
int exec(const char* name, char** const argv);
size_t uart_read(char*, size_t);
size_t uart_write(char*, size_t);
void kill__(int pid, uint64_t cur);
void exit();
int mbox_call(unsigned char ch, unsigned int* mbox);
int signal(int SIGNAL, uint64_t signal_handler);
void kill(int pid, int SIGNAL);
void call_signal(int SIGNAL);

void test_handler();
int open(const char* pathname, int flags);
int close(int fd);
long write(int fd, const void* buf, unsigned long count);
long read(int fd, void* buf, unsigned long count);
int mkdir(const char* pathname, unsigned mode);
int mount(const char* src, const char* target, const char* filesystem, unsigned long flags, const void* data);
int chdir(const char* path);
#endif // __STSTEM_CALL_H_