 .section ".text"
 
 // save general registers to stack
.macro save_all
    //sub sp, sp, 32 * 8
    sub sp, sp, 32*9
    stp x0, x1, [sp ,16 * 0]
    stp x2, x3, [sp ,16 * 1]
    stp x4, x5, [sp ,16 * 2]
    stp x6, x7, [sp ,16 * 3]
    stp x8, x9, [sp ,16 * 4]
    stp x10, x11, [sp ,16 * 5]
    stp x12, x13, [sp ,16 * 6]
    stp x14, x15, [sp ,16 * 7]
    stp x16, x17, [sp ,16 * 8]
    stp x18, x19, [sp ,16 * 9]
    stp x20, x21, [sp ,16 * 10]
    stp x22, x23, [sp ,16 * 11]
    stp x24, x25, [sp ,16 * 12]
    stp x26, x27, [sp ,16 * 13]
    stp x28, x29, [sp ,16 * 14]
    mrs x0, spsr_el1
    mrs x1, elr_el1
    mrs x2, sp_el0
    stp x30, x0, [sp, 16 * 15]
    stp x1,x2,[sp, 16 * 16];
    // str x2,[sp, 16 * 17];
    // store spsr_el,elr_el1 to prevent the higher priority interrupt
.endm

// load general registers from stack
.macro load_all
    ldp x30, x0, [sp ,16 * 15]
    ldp x1, x2, [sp, 16 * 16]
    msr spsr_el1, x0
    msr elr_el1, x1
    msr sp_el0, x2
    ldp x0, x1, [sp ,16 * 0]
    ldp x2, x3, [sp ,16 * 1]
    ldp x4, x5, [sp ,16 * 2]
    ldp x6, x7, [sp ,16 * 3]
    ldp x8, x9, [sp ,16 * 4]
    ldp x10, x11, [sp ,16 * 5]
    ldp x12, x13, [sp ,16 * 6]
    ldp x14, x15, [sp ,16 * 7]
    ldp x16, x17, [sp ,16 * 8]
    ldp x18, x19, [sp ,16 * 9]
    ldp x20, x21, [sp ,16 * 10]
    ldp x22, x23, [sp ,16 * 11]
    ldp x24, x25, [sp ,16 * 12]
    ldp x26, x27, [sp ,16 * 13]
    ldp x28, x29, [sp ,16 * 14]
    //ldr x30, [sp, 16 * 15]
    add sp, sp, 32 * 9
    //add sp, sp, 32 * 8
.endm


.macro SIG_load_all
    ldp x30, x0, [sp ,16 * 15]
    //ldp x1, x2, [sp, 16 * 16]
    //msr spsr_el1, x0
    //msr elr_el1, x1
    //msr sp_el0, x2
    ldp x0, x1, [sp ,16 * 0]
    ldp x2, x3, [sp ,16 * 1]
    ldp x4, x5, [sp ,16 * 2]
    ldp x6, x7, [sp ,16 * 3]
    ldp x8, x9, [sp ,16 * 4]
    ldp x10, x11, [sp ,16 * 5]
    ldp x12, x13, [sp ,16 * 6]
    ldp x14, x15, [sp ,16 * 7]
    ldp x16, x17, [sp ,16 * 8]
    ldp x18, x19, [sp ,16 * 9]
    ldp x20, x21, [sp ,16 * 10]
    ldp x22, x23, [sp ,16 * 11]
    ldp x24, x25, [sp ,16 * 12]
    ldp x26, x27, [sp ,16 * 13]
    ldp x28, x29, [sp ,16 * 14]
    //ldr x30, [sp, 16 * 15]
    add sp, sp, 32 * 9
    //add sp, sp, 32 * 8
.endm


.align 11 // vector table should be aligned to 0x800
.global exception_vector_table
exception_vector_table:
  b EL0_Sync // branch to a handler function.
  .align 7 // entry size is 0x80, .align will pad 0
  b EL0_IRQ
  .align 7
  b EL0_FIQ
  .align 7
  b EL0_SError
  .align 7

  b ELx_Sync
  .align 7
  b ELx_IRQ
  .align 7
  b ELx_FIQ
  .align 7
  b ELx_SError
  .align 7

  b EL_AArch64_Sync
  .align 7
  b EL_AArch64_IRQ
  .align 7
  b EL_AArch64_FIQ
  .align 7
  b EL_AArch64_SError
  .align 7

  b EL_AArch32_Sync
  .align 7
  b EL_AArch32_IRQ
  .align 7
  b EL_AArch32_FIQ
  .align 7
  b EL_AArch32_SError
  .align 7

EL0_Sync:
    save_all
    // store exeception number
    mov x0, #0
    bl exeception_handle
    load_all
    eret

EL0_IRQ:
    save_all
    mov x0, #1
    bl exeception_handle
    load_all
    eret

EL0_FIQ:
    save_all
    mov x0, #2
    bl exeception_handle
    load_all
    eret

EL0_SError:
    save_all
    mov x0, #3
    bl exeception_handle
    load_all
    eret

ELx_Sync:
    save_all
    mov x0, #4
    mov x1, sp
    bl save_rsp
    mrs x2, esr_el1
    bl svc_handler
    load_all
    eret

ELx_IRQ:
    save_all
    mov x0, #5
    bl irq_handle
    load_all
    eret

ELx_FIQ:
    save_all
    mov x0, #6
    bl exeception_handle
    load_all
    eret

ELx_SError:
    save_all
    mov x0, #7
    bl exeception_handle
    load_all
    eret

EL_AArch64_Sync:
    save_all
    mov x0, #8
    mov x1, sp
    bl save_rsp
    mrs x2, esr_el1
    bl svc_handler
    load_all
    eret

EL_AArch64_IRQ:
    save_all
    mov x0, #9
    bl irq_handle
    load_all
    eret

EL_AArch64_FIQ:
    save_all
    mov x0, #10
    bl exeception_handle
    load_all
    eret

EL_AArch64_SError:
    save_all
    mov x0, #11
    bl exeception_handle
    load_all
    eret

EL_AArch32_Sync:
    save_all
    mov x0, #12
    bl exeception_handle
    load_all
    eret

EL_AArch32_IRQ:
    save_all
    mov x0, #13
    bl exeception_handle
    load_all
    eret

EL_AArch32_FIQ:
    save_all
    mov x0, #14
    bl exeception_handle
    load_all
    eret

EL_AArch32_SError:
    save_all
    mov x0, #15
    bl exeception_handle
    load_all
    eret

.global core_timer_enable
core_timer_enable:
    mov     x0, #1
    msr     cntp_ctl_el0, x0 // enable 

    //mrs     x0, cntfrq_el0
    //add     x0,x0,x0 // 2s
    //msr     cntp_tval_el0, x0 // set expired time
    //bl      set_expired_time

    mov     x0, #2 // CNTPNSIRQ IRQ control
    ldr     x1, =0x40000040
    str     w0, [x1] // unmask timer interrupt, w0 = x0 lower 32bits

    //mov     x3, #0
    //msr     spsr_el1, x3
    //msr     elr_el1, lr
    ret

.global core_timer_disable
core_timer_disable:
    mov     x0, #0
    ldr     x1, =0x40000040
    str     w0, [x1]
    ret

// lab5
.global switch_to
switch_to:
    stp x19, x20, [x0, 16 * 0]
    stp x21, x22, [x0, 16 * 1]
    stp x23, x24, [x0, 16 * 2]
    stp x25, x26, [x0, 16 * 3]
    stp x27, x28, [x0, 16 * 4]
    stp fp, lr, [x0, 16 * 5]
    mrs x9, sp_el0
    mov x10, sp
    stp x9, x10, [x0, 16 * 6]

    ldp x19, x20, [x1, 16 * 0]
    ldp x21, x22, [x1, 16 * 1]
    ldp x23, x24, [x1, 16 * 2]
    ldp x25, x26, [x1, 16 * 3]
    ldp x27, x28, [x1, 16 * 4]
    ldp fp, lr, [x1, 16 * 5]
    ldp x9, x10, [x1, 16 * 6]

    mov sp, x10
    msr sp_el0,  x9
    msr tpidr_el1, x1
    ret

.global get_current
get_current:
    mrs x0, tpidr_el1
    ret

.global return_to_user
return_to_user:
    load_all
    eret

.global SIG_return
SIG_return:
    msr sp_el1, x0
    msr elr_el1, x1
    //add sp, sp, 32 * 9
    eret

.global handler
handler:
    mov sp,x2
    bl kill_
    mov sp,x0