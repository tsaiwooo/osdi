#ifndef _MAILBOX_H_
#define _MAILBOX_H_

#define MMIO_BASE       0x3f000000
#define MAILBOX_BASE    MMIO_BASE + 0xb880

#define MAILBOX_READ    MAILBOX_BASE
#define MAILBOX_STATUS  MAILBOX_BASE + 0x18
#define MAILBOX_WRITE   MAILBOX_BASE + 0x20

#define MAILBOX_EMPTY   0x40000000
#define MAILBOX_FULL    0x80000000
void get_board_revision();
void mailbox_call(int *);
#endif // _MAILBOX_H_