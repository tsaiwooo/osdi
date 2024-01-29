#include "main.h"

void main()
{
    // set up serial console
    uart_init();
    sysinfo();
    // say hello
    uart_puts("Hello World!\n");
    
    shell();
    
}