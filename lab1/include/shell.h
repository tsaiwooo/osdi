#ifndef _SHELL_H_
#define _SHELL_H_
#define BUF_SIZE 1024
#include "uart.h"
#include "my_string.h"
#include "reboot.h"
enum ANSI_ESC {
    Unknown,
    CursorForward,
    CursorBackward,
    Delete
};
enum ANSI_ESC parse_CSI();
void shell();
void shell_decode(char *);
void do_(char *);
#endif // _SHELL_H_