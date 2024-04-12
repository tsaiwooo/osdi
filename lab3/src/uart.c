#include "uart.h"
#define demo_advance2

void uart_init() {
  register unsigned int r;

  /* initialize UART */
  *AUX_ENABLE |= 1;  // enable UART1, AUX mini uart
  *AUX_MU_CNTL = 0;
  *AUX_MU_LCR = 3;  // 8 bits
  *AUX_MU_MCR = 0;
  *AUX_MU_IER = 0;
  *AUX_MU_IIR = 0x6;   // disable interrupts
  *AUX_MU_BAUD = 270;  // 115200 baud
  /* map UART1 to GPIO pins */
  r = *GPFSEL1;
  r &= ~((7 << 12) | (7 << 15));  // gpio14, gpio15
  r |= (2 << 12) | (2 << 15);     // alt5
  *GPFSEL1 = r;
  // enable pins 14 and 15
  *GPPUD = 0;
  r = 150;
  while (r--) {
    asm volatile("nop");
  }
  *GPPUDCLK0 = (1 << 14) | (1 << 15);
  r = 150;
  while (r--) {
    asm volatile("nop");
  }
  *GPPUDCLK0 = 0;    // flush GPIO setup
  *AUX_MU_CNTL = 3;  // enable Tx, Rx
}

/**
 * Send a character
 */
void uart_send(unsigned int c) {
  /* wait until we can send */
  do {
    asm volatile("nop");
  } while (!(*AUX_MU_LSR & 0x20));
  /* write the character to the buffer */
  *AUX_MU_IO = c;
}

/**
 * Receive a character
 */
char uart_getc() {
  char r;
  /* wait until something is in the buffer */
  do {
    asm volatile("nop");
  } while (!(*AUX_MU_LSR & 0x01));
  /* read it and return */
  r = (char)(*AUX_MU_IO);
  /* convert carriage return to newline */
  return r == '\r' ? '\n' : r;
}

/**
 * Display a string
 */
void uart_puts(char *s) {
  while (*s) {
    /* convert newline to carriage return + newline */
    if (*s == '\n') uart_send('\r');
    uart_send(*s++);
  }
}

void uart_printf(char *fmt, ...) {
  __builtin_va_list args;
  __builtin_va_start(args, fmt);

  extern volatile unsigned char _end;  // defined in linker
  char *s = (char *)&_end;             // put temporary string after code
  vsprintf(s, fmt, args);

  uart_puts(s);
}

void uart_flush() {
  while (*AUX_MU_LSR & 0x01) {
    *AUX_MU_IO;
  }
}

/*
AUX_MU_IIR 2:1 bits
00 : No interrupts
01 : Transmit holding register empty
10 : Receiver holds valid byte
11 : <Not possible>

AUX_MU_IER
1 : enable transmit interrupts
0 : enable receive interrupts
*/

#ifdef demo_async
char tx_buf[1024] = {"buffer aaaaaaaaaaaa\r\n"};
unsigned int tx_write_idx = 21;
// unsigned int tx_write_idx = 8;
#else
char tx_buf[1024] = {};
unsigned int tx_write_idx = 0;
#endif
unsigned int tx_read_idx = 0;
char rx_buf[1024] = {};
unsigned int rx_write_idx = 0;
unsigned int rx_read_idx = 0;

void async_handler() {
  // write,send.
  *IRQ_disable1 |= AUX_INT;
  if (*AUX_MU_IIR & (0b01 << 1)) {
    // disable_uart_w_interrupt();

    // if (tx_write_idx == tx_read_idx) {
    //   *IRQs1 |= AUX_INT;
    //   enable_uart_w_interrupt();
    //   return;
    // }

    // *AUX_MU_IO = tx_buf[tx_read_idx++];
    // if (tx_read_idx >= MAX_BUF_SIZE) tx_read_idx = 0;  // cycle pointer
    // w_handler();
    add_priority_queue(&w_handler, "w_handler", 2);
  }
  // read
  else if (*AUX_MU_IIR & (0b10 << 1)) {
    // disable_uart_r_interrupt();
    // // if ((rx_write_idx + 1) == rx_read_idx) {
    // //   *IRQs1 |= AUX_INT;
    // //   enable_uart_r_interrupt();
    // //   return;
    // // }
    // char ch = (char)(*AUX_MU_IO);
    // rx_buf[rx_write_idx++] = (ch == '\r') ? '\n' : ch;
    // if (rx_write_idx >= MAX_BUF_SIZE) rx_write_idx = 0;
    // enable_uart_r_interrupt();
    // r_handler();
    add_priority_queue(&r_handler, "r_handler", 2);
  }
#ifdef demo_advance2_nonpreempt
  *IRQ_disable1 |= AUX_INT;
#endif
  *IRQs1 |= AUX_INT;
}
int a = 0;
void w_handler(char *args) {
  disable_uart_w_interrupt();
  // disable_interrupt();
  // uart_printf("in w handler\n");
#ifdef demo_advance2
  while (a == 0) {
    ;
  }
#endif
  if (tx_write_idx == tx_read_idx) {
    *IRQs1 |= AUX_INT;
    enable_uart_w_interrupt();
    return;
  }
  while (tx_read_idx < tx_write_idx) {
    int times = 100;
    *AUX_MU_IO = tx_buf[tx_read_idx++];
    while (times) {
      asm volatile("nop");
      times--;
    }
  }
  if (tx_read_idx >= MAX_BUF_SIZE) tx_read_idx = 0;  // cycle pointer
  enable_interrupt();
}

void r_handler(char *args) {
  disable_uart_r_interrupt();
  // uart_printf("r handler\n");
  char ch = (char)(*AUX_MU_IO);
  rx_buf[rx_write_idx++] = (ch == '\r') ? '\n' : ch;
  // if (rx_write_idx >= MAX_BUF_SIZE) rx_write_idx = 0;
  enable_uart_r_interrupt();
}
// send is ok
void async_send(char ch) {
  disable_interrupt();
  tx_buf[tx_write_idx] = ch;
  tx_write_idx++;
  if (tx_write_idx >= MAX_BUF_SIZE) tx_write_idx = 0;
  enable_interrupt();
  // enable_uart_w_interrupt();
#ifdef demo_advance2
  enable_uart_w_interrupt();
#endif
}

char async_getc() {
  // disable_interrupt();
  while (rx_read_idx == rx_write_idx) {
    asm volatile("nop");
  }
  char ch = rx_buf[rx_read_idx];
  rx_read_idx++;
  if (rx_read_idx >= MAX_BUF_SIZE) rx_read_idx = 0;
  enable_interrupt();
  return ch;
}

void enable_uart_r_interrupt() { *AUX_MU_IER |= (1); }

void disable_uart_r_interrupt() { *AUX_MU_IER &= ~(1); }

void enable_uart_w_interrupt() { *AUX_MU_IER |= (2); }

void disable_uart_w_interrupt() { *AUX_MU_IER &= ~(2); }

void printf_r() {
  for (int i = 0; i < tx_write_idx; ++i) {
    uart_send(tx_buf[i]);
  }
  tx_write_idx = 0;
}