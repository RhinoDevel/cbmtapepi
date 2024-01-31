
// Marcel Timm, RhinoDevel, 2021oct24

#ifndef MT_DMA_GPIO
#define MT_DMA_GPIO

#include <stdint.h>
#include <stdbool.h>

#include "dma_gpio_func.h"

#define GPIO_OFFSET_SET0 ((uint32_t)0x1C) // GPIO pin output set 0.
#define GPIO_OFFSET_CLR0 ((uint32_t)0x28) // GPIO pin output clear 0.

/** Set function of pin with given BCM nr. to given function.
 */
void dma_gpio_set_func(uint32_t const pin_nr, enum dma_gpio_func const func);

void dma_gpio_write(uint32_t const pin_nr, bool const high);

void dma_gpio_set_output(uint32_t const pin_nr, bool const high);

void dma_gpio_deinit();

void dma_gpio_init(void * const base_ptr);

#endif //MT_DMA_GPIO
