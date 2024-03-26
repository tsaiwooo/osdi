#ifndef __CORE_TIMER_H_
#define __CORE_TIMER_H_
#include "gpio.h"
#include "uart.h"
#define CORE0_TIMER_IRQ_CTRL 0x40000040

extern void core_timer_enable();
extern void core_timer_disable();
void timer_on();
void set_expired_time(unsigned int);
unsigned long current_time();
void print_time_mes(char *);
#endif  // __CORE_TIMER_H_
