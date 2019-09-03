
// Marcel Timm, RhinoDevel, 2019sep03

#ifndef MT_SYSTIMER
#define MT_SYSTIMER

#include <stdint.h>

/**
 * - Runs at 1 MHz (independent of system clock).
 * - Uses lower 32 bits of system timer counter.
 */
uint32_t systimer_get_tick();

/**
 * - Uses lower 32 bits of system timer counter.
 */
void systimer_busywait_microseconds(uint32_t const microseconds);

#endif //MT_SYSTIMER
