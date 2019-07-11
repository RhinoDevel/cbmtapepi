
// Marcel Timm, RhinoDevel, 2018jan27

#ifndef MT_TAPE_TRANSFER_BUF
#define MT_TAPE_TRANSFER_BUF

#include <stdbool.h>
#include <stdint.h>

/** Send Commodore datassette/datasette symbols in given buffer via read-from-
 *  tape GPIO pin with given nr., until pseudo-symbol tape_symbol_done is found.
 *
 *  Pauses each time, when the motor signal gets LOW,
 *  until it is getting HIGH again OR exits (with return value true),
 *  if motor-wait was already disabled by pseudo-symbol
 *  tape_symbol_motor_wait_off.
 *
 *  See tape_fill_buf() for how to fill the buffer.
 *
 *  - Does not care about sense line (must already be set to low).
 *  - Read-from-tape GPIO pin must already be configured as output and set to
 *    HIGH.
 *  - Motor GPIO pin must already be configured as input without any internal
 *    pull resistor.
 */
bool tape_send_buf(
    uint8_t const * const buf,
    uint32_t const gpio_pin_nr_motor,
    uint32_t const gpio_pin_nr_read);

#endif //MT_TAPE_TRANSFER_BUF
