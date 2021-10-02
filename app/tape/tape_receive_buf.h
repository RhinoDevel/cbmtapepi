
// Marcel Timm, RhinoDevel, 2019jul11

#ifndef MT_TAPE_RECEIVE_BUF
#define MT_TAPE_RECEIVE_BUF

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Receive data from Commodore datassette/datasette write-to-tape GPIO pin with
 *  given nr. and write it as symbols to given buffer.
 *
 *  Returns count of symbols/bytes in buffer on success or -1 on failure.
 * 
 *  See tape_extract_buf() for how to extract data from buffer.
 *
 *  - Does not care about sense line (must already be set to low).
 *  - Write-to-tape GPIO pin must already be configured as input with internal
 *    pull-down resistor.
 *  - Motor GPIO pin must already be configured as input with internal pull-down
 *    resistor.
 */
int tape_receive_buf(
    uint32_t const gpio_pin_nr_motor,
    uint32_t const gpio_pin_nr_write,
    uint8_t * const buf,
    bool (*is_stop_requested)());

void tape_receive_buf_init(
    void (*timer_start_one_mhz)(), uint32_t (*timer_get_tick)());

#ifdef __cplusplus
}
#endif

#endif //MT_TAPE_RECEIVE_BUF
