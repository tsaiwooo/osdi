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
    mov     x10, x15 // x15 store dtb address ,x15 stores in bootloader
    // mrs     x15, CurrentEL
    // cpu id == 0, switch from EL2 to EL1
    bl      from_el2_to_el1
    // mrs     x15, CurrentEL
    //mov     x15, x0
 set_exception_vector_table:
    adr     x1, exception_vector_table
    msr     vbar_el1, x1    

    // set top of stack just before our code (stack grows to a lower address per AAPCS64)
    // ldr     x1, =_start
    ldr     x1, =_start
    mov     sp, x1

    // clear bss
    ldr     x1, =__bss_start
    ldr     w2, =__bss_size
3:  cbz     w2, 4f
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 3b

    // jump to C code, should not return
4:  
    // open the interrupt
    //msr     DAIFClr, 0xf // open all interrupt then timer interrupt can happen
    //adr     x3, shell
    //msr     elr_el1, x3
    //eret 
//shell:
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

/*
from_el1_el0:
    ldr x1, =_start
    msr sp_el0, x1
    msr spsr_el1, x1
    msr elr_el1, lr
    eret
*/

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