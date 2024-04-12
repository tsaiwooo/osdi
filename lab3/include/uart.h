#ifndef _UART_H_
#define _UART_H_
#include "exception.h"
#include "gpio.h"
#include "irq.h"
#include "my_string.h"
/* Auxilary mini UART registers */
#define AUX_ENABLE ((volatile unsigned int *)(MMIO_BASE + 0x00215004))
#define AUX_MU_IO ((volatile unsigned int *)(MMIO_BASE + 0x00215040))
#define AUX_MU_IER ((volatile unsigned int *)(MMIO_BASE + 0x00215044))
#define AUX_MU_IIR ((volatile unsigned int *)(MMIO_BASE + 0x00215048))
#define AUX_MU_LCR ((volatile unsigned int *)(MMIO_BASE + 0x0021504C))
#define AUX_MU_MCR ((volatile unsigned int *)(MMIO_BASE + 0x00215050))
#define AUX_MU_LSR ((volatile unsigned int *)(MMIO_BASE + 0x00215054))
#define AUX_MU_MSR ((volatile unsigned int *)(MMIO_BASE + 0x00215058))
#define AUX_MU_SCRATCH ((volatile unsigned int *)(MMIO_BASE + 0x0021505C))
#define AUX_MU_CNTL ((volatile unsigned int *)(MMIO_BASE + 0x00215060))
#define AUX_MU_STAT ((volatile unsigned int *)(MMIO_BASE + 0x00215064))
#define AUX_MU_BAUD ((volatile unsigned int *)(MMIO_BASE + 0x00215068))

/**
 * Set baud rate and characteristics (115200 8N1) and map to GPIO
 */
// typedef struct {
//   char ch;
//   async_queue *next;
// } async_queue;

void uart_init();
void uart_send(unsigned int);
char uart_getc();
void uart_puts(char *);
void uart_printf(char *, ...);
void uart_flush();
void async_handler();
void enable_uart_r_interrupt();
void disable_uart_r_interrupt();
void enable_uart_w_interrupt();
void disable_uart_w_interrupt();
void async_send(char);
char async_getc();
void w_handler(char *);
void r_handler(char *);
void printf_r();
#endif  // _UART_H_