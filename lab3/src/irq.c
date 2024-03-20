#include "irq.h"

void irq_handle() {
  if (*IRQ_pending1 & AUX_INT) {
    // uart_printf("in uart handler\n");
    async_handler();
  } else if (*CORE0_INTERRUPT_SOURCE & 0x2) {
    unsigned int count, freq;
    asm volatile("mrs  %[result], cntpct_el0" : [result] "=r"(count));
    asm volatile("mrs  %[result], cntfrq_el0" : [result] "=r"(freq));
    int time = (count) / freq;
    uart_printf("[%d] core timer interrupt\n", time);
    set_expired_time(2);
  }
}