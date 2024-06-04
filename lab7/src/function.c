#include "function.h"

void delay(unsigned long long x)
{
    while (x--) {
        asm volatile("nop");
    }
}