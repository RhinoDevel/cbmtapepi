
// Marcel Timm, RhinoDevel, 2019jul11

#include <stdbool.h>
#include <stdint.h>

#include "tape_receive_buf.h"

bool tape_receive_buf(
    uint32_t const gpio_pin_nr_motor,
    uint32_t const gpio_pin_nr_write,
    uint8_t * const buf)
{
    (void)gpio_pin_nr_motor;
    (void)gpio_pin_nr_write;
    (void)buf;

    return false; // TODO: Implement!
}
