#include "exception.h"

void exeception_handle() {
  unsigned long long spsr_el1 = 0, elr_el1 = 0, esr_el1 = 0;
  unsigned int number;
  // asm volatile(
  //     "mov x3, #0x3c0\n\t"
  //     "msr spsr_el1, x3\n\t");
  asm volatile(
      "mrs %0, ELR_EL1\n\t"
      "mrs %1, SPSR_EL1\n\t"
      "mrs %2, ESR_EL1\n\t"
      "mov %3, x0\n\t"
      : "=r"(elr_el1), "=r"(spsr_el1), "=r"(esr_el1), "=r"(number)
      :
      :);
  uart_printf("spsr_el1 = 0x%x\n", spsr_el1);
  uart_printf("elr_el1 = 0x%x\n", elr_el1);
  uart_printf("esr_el1 = 0x%x\n", esr_el1);
  uart_printf("exeception number = %d\n", number);
  // asm volatile(
  //     "mov x10, #0\n\t"
  //     "msr spsr_el1, x10");
  // while (1)
  //   ;
}

void enable_interrupt() { asm volatile("msr DAIFClr, 0xf\n\t"); }

void disable_interrupt() { asm volatile("msr DAIFSet, 0xf\n\t"); }