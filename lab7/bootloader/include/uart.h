#ifndef _UART_H_
#define _UART_H_
void uart_init();
void uart_send(unsigned int );
char uart_getc();
void uart_puts(char *);
void uart_printf(char* , ...);
void uart_flush();
char uart_getc_pure();
#endif // _UART_H_