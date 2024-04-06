#include "exception.h"

void exeception_handle() {
  unsigned long long spsr_el1, elr_el1, esr_el1;
  unsigned int number;
  asm volatile("mrs %0, elr_el1\n\t"
               "mrs %1, spsr_el1\n\t"
               "mrs %2, esr_el1\n\t"
               "mov %3, x0\n\t"
               : "=r"(elr_el1), "=r"(spsr_el1), "=r"(esr_el1), "=r"(number)
               :
               :);
  uart_printf("spsr_el1 = 0x%x\n", spsr_el1);
  uart_printf("elr_el1 = 0x%x\n", elr_el1);
  uart_printf("esr_el1 = 0x%x\n", esr_el1);
  uart_printf("exeception number = %d\n", number);
  // if(number==8 || number==5){
  //     unsigned int count, freq;
  //     asm volatile("mrs  %[result], cntpct_el0": [result]"=r"(count));
  //     asm volatile("mrs  %[result], cntfrq_el0": [result]"=r"(freq));
  //     int time = (count) / freq;
  //     uart_printf("[%d] core timer interrupt\n", time);
  //     set_expired_time(2);
  //     // asm volatile("b ")
  // }else{
  while (1)
    ;
  // }
}

void enable_interrupt() { asm volatile("msr DAIFClr, 0xf\n\t"); }

void disable_interrupt() { asm volatile("msr DAIFSet, 0xf\n\t"); }