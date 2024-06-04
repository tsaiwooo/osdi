#ifndef _MAILBOX_H_
#define _MAILBOX_H_
#include "gpio.h"
#include "mm.h"
#include "uart.h"
/* a properly aligned buffer */
// extern volatile unsigned int mbox[36];

#define MBOX_REQUEST 0

/* channels */
#define MBOX_CH_POWER 0
#define MBOX_CH_FB 1
#define MBOX_CH_VUART 2
#define MBOX_CH_VCHIQ 3
#define MBOX_CH_LEDS 4
#define MBOX_CH_BTNS 5
#define MBOX_CH_TOUCH 6
#define MBOX_CH_COUNT 7
#define MBOX_CH_PROP 8

/* tags */
#define MBOX_TAG_GETSERIAL 0x10004
#define MBOX_BOARD_REVISION 0x10002
#define MBOX_ARM_MEMORY 0x10005
#define MBOX_TAG_LAST 0

/* mailbox message buffer */
volatile unsigned int __attribute__((aligned(16))) mbox[8];

#define VIDEOCORE_MBOX (MMIO_BASE + 0x0000B880)
#define MBOX_READ ((volatile unsigned int*)(VIDEOCORE_MBOX + 0x0))
#define MBOX_POLL ((volatile unsigned int*)(VIDEOCORE_MBOX + 0x10))
#define MBOX_SENDER ((volatile unsigned int*)(VIDEOCORE_MBOX + 0x14))
#define MBOX_STATUS ((volatile unsigned int*)(VIDEOCORE_MBOX + 0x18))
#define MBOX_CONFIG ((volatile unsigned int*)(VIDEOCORE_MBOX + 0x1C))
#define MBOX_WRITE ((volatile unsigned int*)(VIDEOCORE_MBOX + 0x20))
#define MBOX_RESPONSE 0x80000000
#define MBOX_FULL 0x80000000
#define MBOX_EMPTY 0x40000000

#define REQUEST_CODE 0x00000000
#define REQUEST_SUCCEED 0x80000000
#define REQUEST_FAILED 0x80000001
#define TAG_REQUEST_CODE 0x00000000
#define END_TAG 0x00000000

void sysinfo();
int mailbox_call(unsigned char);
int sys_mailbox(unsigned char, unsigned int*);
void get_info(unsigned int, int);
void get_info_mem(unsigned int, int);
#endif // _MAILBOX_H_