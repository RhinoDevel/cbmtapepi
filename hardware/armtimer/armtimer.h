
// Marcel Timm, RhinoDevel, 2018jan24

#ifndef MT_ARMTIMER
#define MT_ARMTIMER

#include <stdint.h>

#include "../peribase.h"

// ARM Timer (based on an SP804 - NOT an "AP804" -, see page 196):
//
#define ARM_TIMER_BASE (PERI_BASE + 0xB000)
// (see C file for other addresses)
#define ARM_TIMER_CLI (ARM_TIMER_BASE + 0x40C) // IRQ clear/ACK (write only).
// (see C file for other addresses)

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
//void armtimer_irq_clear();
#define armtimer_irq_clear() (mem_write(ARM_TIMER_CLI, 0))

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
