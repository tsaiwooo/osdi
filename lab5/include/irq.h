#ifndef __IRQ_H_
#define __IRQ_H_
#include "core_timer.h"
#include "gpio.h"
#include "malloc.h"
#include "uart.h"
/*
https://github.com/Tekki/raspberrypi-documentation/blob/master/hardware/raspberrypi/bcm2836/QA7_rev3.4.pdf
p.7 register address
p.13 core timer interrupts
p.16 bits definition
*/
#define CORE0_INTERRUPT_SOURCE ((volatile unsigned int*)(0x40000060))

/* base address for the ARM interrupt register is 0x7E00B000
https://cs140e.sergio.bz/docs/BCM2837-ARM-Peripherals.pdf p.112
0x200 IRQ basic pending
0x204 IRQ pending 1
0x208 IRQ pending 2
0x20C FIQ control
0x210 Enable IRQs 1
0x214 Enable IRQs 2
0x218 Enable Basic IRQs
0x21C Disable IRQs 1
0x220 Disable IRQs 2
0x224 Disable Basic IRQs
*/

#define IRQ_basic_pending ((volatile unsigned int*)(MMIO_BASE + 0x0000b200))
// interrupts 0~31 from the GPU side
#define IRQ_pending1 ((volatile unsigned int*)(MMIO_BASE + 0x0000b204))
#define IRQ_pending2 ((volatile unsigned int*)(MMIO_BASE + 0x0000b208))
#define IRQs1 ((volatile unsigned int*)(MMIO_BASE + 0x0000b210))
#define IRQs2 ((volatile unsigned int*)(MMIO_BASE + 0x0000b214))
#define IRQ_disable1 ((volatile unsigned int*)(MMIO_BASE + 0x0000b21C))
#define MAX_BUF_SIZE 1024
/*
ARM peripherals interrupts table.
https://cs140e.sergio.bz/docs/BCM2837-ARM-Peripherals.pdf p.113
*/
#define AUX_INT 1 << 29

// timer struct
typedef void (*callback_type)(char*);
typedef struct irq_task_ irq_task;
struct irq_task_ {
    int nice_value;
    int trigger;
    int seconds;
    char message[128];
    // void(callback)(char *);
    void (*callback)(char*);
    // callback_type callback;
    irq_task* next;
};
irq_task *timer_queue, *task_queue;
void add_irq(void (*callback)(char*), char* message, int seconds,
    int nice_value);
void pop_irq();

// nested interrupt
typedef struct queue_task queue_task;
struct queue_task {
    void (*callkback)();
    int nice_value;
    char args[128];
    queue_task* next;
};
queue_task* queue_head;
void add_priority_queue(void (*callback)(char*), char* args, int nc_value);

// for test
extern int a;
#endif // __IRQ_H_