
// Marcel Timm, RhinoDevel, 2018jan22

#ifndef MT_BAREGPIO
#define MT_BAREGPIO

#include <stdbool.h>
#include <stdint.h>

// GPIO pin functions:
//
enum gpio_func
{
    gpio_func_input = 0,
    gpio_func_output = 1,
    gpio_func_alt0 = 4,
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

void baregpio_write(uint32_t const pin_nr, bool const high);

bool baregpio_read(uint32_t const pin_nr);

/** Busy-wait, until pin with given nr. is LOW.
 */
void baregpio_wait_for_low(uint32_t const pin_nr);

/** Busy-wait, until pin with given nr. is HIGH.
 */
void baregpio_wait_for_high(uint32_t const pin_nr);

/** Set function of pin with given BCM nr. to given function.
 */
void baregpio_set_func(uint32_t const pin_nr, enum gpio_func const func);

void baregpio_set_pud(uint32_t const pin_nr, enum gpio_pud const pud);

void baregpio_set_output(uint32_t const pin_nr, bool const high);

void baregpio_set_input_pull_off(uint32_t const pin_nr);
void baregpio_set_input_pull_up(uint32_t const pin_nr);
void baregpio_set_input_pull_down(uint32_t const pin_nr);

#endif //MT_BAREGPIO
