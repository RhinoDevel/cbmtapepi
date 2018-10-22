
// Marcel Timm, RhinoDevel, 2018jan27

#ifndef MT_TAPE_TRANSFER_BUF
#define MT_TAPE_TRANSFER_BUF

#include <stdbool.h>
#include <stdint.h>

/** Send Commodore datassette/datasette symbols in given buffer via GPIO pin
 *  with given nr., until pseudo-symbol tape_symbol_done is found.
 *
 *  See tape_fill_buf() for how to fill the buffer.
 *
 *  - Does not care about sense line (must already be set to low).
 *  - GPIO pin must already be configured as output and set to HIGH.
 */
bool tape_transfer_buf(uint8_t const * const buf, uint32_t const gpio_pin_nr);

#endif //MT_TAPE_TRANSFER_BUF
