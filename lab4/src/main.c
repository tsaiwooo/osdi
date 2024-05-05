#include "main.h"
// define if using bootloader, dtb address will change
// #define LOCAL
#define bootloader
#define kmalloc_demo
#define dma_demo
void main(char* dtb)
{
// bootloader 把 dtb place先存在r15
// 如果用這行，傳過去會卡住
// void *dtb_adr = 0x8000000;
// asm("mov %0, x1" : "=r"(dtb_place)); // this line also ok!!
#ifdef LOCAL
    dtb_place = dtb;
#else
#ifdef bootloader
    asm("mov %0, x10" : "=r"(dtb_place)); // this line also ok!!
#endif
#endif
    // set up serial console
    uart_init();
    // enable_interrupt();

    fdt_traverse(dtb_callback_initramfs);

    // say hello
    uart_puts("Hello World!\n");

    char* string = simple_malloc(sizeof("test1 malloc\n"));
    memcpy(string, "test1 malloc\n", sizeof("test1 malloc\n"));
    uart_printf("string = %s", string);

    char* string1 = simple_malloc(sizeof("hello world\n"));
    memcpy(string1, "hello world\n", sizeof("hello world\n"));
    uart_printf("string1 = %s", string1);

    char* string2 = simple_malloc(sizeof("hehehehe\n"));
    memcpy(string2, "hehehehe\n", sizeof("hehehehe\n"));
    uart_printf("string2 = %s", string2);

    uart_printf("dtb : 0x%x\r\n", dtb_place);
    // core_timer_enable();
    // int el=0;
    // asm volatile("mrs %0, CurrentEL\n\t":"=r"(el):);
    // uart_printf("EL = %d\n",el);
    // timer_on();
    // asm volatile(
    //     "mov x3, 0x0\n\t"
    //     "msr spsr_el1, x3\n\t"
    //     "msr elr_el1, %[data]\n\t"
    //     "msr sp_el0, %[stack_ptr]\n\t"
    //     "eret"
    //     :
    //     : [data] "r"(shell), [stack_ptr] "r"(0x10000));
    // range in 0~20000
    init_buddy();
    mem_zone_reserve();
    uart_printf(
        "=======================mem reserve done========================\n");
#ifdef kmalloc_demo
    int size = 7 * KB;
    char* addr = kmalloc(64 * KB);
    uart_printf("64KB kmalloc start addr = %x\n", addr - &_buddy_start);
    char* addr2 = kmalloc(size);
    uart_printf("7KB kmalloc start addr = %x\n", addr2 - &_buddy_start);
    char* addr3 = kmalloc(32 * KB);
    uart_printf("32KB kmalloc start addr = %x\n", addr3 - &_buddy_start);
    char* addr4 = kmalloc(8 * KB);
    uart_printf("8KB kmalloc start addr = %x\n", addr4 - &_buddy_start);
    // uart_printf(
    //     "=========================== now kfree start "
    //     "===================================\n");
    uart_printf("kfree 8KB\n");
    kfree(addr4);
    uart_printf("kfree 32KB\n");
    kfree(addr3);
    uart_printf("kfree 7KB\n");
    kfree(addr2);
    addr2 = (char*)kmalloc(size);
    uart_printf("7KB kmalloc start addr = %x\n", addr2 - &_buddy_start);
    uart_printf("kfree 64KB\n");
    kfree(addr);
    uart_printf("kfree 7KB\n");
    kfree(addr2);
    uart_printf(
        "==============================================kfree "
        "done=====================================\n");
#endif
#ifdef dma_demo
    init_DMA();
    char* DMA_malloc = kmalloc(2000);
    uart_printf("15 address = %x\n", DMA_malloc - &_buddy_start);
    char* DMA_malloc1 = kmalloc(513);
    uart_printf("513 address = %x\n", DMA_malloc1 - &_buddy_start);
    char* DMA_malloc2 = kmalloc(514);
    uart_printf("514 address = %x\n", DMA_malloc2 - &_buddy_start);
    kfree(DMA_malloc2);
    char* DMA_malloc3 = kmalloc(522);
    uart_printf("522 address = %x\n", DMA_malloc3 - &_buddy_start);
    uart_printf(
        "===================================================DMA "
        "done===============================\n");
#endif
    // char* a0 = kmalloc(100);
    // uart_printf("%x\n", a0);
    // char* a1 = kmalloc(100);
    // uart_printf("%x\n", a1);
    // char* a2 = kmalloc(100);
    // uart_printf("%x\n", a2);
    // char* a3 = kmalloc(100);
    // uart_printf("%x\n", a3);
    // char* a4 = kmalloc(100);
    // uart_printf("%x\n", a4);
    // char* a5 = kmalloc(100);
    // uart_printf("%x\n", a5);
    // char* a6 = kmalloc(100);
    // uart_printf("%x\n", a6);
    shell();
}