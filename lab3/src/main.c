#include "main.h"
// define if using bootloader, dtb address will change
#define bootloader

void main(char *dtb) {
// bootloader 把 dtb place先存在r15
// 如果用這行，傳過去會卡住
// void *dtb_adr = 0x8000000;
// asm("mov %0, x1" : "=r"(dtb_place)); // this line also ok!!
#ifdef LOCAL
  dtb_place = dtb;
#else
#ifdef bootloader
  asm("mov %0, x10" : "=r"(dtb_place));  // this line also ok!!
#endif
#endif
  // set up serial console
  uart_init();
  enable_interrupt();

  fdt_traverse(dtb_callback_initramfs);

  // say hello
  uart_puts("Hello World!\n");

  char *string = simple_malloc(sizeof("test1 malloc\n"));
  memcpy(string, "test1 malloc\n", sizeof("test1 malloc\n"));
  uart_printf("string = %s", string);

  char *string1 = simple_malloc(sizeof("hello world\n"));
  memcpy(string1, "hello world\n", sizeof("hello world\n"));
  uart_printf("string1 = %s", string1);

  char *string2 = simple_malloc(sizeof("hehehehe\n"));
  memcpy(string2, "hehehehe\n", sizeof("hehehehe\n"));
  uart_printf("string2 = %s", string2);

  uart_printf("dtb : 0x%x\r\n", dtb_place);
  // core_timer_enable();
  // int el=0;
  // asm volatile("mrs %0, CurrentEL\n\t":"=r"(el):);
  // uart_printf("EL = %d\n",el);
  // timer_on();
  // asm volatile(
  //     "mov x3, 0x0\n\t"
  //     "msr spsr_el1, x3\n\t"
  //     "msr elr_el1, %[data]\n\t"
  //     "msr sp_el0, %[stack_ptr]\n\t"
  //     "eret"
  //     :
  //     : [data] "r"(shell), [stack_ptr] "r"(0x10000));
  shell();
}