
// Marcel Timm, RhinoDevel, 2019sep03

// Pages mentioned in source code comments can be found in the document
// "BCM2835 ARM Peripherals".
//
// Also you MUST USE https://elinux.org/BCM2835_datasheet_errata
// together with that document!

#include "systimer.h"
#include "../peribase.h"
#include "../../lib/mem/mem.h"

#include <stdbool.h>

// System timer (see page 172):
//
#define SYS_TIMER_BASE (PERI_BASE + 0x3000)
#define SYS_TIMER_CS (SYS_TIMER_BASE + 0) // Control/status
#define SYS_TIMER_CLO (SYS_TIMER_BASE + 4) // Lower 32 bits of counter.
// ...

uint32_t systimer_get_tick()
{
    return mem_read(SYS_TIMER_CLO);
}

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
