#include "mailbox.h"

void sysinfo()
{
  get_info(MBOX_BOARD_REVISION,4);
  uart_printf("BOARD REVISION : 0x%x\n",mbox[5]); // it should be 0xa020d3 for rpi3 b+
  get_info(MBOX_ARM_MEMORY,8);
  if(mbox[5]==0){
    uart_printf("ARM base address : 0x00000\n");
    uart_printf("ARM memory size : 0x%x\n",mbox[6]);
  }else{
    uart_printf("ARM base address : 0x%x\n",mbox[5]);
    uart_printf("ARM memory size : 0x%x\n",mbox[6]);
  }
  // uart_printf("ARM memory size : 0x%x\n",mbox[6]);

}

void get_info(unsigned int tags,int size){
  mbox[0] = (6 + (size/4)) * 4; // buffer size in bytes
  mbox[1] = REQUEST_CODE;
  // tags begin
  mbox[2] = tags; // tag identifier
  mbox[3] = size; // maximum of request and response value buffer's length.
  mbox[4] = TAG_REQUEST_CODE;
  mbox[5] = 0; // value buffer
  // tags end
  mbox[6] = END_TAG;

  mailbox_call(MBOX_CH_PROP); // message passing procedure call, you should implement it following the 6 steps provided above.
}

int mailbox_call(unsigned char ch)
{
    unsigned int r = (((unsigned int)((unsigned long)&mbox)&~0xF) | (ch&0xF));
    /* wait until we can write to the mailbox */
    do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_FULL);
    /* write the address of our message to the mailbox with channel identifier */
    *MBOX_WRITE = r;
    /* now wait for the response */
    while(1) {
        /* is there a response? */
        do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_EMPTY);
        /* is it a response to our message? */
        if(r == *MBOX_READ)
            /* is it a valid successful response? */
            return mbox[1]==MBOX_RESPONSE;
    }
    return 0;
}