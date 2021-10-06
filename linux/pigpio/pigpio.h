
// Marcel Timm, RhinoDevel, 2021oct06

#ifndef MT_PIGPIO
#define MT_PIGPIO

#include <stdbool.h>

/**
 * - Needs console already initialized.
 */
bool pigpio_init();

int pigpio_create_wave(
        uint32_t const gpio_pin_nr,
        uint8_t const * const symbols,
        int const symbol_count);

#endif //MT_PIGPIO
