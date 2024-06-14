#include "reboot.h"

void set_reboot(long addr, unsigned int value)
{
    volatile unsigned int* point = (unsigned int*)addr;
    *point = value;
}

void reset(int tick)
{ // reboot after watchdog timer expire
    set_reboot(PM_RSTC, PM_PASSWORD | 0x20); // full reset_reboot
    set_reboot(PM_WDOG, PM_PASSWORD | tick); // number of watchdog tick
}

void cancel_reset()
{
    set_reboot(PM_RSTC, PM_PASSWORD | 0); // full reset
    set_reboot(PM_WDOG, PM_PASSWORD | 0); // number of watchdog tick
}