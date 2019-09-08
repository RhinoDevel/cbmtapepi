
// Marcel Timm, RhinoDevel, 2019aug24

#ifndef MT_GPIO_PARAMS
#define MT_GPIO_PARAMS

#include <stdint.h>
#include <stdbool.h>

struct gpio_params
{
    void (*wait_microseconds)(uint32_t const microseconds);

    uint32_t (*mem_read)(uint32_t const addr);
    void (*mem_write)(uint32_t const addr, uint32_t const val);

    uint32_t peri_base; // Peripheral base address.
};

#endif //MT_GPIO_PARAMS
