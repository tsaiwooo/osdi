#include "irq.h"
int cur_nv = 3;

void irq_handle()
{
    if (*IRQ_pending1 & AUX_INT) {
        // uart_printf("in uart handler\n");
        async_handler();
    } else if (*CORE0_INTERRUPT_SOURCE & 0x2) {
        // core_timer_disable();
        // uart_printf("\nin core timer irq\n");
        // pop_irq();
        // disable_interrupt();
        // kill_zombies();
        unsigned long cntfrq_el0;
        asm volatile("mrs %0, cntfrq_el0" : "=r"(cntfrq_el0));
        asm volatile("msr cntp_tval_el0, %0" : : "r"(cntfrq_el0 >> 5));
        schedule();
        // enable_interrupt();
    }
}

void add_irq(void (*callback)(char*), char* message, int seconds,
    int nice_value)
{
    irq_task* tmp = (irq_task*)simple_malloc((unsigned long)sizeof(irq_task));
    // handle nice_value==0 -> represents timer
    // if (nice_value < pre_nv) {
    //   enable_interrupt();
    // }
    // if (nice_value == 0) {
    tmp->seconds = seconds;
    tmp->trigger = current_time() + seconds;
    tmp->callback = callback;
    tmp->nice_value = nice_value;
    tmp->next = 0;
    int i;
    for (i = 0; message[i] != '\0'; ++i) {
        tmp->message[i] = message[i];
    }
    tmp->message[i] = '\0';
    uart_printf("add_irq,tmp->message = %s,seconds = %d,trigger =%d\n",
        tmp->message, tmp->seconds, tmp->trigger);
    irq_task* cur = timer_queue;
    irq_task* pre;

    // head
    if (!cur) {
        // uart_printf("null\n");
        timer_queue = tmp;
        // if (nice_value == 0) {
        set_expired_time(tmp->seconds);
        core_timer_enable();
        // }
        enable_interrupt();
        // trigger時間比較早到,則改變head value
    } else if (tmp->trigger <= cur->trigger || nice_value < cur->nice_value) {
        tmp->next = cur;
        timer_queue = tmp;
        unsigned int spare_time = tmp->trigger - current_time();
        // if (nice_value == 0) {
        set_expired_time(spare_time);
        core_timer_enable();
        // }
        enable_interrupt();
    } else {
        while (cur) {
            if (tmp->trigger <= cur->trigger) {
                pre->next = tmp;
                tmp->next = cur;
                pre = cur;
                cur = cur->next;
                return;
            }
            pre = cur;
            cur = cur->next;
        }
        pre->next = tmp;
    }
}

void pop_irq()
{
    irq_task* cur = timer_queue;
    // uart_printf("in irq\n");

    if (!cur) {
        return;
    } else {
        // (*(cur->callback))(cur->message);
        // uart_printf("before enter priority : %s\n",cur->message);
        add_priority_queue(cur->callback, cur->message, cur->nice_value);
        a = 1;
        // uart_printf("after priority : %s\n",cur->message);
        // cur->callback(cur->message);
        // head移到下一個
        // irq_task *pre = cur;
        unsigned int before_time, after_time;
        before_time = cur->trigger;
        cur = cur->next;
        if (cur != 0) {
            // uart_printf("not null\n");
            after_time = cur->trigger;
            // if (cur->nice_value == 0) {
            set_expired_time(after_time - before_time);
            core_timer_enable();
            // }
            enable_interrupt();
        }
        timer_queue = cur;
    }
}

void add_priority_queue(void (*callback)(char*), char* args, int nc_value)
{
    // priority high -> nice value low
    if (nc_value < cur_nv) {
        // uart_printf("\n111111111111\n");
        // 要執行則把cur_nv換成現在執行的nc_value
        cur_nv = nc_value;
        // uart_printf("now nice = %d\n",cur_nv);
        enable_interrupt();
        callback(args);
    }
    // sort the queue according to nice_value
    else {
        // uart_printf("\n22222222222222\n");
        queue_task* tmp = (queue_task*)simple_malloc(sizeof(queue_task));
        tmp->nice_value = nc_value;
        tmp->callkback = callback;
        tmp->next = 0;
        int i;
        for (i = 0; args[i] != '\0'; ++i) {
            tmp->args[i] = args[i];
        }
        tmp->args[i] = '\0';
        if (queue_head == 0) {
            queue_head = tmp;
        } else if (queue_head->nice_value > tmp->nice_value) {
            tmp->next = queue_head;
            queue_head = tmp;
        } else {
            queue_task *pre, *cur = queue_head;
            while (cur) {
                if (cur->nice_value > tmp->nice_value) {
                    pre->next = tmp;
                    tmp->next = cur;
                    break;
                }
                pre = cur;
                cur = cur->next;
            }
        }
        // // 處理queue裡面的handler
        while (queue_head) {
            // uart_printf("in priority queue\n");
            // 如果nice value比較小，卡住，直到priority較高的handler處理完，把nice value設回去3
            while (queue_head->nice_value > nc_value)
                asm volatile("nop");
            uart_printf("in priority queue\n");
            enable_interrupt();
            nc_value = queue_head->nice_value;
            (*(queue_head->callkback))(queue_head->args);
            queue_head = queue_head->next;
        }
        // after all handler finish, cur_nv set to original value->3
    }
    // 處理queue裡面的handler
    // while(queue_head){
    //   enable_interrupt();
    //   // uart_printf("in priority queue\n");
    //   nc_value = queue_head->nice_value;
    //   (*(queue_head->callkback))(queue_head->args);
    //   queue_head = queue_head->next;
    // }
    cur_nv = 3;
    // uart_printf("\nfinish at nice value = %d\n", nc_value);
}
