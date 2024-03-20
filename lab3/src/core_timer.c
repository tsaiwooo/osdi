#include "core_timer.h"
// https://github.com/Tekki/raspberrypi-documentation/blob/master/hardware/raspberrypi/bcm2836/QA7_rev3.4.pdf

void timer_on() {
  // int tmp;

  // __asm__ __volatile__(
  // "mov x1, 1\n\t"
  // "msr cntp_ctl_el0, x1\n\t" // enable

  // "mrs x1, cntfrq_el0\n\t"
  // "mov x2, 1\n\t"
  // "mul x1, x1, x2\n\t"        //set a big value prevent interrupt immediately
  // "msr cntp_tval_el0, x1\n\t" // set expired time

  // "mov x2, 2\n\t"
  // "ldr x1, =0x40000040 \n\t"
  // "str w2, [x1]\n\t" // unmask timer interrupt
  // "mov x3, 0\n\t"
  // "msr spsr_el1, x3\n\t"
  // "msr elr_el1, lr\n\t"
  // "ret"
  // :::"x1","x2");
  // asm volatile("mov %0, #1\n\t"
  //              "msr cntp_ctl_el0, %0\n\t"
  //              "mrs %0, cntfrq_el0\n\t"
  //              "msr cntp_tval_el0, %0\n\t"
  //              "mov %0, #2\n\t"
  //              "ldr %1, %2\n\t"
  //              "str %0, [%1]\n\t"
  //              : "+r"(tmp), "+r"(tmp_adr)
  //              : "+r"(CORE0_TIMER_IRQ_CTRL)
  //              : "memory");
  uart_printf("in core timer enable\n");
  core_timer_enable();
}

void set_expired_time(unsigned int time) {
  unsigned long cntfrq_el0;
  asm volatile("mrs %0, cntfrq_el0" : "=r"(cntfrq_el0));
  // asm volatile("msr cntp_tval_el0, %0" : : "r"(cntfrq_el0 * time));
  asm volatile("msr cntp_tval_el0, %0" : : "r"(cntfrq_el0 * time));
}

// void add_timer((void *)function,char *message)
// {

// }
