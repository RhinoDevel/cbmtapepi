
// Marcel Timm, RhinoDevel, 2018jan24

#ifndef MT_ARMTIMER
#define MT_ARMTIMER

#include <stdint.h>

/**
 * - Uses free running counter.
 */
uint32_t armtimer_get_tick();

/**
 * - Uses free running counter.
 */
void armtimer_busywait_microseconds(uint32_t const microseconds);

/** Start free running counter with 1 MHz frequency.
 *
 *  - Will NOT restart, if already running(!).
 *  - Hard-coded for 250 MHz core clock.
 */
void armtimer_start_one_mhz();

/** Call this once during timer interrupt initialization [see armtimer_start()]
 *  and from the IRQ service routine when entered.
 * 
 *  DON'T call this from anywhere else!
 */
void armtimer_irq_clear();

/**
 * - Does NOT use free running counter, but timer.
 */
void armtimer_busywait(uint32_t const start_val, uint32_t const divider);

/**
 * - Does NOT use free running counter, but timer.
 * - NO busy wait.
 * - Enables interrupt.
 */
void armtimer_start(uint32_t const start_val, uint32_t const divider);

#endif //MT_ARMTIMER
