
// Marcel Timm, RhinoDevel, 2019sep03

// Pages mentioned in source code comments can be found in the document
// "BCM2835 ARM Peripherals".
//
// Also you MUST USE https://elinux.org/BCM2835_datasheet_errata
// together with that document!

#include "systimer.h"

#include <stdbool.h>

// System timer (see page 172):
//
// SYS_TIMER_BASE is defined in header file.
// ...
// SYS_TIMER_CLO is defined in header file.
// ...

// uint32_t systimer_get_tick()
// {
//     return mem_read(SYS_TIMER_CLO);
// }

void systimer_busywait_microseconds(uint32_t const microseconds)
{
    uint32_t rx;

    rx = systimer_get_tick();
    while(true)
    {
        if(systimer_get_tick() - rx >= microseconds)
        {
            return;
        }
    }
    //assert(false);
}
