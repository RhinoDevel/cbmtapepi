
// Marcel Timm, RhinoDevel, 2019jul11

#ifndef MT_TAPE_RECEIVE_BUF
#define MT_TAPE_RECEIVE_BUF

#include <stdbool.h>
#include <stdint.h>

/** Receive data from Commodore datassette/datasette write-to-tape GPIO pin with
 *  given nr. and write it as symbols to given buffer.
 *
 *  See tape_extract_buf() for how to extract data from buffer.
 *
 *  - Does not care about sense line (must already be set to low).
 *  - Write-to-tape GPIO pin must already be configured as input with internal
 *    pull-down resistor.
 *  - Motor GPIO pin must already be configured as input with internal pull-down
 *    resistor.
 */
bool tape_receive_buf(
    uint32_t const gpio_pin_nr_motor,
    uint32_t const gpio_pin_nr_write,
    uint8_t * const buf);

#endif //MT_TAPE_RECEIVE_BUF
