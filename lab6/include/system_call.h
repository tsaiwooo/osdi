#ifndef __SYSTEM_CALL_H_
#define __SYSTEM_CALL_H_
#include "buddy_system.h"
#include "exception.h"
#include "mm.h"
#include "thread.h"
#include "uart.h"
#define uint64_t unsigned long long
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
#endif // __STSTEM_CALL_H_