#include "main.h"

void main()
{
    // set up serial console
    uart_init();
    get_board_revision();
    // say hello
    uart_puts("Hello World!\n");
    
    shell();
    
}