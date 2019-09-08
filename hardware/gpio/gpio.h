
// Marcel Timm, RhinoDevel, 2018jan22

#ifndef MT_GPIO
#define MT_GPIO

#include <stdbool.h>
#include <stdint.h>

#include "gpio_params.h"

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

/** Busy-wait, until pin with given nr. is LOW.
 */
void gpio_wait_for_low(uint32_t const pin_nr);

/** Busy-wait, until pin with given nr. is HIGH.
 */
void gpio_wait_for_high(uint32_t const pin_nr);

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
