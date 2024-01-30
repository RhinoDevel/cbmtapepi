
// Marcel Timm, RhinoDevel, 2021oct24

// GPIO pins (first 26 should be equal on at least Raspi 1 and 2)
// **************************************************************
//
//  1 3V3                  | 5V                    2
//    -------------------------------------------
//  3 BCM 2                | 5V                    4
//    -------------------------------------------
//  5 BCM 3                | GND                   6
//    -------------------------------------------
//  7 BCM 4                | BCM 14 / UART TXD     8
//    -------------------------------------------
//  9 GND                  | BCM 15 / UART RXD    10
//    -------------------------------------------
// 11 BCM 17               | BCM 18               12
//    -------------------------------------------
// 13 BCM 27               | GND                  14
//    -------------------------------------------
// 15 BCM 22               | BCM 23               16
//    -------------------------------------------
// 17 3V3                  | BCM 24               18
//    -------------------------------------------
// 19 BCM 10               | GND                  20
//    -------------------------------------------
// 21 BCM 9                | BCM 25               22
//    -------------------------------------------
// 23 BCM 11               | BCM 8                24
//    -------------------------------------------
// 25 GND                  | BCM 7                26
//    -------------------------------------------
//
// => At least 17 available GPIO pins on Raspi 1 and 2 GPIO expansion port:
//
//    BCM 2, 3, 4, 7, 8, 9, 10, 11, 14, 15, 17, 18, 22, 23, 24, 25, 27

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "gpio.h"
#include "gpio_func.h"
#include "../inf/inf.h"
#include "../reg.h"

#define OFFSET_FSEL0 ((uint32_t)0) // GPIO function select 0.
//
// [fsel1 will dynamically be used with the help of function get_fsel()]

// Get offset of GPFSEL register from base responsible for pin with given nr.:
//
#define GET_FSEL_OFFSET(PIN_NR) (OFFSET_FSEL0 + 4 * ((uint32_t)(PIN_NR) / 10))

#define GET_SET_OFFSET(PIN_NR) \
    (GPIO_OFFSET_SET0 + 4 * ((uint32_t)(PIN_NR) / 32))
#define GET_CLR_OFFSET(PIN_NR) \
    (GPIO_OFFSET_CLR0 + 4 * ((uint32_t)(PIN_NR) / 32))

#define GET_PIN_MASK(PIN_NR) (1 << ((uint32_t)(PIN_NR)) % 32)

static void * s_base_ptr = NULL; // Set gpio_init() and gpio_deinit().

void gpio_set_func(uint32_t const pin_nr, enum gpio_func const func)
{
    assert(s_base_ptr != NULL);

    uint32_t const fsel_offset = GET_FSEL_OFFSET(pin_nr);
    uint32_t const shift = (pin_nr % 10) * 3,
        clear_mask = ~(7 << shift);
    volatile uint32_t * const reg_ptr = REG_PTR(s_base_ptr, fsel_offset);
    uint32_t val = *reg_ptr;

    val &= clear_mask;
    val |= (uint32_t)func << shift;

    *reg_ptr = val;
}

void gpio_write(uint32_t const pin_nr, bool const high)
{
    uint32_t const pin_mask = GET_PIN_MASK(pin_nr),
        offset = high ? GET_SET_OFFSET(pin_nr) : GET_CLR_OFFSET(pin_nr);
    volatile uint32_t * const reg_ptr = REG_PTR(s_base_ptr, offset);

    *reg_ptr = pin_mask;
}

void gpio_set_output(uint32_t const pin_nr, bool const high)
{
    gpio_set_func(pin_nr, gpio_func_output);
    gpio_write(pin_nr, high);
}

void gpio_deinit()
{
    s_base_ptr = NULL;
}

void gpio_init(void * const base_ptr)
{
    assert(s_base_ptr == NULL);
    assert(base_ptr != NULL);

    s_base_ptr = base_ptr;
}
