#include "main.h"
// define if using bootloader, dtb address will change
#define LOCAL
// #define bootloader
#define kmalloc_demo
// #define dma_demo
#define THREAD
#define SYSTEM_CALL
int first = 1;
void main(char* dtb)
{
    if (first) {
        first = 0;
// bootloader 把 dtb place先存在r15
// 如果用這行，傳過去會卡住
// void *dtb_adr = 0x8000000;
// asm("mov %0, x1" : "=r"(dtb_place)); // this line also ok!!
#ifdef LOCAL
        // dtb_place = ((char**)&dtb + VIRTUAL_MEM_BASE);
        asm("mov %0, x9" : "=r"(dtb_place)); // this line also ok!!
        // dtb_place = (char**)&dtb;
        // dtb_place = (char*)(VIRTUAL_MEM_BASE + dtb);
#else
#ifdef bootloader
        asm("mov %0, x10" : "=r"(dtb_place)); // this line also ok!!
        dtb_place += VIRTUAL_MEM_BASE;
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
#ifdef kmalloc_demo
        init_buddy();
        mem_zone_reserve();
        uart_printf(
            "=======================mem reserve done========================\n");
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

#ifdef SYSTEM_CALL
        uint64_t tmp;
        asm volatile("mrs %0, cntkctl_el1" : "=r"(tmp));
        tmp |= 1;
        asm volatile("msr cntkctl_el1, %0" : : "r"(tmp));
        enable_interrupt();
        core_timer_enable();
        scheduler_timer();
        /* debug for fork */
        // unsigned int* mbox;
        // // uart_printf("address = 0x%x\n", mbox);
        // mbox = (unsigned int*)kmalloc(sizeof(unsigned int) * 7);
        // mbox[0] = 8 * 4; // buffer size in bytes
        // mbox[1] = REQUEST_CODE;

        // // tags begin
        // mbox[2] = MBOX_BOARD_REVISION; // tag identifier

        // mbox[3] = 8; // maximum of request and response value buffer's length.
        // mbox[4] = TAG_REQUEST_CODE;
        // mbox[5] = 0; // value buffer

        // mbox[6] = END_TAG; // value buffer
        // // tags end
        // // mbox[7] = END_TAG;
        // mbox_call(MBOX_CH_PROP, mbox);
        // uart_printf("0x%x\n", mbox[5]);
        // fork_test();
        // while(1);
        /* debug for fork */

        exec_("vm.img");
        thread* cur = get_current();
        uart_printf("cur pgd = %x\n", cur->mm.pgd);
        for (int i = 0; i < ((0x90000 - 0x80000) / 4096); ++i) {
            // uart_printf("i = %d\n", i);
            // char* gpu_mem = kmalloc(PAGE_SIZE);
            // map_page(child, (uint64_t)gpu_mem, 0x3C000000 + i * 0x1000, 3);
            map_page(cur, (uint64_t)0x80000 + i * 4096, 0x80000 + i * 4096, 3);
        }
        // set_pgd(cur->mm.pgd);
        // uint64_t start_addr = TO_PA((uint64_t)shell) & CODE_MASK;
        // uint64_t start_addr = TO_PA((uint64_t)shell) - (uint64_t)0x80000;
        // uint64_t x29, x30;
        // asm volatile("mov %0, x29" : "=r"(x29));
        // asm volatile("mov %0, x30" : "=r"(x30));
        // x29 = TO_PA(x29);
        // x30 = TO_PA(x30);
        // asm volatile("mov x29, %0" ::"r"(x29));
        // asm volatile("mov x30, %0" ::"r"(x30));
        asm volatile(
            "mov     x5, #1\n\t"
            "msr     cntp_ctl_el0, x5\n\t" // enable
            "mov     x5, #2\n\t" // CNTPNSIRQ IRQ control
            "ldr     x6, =0xffff000040000040\n\t"
            "str     w5, [x6]\n\t" // unmask timer interrupt, w0 = x0 lower 32bits"
            "mov x3, 0x0\n\t"
            "msr spsr_el1, x3\n\t"
            "msr elr_el1, %[data]\n\t"
            "msr sp_el0, %[stack_ptr]\n\t"
            "dsb     ish\n\t" // ensure write has completed
            "msr     ttbr0_el1, %[pgd]\n\t" // switch translation based address.
            "tlbi    vmalle1is\n\t" // invalidate all TLB entries
            "dsb     ish\n\t" // ensure completion of TLB invalidatation
            "eret"
            :
            // : [data] "r"(start_addr), [stack_ptr] "r"(0xfffffffff000));
            : [data] "r"(TO_PA((uint64_t)shell)), [stack_ptr] "r"(0xfffffffff000), [pgd] "r"(cur->mm.pgd));
    }
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