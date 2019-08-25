
// Marcel Timm, RhinoDevel, 2018dec26

#ifndef MT_TAPE_INIT
#define MT_TAPE_INIT

#include <stdint.h>

void tape_init(
    void (*timer_start_one_mhz)(),
    uint32_t (*timer_get_tick)(),
    void (*timer_busywait_microseconds)(uint32_t const microseconds));

#endif //MT_TAPE_INIT
