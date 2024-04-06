#ifndef __EXCEPTION_H_
#define __EXCEPTION_H_
#include "core_timer.h"
#include "uart.h"

void exeception_handle();
void enable_interrupt();
void disable_interrupt();
void irq_handle();
#endif // __EXCEPTION_H_