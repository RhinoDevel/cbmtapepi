
// Marcel Timm, RhinoDevel, 2018jan22

// * This supports 32-bit Raspi processors (e.g. Raspi 1 & 2).

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

#include "baregpio.h"
#include "../peribase.h"
#include "../mem/mem.h"

#define GPIO_OFFSET 0x00200000

#define GPIO_BASE (PERI_BASE + GPIO_OFFSET)

static uint32_t const gpfsel0 = GPIO_BASE + 0; // GPIO function select 0.

// GPIO function codes:
//
static uint32_t const func_output = 1;

static uint32_t const gpset0 = GPIO_BASE + 0x1C; // GPIO pin output set 0.
static uint32_t const gpclr0 = GPIO_BASE + 0x28; // GPIO pin output clear 0.

/** Return address of GPFSEL register responsible for pin with given nr.
 */
static uint32_t get_gpfsel(uint32_t const pin_nr)
{
    return gpfsel0 + 4 * (pin_nr / 10); // 4 bytes for 10 pins.
}

static uint32_t get_gpset(uint32_t const pin_nr)
{
    return gpset0 + 4 * (pin_nr / 32); // 4 bytes for 32 pins.
}
static uint32_t get_gpclr(uint32_t const pin_nr)
{
    return gpclr0 + 4 * (pin_nr / 32); // 4 bytes for 32 pins.
}

static uint32_t get_pin_mask(uint32_t const pin_nr)
{
    return 1 << (pin_nr % 32);
}

/** Set function of pin with given BCM nr. to given function.
 */
static void set_func(uint32_t const pin_nr, uint32_t const func)
{
    uint32_t const gpfsel = get_gpfsel(pin_nr),
        shift = (pin_nr % 10) * 3,
        clear_mask = ~(7 << shift);
    uint32_t val = mem_read(gpfsel);

    val &= clear_mask;
    val |= func << shift;

    mem_write(gpfsel, val);
}

void baregpio_write(uint32_t const pin_nr, bool const high)
{
    uint32_t const pin_mask = get_pin_mask(pin_nr);

    if(high)
    {
        mem_write(get_gpset(pin_nr), pin_mask);
        return;
    }
    mem_write(get_gpclr(pin_nr), pin_mask);
}

void baregpio_set_output(uint32_t const pin_nr, bool const high)
{
    set_func(pin_nr, func_output);
    baregpio_write(pin_nr, high);
}
