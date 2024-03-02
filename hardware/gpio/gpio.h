
// Marcel Timm, RhinoDevel, 2018jan22

#ifndef MT_GPIO
#define MT_GPIO

#include <stdbool.h>
#include <stdint.h>

#include "gpio_params.h"

#define HIGH true
#define LOW false

// GPIO pin functions:
//
enum gpio_func
{
    gpio_func_input = 0,
    gpio_func_output = 1,
    gpio_func_alt0 = 4,
    gpio_func_alt3 = 7,
    gpio_func_alt5 = 2
    // ...
};

// GPIO pull-up/down values:
//
enum gpio_pud
{
    gpio_pud_off = 0,
    gpio_pud_down = 1,
    gpio_pud_up = 2
};

void gpio_write(uint32_t const pin_nr, bool const high);

bool gpio_read(uint32_t const pin_nr);

/** Enable rising edge detection for pin with given nr.
 */
void gpio_enable_ren(uint32_t const pin_nr);

/** Enable falling edge detection for pin with given nr.
 */
void gpio_disable_ren(uint32_t const pin_nr);

/** Enable falling edge detection for pin with given nr. 
 */
void gpio_enable_fen(uint32_t const pin_nr);

/** Disable falling edge detection for pin with given nr.
 */
void gpio_disable_fen(uint32_t const pin_nr);

/** Clear edge-detected flag for pin with given nr. 
 */
void gpio_clear_eds(uint32_t const pin_nr);

/** Returns value of edge-detected flag for pin with given nr.
 */
bool gpio_get_eds(uint32_t const pin_nr);

/** Busy-wait, until pin with given nr. is LOW.
 */
void gpio_wait_for_low(uint32_t const pin_nr, bool (*is_stop_requested)());

/** Busy-wait, until pin with given nr. is HIGH.
 */
void gpio_wait_for_high(uint32_t const pin_nr, bool (*is_stop_requested)());

/** First busy-wait, until pin with given nr. has given value (false = LOW,
 *  true = HIGH).
 *
 *  Then return immediately, if given microseconds value is zero
 *  [same functionality as gpio_wait_for_low() or gpio_wait_for_high()].
 *
 *  Otherwise (if microseconds value given is above zero) busy-wait for given
 *  timespan and after that check once, if value is still at wanted level.
 *
 *  Return, if it is. If not, restart with first busy-wait.
 *
 *  - Given microseconds timespan should be at least a little bit longer than
 *    maximum possible change time of signal to wanted value (the maximum rise
 *    time, if waiting for HIGH, the maximum fall time, if waiting for LOW).
 */
void gpio_wait_for(
    uint32_t const pin_nr,
    bool const val,
    uint32_t const max_change_microseconds);

/** Set function of pin with given BCM nr. to given function.
 */
void gpio_set_func(uint32_t const pin_nr, enum gpio_func const func);

void gpio_set_pud(uint32_t const pin_nr, enum gpio_pud const val);

void gpio_set_output(uint32_t const pin_nr, bool const high);

void gpio_set_input_pull_off(uint32_t const pin_nr);
void gpio_set_input_pull_up(uint32_t const pin_nr);
void gpio_set_input_pull_down(uint32_t const pin_nr);

/** Initialize Bare GPIO singleton before use.
 */
void gpio_init(struct gpio_params const p);

#endif //MT_GPIO
