#include "main.h"
extern char **_dtb;
extern char *__relocoate_place;
extern unsigned long long __code_size;
extern char *__code_start;

int relo = 1;

#pragma GCC push_options
#pragma GCC optimize ("O0")
void main(char *args)
{
    // x0會指向dtb, 所以x0可以讀取到DTB address
    // asm("mov %0, x0": "=r"(_dtb));
    *_dtb = (char *)0x8000000;
    // _dtb = args;
    // relocate bootloader to 0x60000
    // point to 0x60000
    char* relo_place = (char*)&__relocoate_place;
    if(relo){
        relo = 0;
        relocate(relo_place);
    }
    
    uart_init();
    uart_puts("Hello bootloader!\n");
    // load_kernel();
    shell();
}
#pragma GCC pop_options

void relocate(char *addr)
{
    unsigned long long size = (unsigned long long)&__code_size;
    char* start = (char *)&__code_start;
    for(unsigned long long i=0;i<size;i++)
    {
        addr[i] = start[i];
    }

    ((void (*)(char**))addr)(_dtb);  //jump to new place 
}