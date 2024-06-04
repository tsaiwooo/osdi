#ifndef _SHELL_H_
#define _SHELL_H_
#define BUF_SIZE 1024
#include "core_timer.h"
#include "cpio.h"
#include "irq.h"
#include "mailbox.h"
#include "malloc.h"
#include "my_string.h"
#include "reboot.h"
#include "uart.h"

enum ANSI_ESC { Unknown, CursorForward, CursorBackward, Delete };
enum ANSI_ESC parse_CSI();
void shell();
void shell_decode(char *);
void do_(char *);
#endif  // _SHELL_H_