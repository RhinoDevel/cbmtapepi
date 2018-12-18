
// Marcel Timm, RhinoDevel, 2018jan24

#ifndef MT_ARMTIMER
#define MT_ARMTIMER

#include <stdint.h>

uint32_t armtimer_get_tick();

/** Start ARM time with 1 MHz frequency.
 *
 *  - Hard-coded for 250 MHz core clock.
 */
void armtimer_start_one_mhz();

void armtimer_busywait(uint32_t const start_val, uint32_t const divider);

#endif //MT_ARMTIMER
