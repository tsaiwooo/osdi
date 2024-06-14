#ifndef _REBOOT_H_
#define _REBOOT_H_
#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001c
#define PM_WDOG 0x3F100024
void set_reboot(long, unsigned int);
void reset(int);
void cancel_reset();
#endif // _REBOOT_H_