#ifndef __CORE_TIMER_H_
#define __CORE_TIMER_H_
#include "gpio.h"
#include "uart.h"
#define CORE0_TIMER_IRQ_CTRL 0x40000040

// typedef struct {
//   int seconds;

// } timer_list;

// timer_list *list = 0;

extern void core_timer_enable();
void timer_on();
void set_expired_time(unsigned int);
#endif // __CORE_TIMER_H_
