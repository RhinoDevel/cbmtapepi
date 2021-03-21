
// Marcel Timm, RhinoDevel, 2019sep03

#ifndef MT_SYSTIMER
#define MT_SYSTIMER

#include "../../lib/mem/mem.h"
#include "../peribase.h"

#include <stdint.h>

// System timer (see page 172):
//
#define SYS_TIMER_BASE (PERI_BASE + 0x3000)
#define SYS_TIMER_CLO (SYS_TIMER_BASE + 4) // Lower 32 bits of counter.

/**
 * - Runs at 1 MHz (independent of system clock).
 * - Uses lower 32 bits of system timer counter.
 */
//uint32_t systimer_get_tick();
#define systimer_get_tick() (mem_read(SYS_TIMER_CLO))

/**
 * - Uses lower 32 bits of system timer counter.
 */
void systimer_busywait_microseconds(uint32_t const microseconds);

#endif //MT_SYSTIMER
