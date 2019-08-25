
// Marcel Timm, RhinoDevel, 2019jul11

#ifndef MT_TAPE_RECEIVE_BUF
#define MT_TAPE_RECEIVE_BUF

#include <stdbool.h>
#include <stdint.h>

/** Receive data from Commodore datassette/datasette write-to-tape GPIO pin with
 *  given nr. and write it as symbols to given buffer.
 *
 *  Adds pseudo-symbol tape_symbol_done after received data in buffer.
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
    uint8_t * const buf,
    bool (*is_stop_requested)());

void tape_receive_buf_init(
    void (*timer_start_one_mhz)(), uint32_t (*timer_get_tick)());

#endif //MT_TAPE_RECEIVE_BUF
