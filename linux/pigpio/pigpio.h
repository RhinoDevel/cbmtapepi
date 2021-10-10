
// Marcel Timm, RhinoDevel, 2021oct06

#ifndef MT_PIGPIO
#define MT_PIGPIO

#include <stdbool.h>
#include <pigpio.h>

#include "../../app/tape/tape_symbol.h"

/**
 * - Needs console already initialized.
 */
bool pigpio_init();

/**
 * - Overwrites maybe existing wave(-s)!
 * - Returns the created wave ID or -1 on failure.
 */
int pigpio_create_wave(
        uint32_t const gpio_pin_nr,
        uint8_t const * const symbols,
        int const symbol_count);

#endif //MT_PIGPIO
