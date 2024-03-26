#include "core_timer.h"
// https://github.com/Tekki/raspberrypi-documentation/blob/master/hardware/raspberrypi/bcm2836/QA7_rev3.4.pdf

void timer_on() {
  // uart_printf("in core timer enable\n");
  set_expired_time(2);
  core_timer_enable();
}

void set_expired_time(unsigned int time) {
  unsigned long cntfrq_el0;
  asm volatile("mrs %0, cntfrq_el0" : "=r"(cntfrq_el0));
  // asm volatile("msr cntp_tval_el0, %0" : : "r"(cntfrq_el0 * time));
  asm volatile("msr cntp_tval_el0, %0" : : "r"(cntfrq_el0 * time));
}

unsigned long current_time() {
  unsigned long time, freq;
  asm volatile(
      "mrs %0, cntpct_el0\n\t"
      "mrs %1, cntfrq_el0\n\t"
      : "=r"(time), "=r"(freq)::);
  return time / freq;
}

void print_time_mes(char *message) {
  unsigned int count, freq;
  asm volatile("mrs  %[result], cntpct_el0" : [result] "=r"(count));
  asm volatile("mrs  %[result], cntfrq_el0" : [result] "=r"(freq));
  int time = (count) / freq;
  uart_printf("[%d] %s\n", time, message);
}