#include "shell.h"

void shell() {
  do {
    char cmd[BUF_SIZE];
    shell_decode(cmd);
    do_(cmd);
  } while (1);
}

void shell_decode(char *cmd) {
  uart_printf("\r# ");
  int end = 0, idx = 0, i;
  cmd[0] = '\0';
  char c;
  while ((c = uart_getc()) != '\n') {
    // decode CSI
    if (c == 27) {
      enum ANSI_ESC key = parse_CSI();
      switch (key) {
        case CursorForward:
          if (idx < end) idx++;
          break;
        case CursorBackward:
          if (idx > 0) idx--;
          break;
        case Delete:
          for (i = idx; i < end; i++) {
            cmd[i] = cmd[i + 1];
          }
          cmd[--end] = '\0';
          break;
        case Unknown:
          uart_flush();
          break;
        default:
          break;
      }
    } else if (c == 3) {  // CTRL-C
      cmd[0] = '\0';
      break;
    } else if (c == 8 || c == 127) {  // Backapce
      if (idx > 0) {
        idx--;
        for (i = idx; i < end; i++) {
          cmd[i] = cmd[i + 1];
        }
        cmd[--end] = '\0';
      }
    } else {
      if (idx < end) {
        for (i = end; i > idx; i--) {
          cmd[i] = cmd[i - 1];
        }
      }
      cmd[idx++] = c;
      cmd[++end] = '\0';
    }
    uart_printf("\r# %s \r\e[%dC", cmd, idx + 2);
    // uart_printf("\r# %s \r\e[%dC", cmd, idx + 2);
  }
  uart_printf("\n");
}

enum ANSI_ESC parse_CSI() {
  char c = uart_getc();
  if (c == '[') {
    c = uart_getc();
    if (c == 'C') {
      return CursorForward;
    } else if (c == 'D') {
      return CursorBackward;
    } else if (c == '3') {
      c = uart_getc();
      if (c == '~') {
        return Delete;
      }
    }
  }
  return Unknown;
}

void do_(char *cmd) {
  if (!strcmp(cmd, "")) {
    return;
  } else if (!strcmp(cmd, "help")) {
    uart_printf("help: print all available commands\n");
    uart_printf("hello: print Hello World!\n");
    uart_printf("reboot: reboot the device\n");
    uart_printf("load_kernel: load kernel with bootloader\n");
  } else if (!strcmp(cmd, "hello")) {
    uart_printf("Hello World!\n");
  } else if (!strcmp(cmd, "reboot")) {
    uart_printf("Rebooting...\n");
    reset(500);
    // while (1); // hang until reboot
  } else if (!strcmp(cmd, "load_kernel")) {
    uart_printf("Start loading......\n");
    load_kernel();
  } else {
    uart_printf("shell: command not found: %s\n", cmd);
  }
}

// #pragma GCC push_options
// #pragma GCC optimize ("O0")
void load_kernel() {
  unsigned long long kernel_size = 0;
  char *kernel_start = (char *)&__code_start;
  char ch;
  // uart_printf("welcome uart bootloader\n\rkernel size : \n\r");
  for (int i = 0; i < 4; i++) {
    ch = uart_getc_pure();
    kernel_size += ch << (i * 8);
  }

  // if(kernel_size<8192 || kernel_size > 65536){
  //     uart_puts("data error\n");
  //     return load_kernel();
  // }
  for (int i = 0; i < kernel_size; i++) {
    ch = uart_getc_pure();
    kernel_start[i] = ch;
  }
  // uart_printf("finished\n");
  // void (*kernel) (char *) = (void (*) (char *))kernel_start;
  // kernel(_dtb);
  ((void (*)(char **))kernel_start)(_dtb);
}
// #pragma GCC pop_options