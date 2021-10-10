
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
 * - Caller takes ownership of returned object.
 * - Returns NULL on error.
 */
gpioPulse_t* pigpio_create_pulses(
    uint32_t const gpio_pin_nr,
    uint8_t const * const symbols,
    int const symbol_count,
    int * const out_pulse_count);

/**
 * - Returns the created wave ID or -1 on failure.
 */
int pigpio_create_wave(gpioPulse_t * const pulses, int const pulse_count);

#endif //MT_PIGPIO
