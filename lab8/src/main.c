#include "main.h"
// define if using bootloader, dtb address will change
#define LOCAL
// #define bootloader
// #define kmalloc_demo
// #define dma_demo
#define THREAD
#define SYSTEM_CALL
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
    uart_printf("kmalloc start addr = %x\n", addr - &_buddy_start);
    char* addr2 = kmalloc(size);
    uart_printf("kmalloc start addr = %x\n", addr2 - &_buddy_start);
    char* addr3 = kmalloc(32 * KB);
    uart_printf("kmalloc start addr = %x\n", addr3 - &_buddy_start);
    char* addr4 = kmalloc(8 * KB);
    uart_printf("kmalloc start addr = %x\n", addr4 - &_buddy_start);
    uart_printf(
        "=========================== now kfree start "
        "===================================\n");
    uart_printf("kfree 8KB\n");
    kfree(addr4);
    uart_printf("kfree 32KB\n");
    kfree(addr3);
    uart_printf("kfree 7KB\n");
    kfree(addr2);
    addr2 = (char*)kmalloc(size);
    uart_printf("kmalloc start addr = %x\n", addr2 - &_buddy_start);
    uart_printf("kfree 64KB\n");
    kfree(addr);
    uart_printf("kfree 7KB\n");
    kfree(addr2);
    uart_printf(
        "==============================================kfree "
        "done=====================================\n");
#endif

#ifdef dma_demo
    // extern unsigned char* top;
    init_DMA();
    int dma_size = 32;

    char* DMA_malloc = kmalloc(2000);
    uart_printf("15 address = %x\n", DMA_malloc - &_buddy_start);
    char* DMA_malloc1 = kmalloc(513);
    uart_printf("513 address = %x\n", DMA_malloc1 - &_buddy_start);
    char* DMA_malloc2 = kmalloc(514);
    uart_printf("514 address = %x\n", DMA_malloc2 - &_buddy_start);
    // kfree(DMA_malloc2);
    char* DMA_malloc3 = kmalloc(522);
    char* DMA_malloc4 = kmalloc(dma_size);
    char* DMA_malloc5 = kmalloc(dma_size);
    char* DMA_malloc6 = kmalloc(dma_size);
    uart_printf("522 address = %x\n", DMA_malloc3 - &_buddy_start);
    uart_printf(
        "===================================================DMA "
        "done===============================\n");

    // uart_printf("top = %x\n", top);
#endif

#ifdef THREAD

    init_thread();
    // for (int i = 0; i < MAX_THREADS; ++i) {
    //     thread_create(&test_function);
    // }

    // idle();

    // enable_interrupt();
    // int pid = fork();
    // uart_printf("pid = %d\n", pid);
    // degub_info();
#endif
    rootfs_init();
    initramfs();
    init_device();

    // lab8
    sd_init();
    fat32_init();
#ifdef SYSTEM_CALL
    uint64_t tmp;
    asm volatile("mrs %0, cntkctl_el1" : "=r"(tmp));
    tmp |= 1;
    asm volatile("msr cntkctl_el1, %0" : : "r"(tmp));
    enable_interrupt();
    core_timer_enable();
    scheduler_timer();
    // unsigned int __attribute__((aligned(16))) mbox[8];
    unsigned int* mbox;
    // uart_printf("address = 0x%x\n", mbox);
    mbox = (unsigned int*)kmalloc(sizeof(unsigned int) * 7);
    mbox[0] = 8 * 4; // buffer size in bytes
    mbox[1] = REQUEST_CODE;

    // tags begin
    mbox[2] = MBOX_BOARD_REVISION; // tag identifier

    mbox[3] = 8; // maximum of request and response value buffer's length.
    mbox[4] = TAG_REQUEST_CODE;
    mbox[5] = 0; // value buffer

    mbox[6] = END_TAG; // value buffer
    // tags end
    // mbox[7] = END_TAG;
    mbox_call(MBOX_CH_PROP, mbox);
    uart_printf("0x%x\n", mbox[5]);
    // char* a0 = kmalloc(4096);
    // size_t size = uart_read(buf, (int)10);
    // uart_printf("read return size = %d\n", size);
    // size = uart_write("test_uart_write\n", sizeof("test_uart_write\n"));
    // uart_printf("write return size = %d\n", size);
    asm volatile(
        "mov x3, 0x0\n\t"
        "msr spsr_el1, x3\n\t"
        "msr elr_el1, %[data]\n\t"
        "msr sp_el0, %[stack_ptr]\n\t"
        "eret"
        :
        : [data] "r"(shell), [stack_ptr] "r"(0x60000));
    // int res = 0;
    // res = fork();
    // if (!res) {
    //     uart_printf("id = %d\n", res);
    // } else {
    //     uart_printf("id = %d\n", res);
    //     // exit();
    // }
#endif
    shell();
}