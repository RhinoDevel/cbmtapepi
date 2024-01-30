
// Marcel Timm, RhinoDevel, 2021oct24

#ifndef MT_GPIO
#define MT_GPIO

#include <stdint.h>
#include <stdbool.h>

#include "gpio_func.h"

#define GPIO_OFFSET_SET0 ((uint32_t)0x1C) // GPIO pin output set 0.
#define GPIO_OFFSET_CLR0 ((uint32_t)0x28) // GPIO pin output clear 0.

/** Set function of pin with given BCM nr. to given function.
 */
void gpio_set_func(uint32_t const pin_nr, enum gpio_func const func);

void gpio_write(uint32_t const pin_nr, bool const high);

void gpio_set_output(uint32_t const pin_nr, bool const high);

void gpio_deinit();

void gpio_init(void * const base_ptr);

#endif //MT_GPIO
