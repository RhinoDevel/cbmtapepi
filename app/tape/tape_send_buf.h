
// Marcel Timm, RhinoDevel, 2018jan27

#ifndef MT_TAPE_SEND_BUF
#define MT_TAPE_SEND_BUF

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
 *  - Motor GPIO pin must already be configured as input with internal pull-down
 *    resistor.
 */
bool tape_send_buf(
    uint8_t const * const buf,
    uint32_t const gpio_pin_nr_motor,
    uint32_t const gpio_pin_nr_read,
    bool (*is_stop_requested)());

void tape_send_buf_init(
    void (*timer_busywait_microseconds)(uint32_t const microseconds),
    void (*gpio_write)(uint32_t const pin_nr, bool const high),
    bool (*gpio_read)(uint32_t const pin_nr));

#endif //MT_TAPE_SEND_BUF
