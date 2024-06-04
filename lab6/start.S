#include "mmu.h"
.section ".text.boot"


.global _start
_start:
    // read cpu id, stop slave cores
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, 2f
    // cpu id > 0, stop
1:  wfe
    b       1b
2:  

    mov     x9, x0
    mov     x10, x15 // x15 store dtb address ,x15 stores in bootloader
    // mrs     x15, CurrentEL
    // cpu id == 0, switch from EL2 to EL1
    bl      from_el2_to_el1
    // mrs     x15, CurrentEL
    //mov     x15, x0
 set_exception_vector_table:
    adr     x1, exception_vector_table
    msr     vbar_el1, x1    
    
    bl      set_page_table
    // bug here
    mrs x2, sctlr_el1
    orr x2 , x2, 1
    msr sctlr_el1, x2 // bug
    isb
    // set top of stack just before our code (stack grows to a lower address per AAPCS64)
    ldr     x1, =_start
    mov     sp, x1



    // clear bss
    ldr     x1, =__bss_start
    ldr     w2, =__bss_size
3: 
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 3b

    // jump to C code, should not return

4:   
    //ldr x1, =virtual_mem
    //add sp,sp,x1
    ldr x2, =boot_rest
    //adr x2, boot_rest
    br x2

boot_rest:
    bl      main
    // for failsafe, halt this core too
    b       1b

from_el2_to_el1:
    mov x1, (0b11 << 20) // this fixes printf (printf use stdargs, which access floating register v0~v7, and those are SIMD registers)
    msr cpacr_el1, x1
    mov x1, (1 << 31) // EL1 uses aarch64
    msr hcr_el2, x1
    mov x1, 0x3c5 // EL1h (SPSel = 1) with interrupt disabled
    msr spsr_el2, x1
    msr elr_el2, lr
    eret // return to EL1

from_el1_el0:
    ldr     x1, =_start
    msr     sp_el0, x1
    mov     x2, 0
    //msr     spsr_el1, x2
    //adr     x2, _jump_main
    //msr     DAIFClr, 0xf
    msr     elr_el1, lr
    eret
    //ret

core_timer_handler:
    mrs x0, cntfrq_el0 // read frequency 
    msr cntp_tval_el0, x0


set_page_table:
    mov x29, x30

    // TCR setting
    ldr x4, = TCR_CONFIG_DEFAULT
    msr tcr_el1, x4
    // MAIR setting
    ldr x4, =( \
            (MAIR_DEVICE_nGnRnE << (MAIR_IDX_DEVICE_nGnRnE * 8)) | \
            (MAIR_NORMAL_NOCACHE << (MAIR_IDX_NORMAL_NOCACHE * 8)) \
            )
    msr mair_el1, x4

    ldr x0, =pg_dir // PGD start
    and x0, x0, 0x0000ffffffffffff // first 16bits set 0, because 64byte aligned in page table
    //lsl     x0, x0, #16  // omit first 16 bit (using physical address)
    //lsr     x0, x0, #16
    add x1, x0, #PAGE_SIZE // PUD start
    add x2, x1, #PAGE_SIZE // PMD start
    add x3, x2, #PAGE_SIZE // PTE start
    
    // PGD settup
    ldr x4, =BOOT_PGD0_ATTR
    orr  x4, x1, x4
    str x4, [x0]

    // PUD0
    ldr x4, =BOOT_PUD0_ATTR
    orr x4, x2, x4
    //mov x5, 0x00000000
    //orr x4, x5, x4
    str x4, [x1]

    // PUD1
    ldr x4, =BOOT_PUD1_ATTR
    mov x5, 0x40000000
    orr x4, x5, x4
    str x4, [x1, #8]

    // PMD
    mov x4, x3 // PTE base address
    mov x5, xzr // i
    mov x6, #512 // PMD has 512 entries

PMD_SETUP:
    ldr x7, =BOOT_PMD0_ATTR
    orr x7, x4, x7
    str x7, [x2, x5,lsl #3]
    add x5, x5, #1
    add x4, x4, #PAGE_SIZE // move to next page address
    cmp x5, x6
    b.ls PMD_SETUP

    mov x4, xzr // physical address -> 0~0x3C000000
    mov x5, xzr // i
    mov x6, #258048 // 512*512 - 4096(0x40000000~0x3C000000)

PTE_SETUP:
    ldr x7, =BOOT_PTE_NORMAL
    orr x7, x4, x7
    str x7, [x3, x5, lsl #3]
    add x5, x5, #1
    add x4, x4, #PAGE_SIZE
    cmp x5, x6
    b.ls PTE_SETUP

    // 0x40000000~0x3C000000 device PTE
    add x6, x6, #4096
PTE_SETUP_DEVICE:
    //ldr x7, =BOOT_PTE_DEVICE
    ldr x7, =BOOT_PTE_NORMAL
    orr x7, x4, x7
    str x7, [x3, x5, lsl #3]
    add x5, x5, #1
    add x4, x4, #PAGE_SIZE
    cmp x5,x6
    b.ls PTE_SETUP_DEVICE

    msr ttbr0_el1, x0 // load PGD to the bottom translation-    based register.
    msr ttbr1_el1, x0 // also load PGD to the upper translation based register.
    mov x30, x29
    ret

    

    // TCR setting
    /*ldr x0, = TCR_CONFIG_DEFAULT
    msr tcr_el1, x0
    // MAIR setting
    ldr x0, =( \
            (MAIR_DEVICE_nGnRnE << (MAIR_IDX_DEVICE_nGnRnE * 8)) | \
            (MAIR_NORMAL_NOCACHE << (MAIR_IDX_NORMAL_NOCACHE * 8)) \
            )
    msr mair_el1, x0
    // Page table setting
    mov x0, 0 // PGD's page frame at 0x0
    mov x1, 0x1000 // PUD's page frame at 0x1000

    ldr x2, = BOOT_PGD0_ATTR
    orr x2, x1, x2 // combine the physical address of next level page with attribute.
    str x2, [x0]

    ldr x2, = BOOT_PUD_ATTR
    mov x3, 0x00000000
    orr x3, x2, x3
    str x3, [x1] // 1st 1GB mapped by the 1st entry of PUD
    mov x3, 0x40000000
    ldr x2, = BOOT_PUD_ATTR
    orr x3, x2, x3
    str x3, [x1, 8] // 2nd 1GB mapped by the 2nd entry of PUD

    msr ttbr0_el1, x0 // load PGD to the bottom translation-    based register.
    msr ttbr1_el1, x0 // also load PGD to the upper translation based register.
    mrs x2, sctlr_el1
    orr x2 , x2, 1
    msr sctlr_el1, x2
    ret*/

.globl set_pgd
set_pgd:
    dsb     ish             // ensure write has completed
    msr     ttbr0_el1, x0   // switch translation based address.
    tlbi    vmalle1is       // invalidate all TLB entries
    dsb     ish             // ensure completion of TLB invalidatation
    isb                     // clear pipeline
    ret

.globl get_pgd
get_pgd:
	mov x1, 0
	ldr x0, [x1]
	mov x0, 0x1000
	msr	ttbr0_el1, x0
	ldr x0, [x1]
	ret