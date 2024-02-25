#include "main.h"

void main(char *dtb)
{
    dtb_place = dtb;
    // set up serial console    
    uart_init();

    fdt_traverse(dtb_callback_initramfs);

    sysinfo();

    // say hello
    uart_puts("Hello World!\n");


    char *string = simple_malloc(sizeof("test1 malloc\n"));
    memcpy(string,"test1 malloc\n",sizeof("test1 malloc\n"));
    uart_printf("string = %s",string);

    char *string1 = simple_malloc(sizeof("hello world\n"));
    memcpy(string1,"hello world\n",sizeof("hello world\n"));
    uart_printf("string1 = %s",string1);

    char *string2 = simple_malloc(sizeof("hehehehe\n"));
    memcpy(string2,"hehehehe\n",sizeof("hehehehe\n"));
    uart_printf("string2 = %s",string2);

    shell();
    
}