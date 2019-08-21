
// Marcel Timm, RhinoDevel, 2018jan22

// Pages mentioned in source code comments can be found in the document
// "BCM2835 ARM Peripherals".
//
// Also you MUST USE https://elinux.org/BCM2835_datasheet_errata
// together with that document!

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
#include "../../mem/mem.h"
#include "../armtimer/armtimer.h"

#define GPIO_OFFSET 0x00200000

#define GPIO_BASE (PERI_BASE + GPIO_OFFSET)

static uint32_t const gpfsel0 = GPIO_BASE + 0; // GPIO function select 0.
// ["gpfsel1" will dynamically be used with the help of function get_gpfsel()]

static uint32_t const gpset0 = GPIO_BASE + 0x1C; // GPIO pin output set 0.
// ["gpset1" will dynamically be used with the help of function get_gpset()]
static uint32_t const gpclr0 = GPIO_BASE + 0x28; // GPIO pin output clear 0.
// ["gpclr1" will dynamically be used with the help of function get_gpclr()]

static uint32_t const gplev0 = GPIO_BASE + 0x34; // GPIO pin level 0.
// ["gplev1" will dynamically be used with the help of function get_gplev()]

static uint32_t const gppud = // GPIO pull-up/-down (page 100).
    GPIO_BASE + 0x94;

static uint32_t const gppudclk0 = // GPIO pull-up/-down enable clock 0.
    GPIO_BASE + 0x98;
// ["gppudclk1" will dyn. be used with the help of function get_gppudclk()]

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
static uint32_t get_gplev(uint32_t const pin_nr)
{
    return gplev0 + 4 * (pin_nr / 32); // 4 bytes for 32 pins.
}

/** Return address of GPPUDCLK register responsible for pin with given nr.
 */
static uint32_t get_gppudclk(uint32_t const pin_nr)
{
    return gppudclk0 + 4 * (pin_nr / 32); // 4 bytes for 32 pins.
}

static uint32_t get_pin_mask(uint32_t const pin_nr)
{
    return 1 << (pin_nr % 32);
}

void baregpio_set_pud(uint32_t const pin_nr, enum gpio_pud const pud)
{
    uint32_t const gppudclk = get_gppudclk(pin_nr);

    // Set pull-up/-down mode:
    //
    mem_write(gppud, pud);
    armtimer_busywait_microseconds(1);

    mem_write(gppudclk, get_pin_mask(pin_nr));
    armtimer_busywait_microseconds(1);

    // Maybe not necessary:
    //
    mem_write(gppud, gpio_pud_off);
    mem_write(gppudclk, 0);
}

void baregpio_set_func(uint32_t const pin_nr, enum gpio_func const func)
{
    uint32_t const gpfsel = get_gpfsel(pin_nr),
        shift = (pin_nr % 10) * 3,
        clear_mask = ~(7 << shift);
    uint32_t val = mem_read(gpfsel);

    val &= clear_mask;
    val |= (uint32_t)func << shift;

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

bool baregpio_read(uint32_t const pin_nr)
{
    return (mem_read(get_gplev(pin_nr)) & get_pin_mask(pin_nr)) != 0;
}

void baregpio_wait_for_low(uint32_t const pin_nr)
{
    while(baregpio_read(pin_nr))
    {
        // Pin is HIGH.
    }

    // Pin is LOW.
}

void baregpio_wait_for_high(uint32_t const pin_nr)
{
    while(!baregpio_read(pin_nr))
    {
        // Pin is LOW.
    }

    // Pin is HIGH.
}

void baregpio_set_output(uint32_t const pin_nr, bool const high)
{
    baregpio_set_func(pin_nr, gpio_func_output);
    baregpio_write(pin_nr, high);
}

void baregpio_set_input_pull_off(uint32_t const pin_nr)
{
    baregpio_set_func(pin_nr, gpio_func_input);
    baregpio_set_pud(pin_nr, gpio_pud_off);
}

void baregpio_set_input_pull_up(uint32_t const pin_nr)
{
    baregpio_set_func(pin_nr, gpio_func_input);
    baregpio_set_pud(pin_nr, gpio_pud_up);
}

void baregpio_set_input_pull_down(uint32_t const pin_nr)
{
    baregpio_set_func(pin_nr, gpio_func_input);
    baregpio_set_pud(pin_nr, gpio_pud_down);
}
