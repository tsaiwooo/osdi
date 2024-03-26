#include "irq.h"

void irq_handle() {
  if (*IRQ_pending1 & AUX_INT) {
    // uart_printf("in uart handler\n");
    async_handler();
  } else if (*CORE0_INTERRUPT_SOURCE & 0x2) {
    core_timer_disable();
    // disable_interrupt();
    // unsigned int count, freq;
    // asm volatile("mrs  %[result], cntpct_el0" : [result] "=r"(count));
    // asm volatile("mrs  %[result], cntfrq_el0" : [result] "=r"(freq));
    // int time = (count) / freq;
    // uart_printf("[%d] core timer interrupt\n", time);
    // set_expired_time(2);
    // set_expired_time(1);
    // uart_printf("in timer irq\n");
    pop_irq();
  }
}

void add_irq(void (*callback)(char *), char *message, int seconds,
             int nice_value) {
  irq_task *tmp = (irq_task *)simple_malloc((unsigned long)sizeof(irq_task));
  tmp->seconds = seconds;
  tmp->trigger = current_time() + seconds;
  tmp->callback = callback;
  tmp->next = 0;
  int i;
  uart_printf("size = %d\n", sizeof(irq_task));
  for (i = 0; message[i] != '\0'; ++i) {
    tmp->message[i] = message[i];
  }
  tmp->message[i] = '\0';
  // strcpy(tmp->message, message);
  // tmp->message[sizeof(message) + 1] = '\0';
  uart_printf("add_irq,tmp->message = %s,seconds = %d,trigger =%d\n",
              tmp->message, tmp->seconds, tmp->trigger);
  irq_task *cur = timer_queue;
  irq_task *pre;
  // head
  if (!cur) {
    timer_queue = tmp;

    set_expired_time(tmp->seconds);
    core_timer_enable();
    // trigger時間比較早到,則改變value
  } else if (tmp->trigger <= cur->trigger) {
    tmp->next = cur;
    timer_queue = tmp;
    unsigned int spare_time = tmp->trigger - current_time();
    set_expired_time(spare_time);
    core_timer_enable();
  } else {
    while (cur) {
      if (tmp->trigger <= cur->trigger) {
        pre->next = tmp;
        tmp->next = cur;
        pre = cur;
        cur = cur->next;
        // set_expired_time(tmp->seconds);
        // core_timer_enable();

        return;
      }
      pre = cur;
      cur = cur->next;
    }
    pre->next = tmp;
  }
}

void pop_irq() {
  irq_task *cur = timer_queue;
  // uart_printf("in irq\n");
  if (!cur) {
    return;
  } else {
    (*(cur->callback))(cur->message);
    // cur->callback(cur->message);
    // head移到下一個
    unsigned int before_time, after_time;
    before_time = cur->trigger;
    cur = cur->next;
    if (cur != 0) {
      // uart_printf("not null\n");
      after_time = cur->trigger;

      set_expired_time(after_time - before_time);
      core_timer_enable();
    }
    timer_queue = cur;
    // while (1)
    //   ;
  }
}