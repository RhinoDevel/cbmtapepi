
// Marcel Timm, RhinoDevel, 2018jan24

#ifndef MT_ARMTIMER
#define MT_ARMTIMER

#include <stdint.h>

void armtimer_busywait(uint32_t const start_val, uint32_t const divider);

#endif //MT_ARMTIMER
