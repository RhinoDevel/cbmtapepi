
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

#ifdef MT_LINUX
    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/mman.h>
#endif //MT_LINUX

#include "gpio.h"
#include "gpio_params.h"
#include "../../lib/mem/mem.h"

static uint32_t const s_offset_from_peribase = 0x00200000;

//static uint32_t const s_offset_fsel0 = 0; // GPIO function select 0.
#define OFFSET_FSEL0 ((uint32_t)0) // GPIO function select 0.
//
// [fsel1 will dynamically be used with the help of function get_fsel()]

//static uint32_t const s_offset_set0 = 0x1C; // GPIO pin output set 0.
#define OFFSET_SET0 ((uint32_t)0x1C) // GPIO pin output set 0.
//
// [set1 will dynamically be used with the help of function get_set()]

//static uint32_t const s_offset_clr0 = 0x28; // GPIO pin output clear 0.
#define OFFSET_CLR0 ((uint32_t)0x28) // GPIO pin output clear 0.
//
// [clr1 will dynamically be used with the help of function get_clr()]

static uint32_t const s_offset_lev0 = 0x34; // GPIO pin level 0.
//
// [lev1 will dynamically be used with the help of function get_lev()]

static uint32_t const s_offset_pud = 0x94; // GPIO pull-up/-down (page 100).

// GPIO pull-up/-down enable clock 0:
//
static uint32_t const s_offset_pudclk0 = 0x98;
//
// [pudclk1 will dyn. be used with the help of function get_pudclk()]

// Will be set in gpio_init():
//
static uint32_t s_addr_base = 0;
static void (*s_wait_microseconds)(uint32_t const microseconds) = 0;

/** Return address of GPFSEL register responsible for pin with given nr.
 */
// static uint32_t get_fsel(uint32_t const pin_nr)
// {
//     return s_addr_base + s_offset_fsel0 + 4 * (pin_nr / 10);
//     //
//     // 4 bytes for 10 pins.
// }
#define get_fsel(PIN_NR) (s_addr_base + OFFSET_FSEL0 + 4 * ((PIN_NR) / 10))

static uint32_t get_set(uint32_t const pin_nr)
{
    return s_addr_base + OFFSET_SET0 + 4 * (pin_nr / 32);
    //
    // 4 bytes for 32 pins.
}

static uint32_t get_clr(uint32_t const pin_nr)
{
    return s_addr_base + OFFSET_CLR0 + 4 * (pin_nr / 32);
    //
    // 4 bytes for 32 pins.
}
static uint32_t get_lev(uint32_t const pin_nr)
{
    return s_addr_base + s_offset_lev0 + 4 * (pin_nr / 32);
    //
    // 4 bytes for 32 pins.
}

/** Return address of GPPUDCLK register responsible for pin with given nr.
 */
static uint32_t get_pudclk(uint32_t const pin_nr)
{
    return s_addr_base + s_offset_pudclk0 + 4 * (pin_nr / 32);
    //
    // 4 bytes for 32 pins.
}

static uint32_t get_pin_mask(uint32_t const pin_nr)
{
    return 1 << (pin_nr % 32);
}

#ifdef MT_LINUX
/**
 * - Hard-coded for 32-bit processor/system.
 * - Returns 0 on error.
 */
static uint32_t get_addr_base_for_linux(uint32_t const peri_base)
{
    int mem_dev = -1;
    void* mem_map = 0;

    if(s_addr_base != 0)
    {
        //printf("Error: Seems to be already initialized!\n");
        return 0;
    }

    mem_dev = open("/dev/mem", O_RDWR|O_SYNC);
    if(mem_dev == -1)
    {
        //printf("Error: Can't open \"/dev/mem\"!\n");
        return 0;
    }

    mem_map = mmap(
        0, // Local mapping start address (NULL means don't care).
        4 * 1024, // Mapped memory block size.
        PROT_READ|PROT_WRITE, // Enable read and write.
        MAP_SHARED, // No exclusive access.
        mem_dev,
        peri_base + s_offset_from_peribase); // Offset to GPIO peripheral.

    close(mem_dev);
    mem_dev = -1;

    if(mem_map == MAP_FAILED)
    {
        //printf("Error: Failed to create memory mapping!\n");
        return 0;
    }

    return (uint32_t)mem_map;

    // [munmap() call is not necessary later,
    // because it will automatically unmap on process termination]
}
#endif //MT_LINUX

void gpio_set_pud(uint32_t const pin_nr, enum gpio_pud const val)
{
    uint32_t const pud = s_addr_base + s_offset_pud,
        pudclk = get_pudclk(pin_nr);

    // Set pull-up/-down mode:
    //
    mem_write(pud, val);
    s_wait_microseconds(1);

    mem_write(pudclk, get_pin_mask(pin_nr));
    s_wait_microseconds(1);

    // Maybe not necessary:
    //
    mem_write(pud, gpio_pud_off);
    mem_write(pudclk, 0);
}

void gpio_set_func(uint32_t const pin_nr, enum gpio_func const func)
{
    uint32_t const fsel = get_fsel(pin_nr),
        shift = (pin_nr % 10) * 3,
        clear_mask = ~(7 << shift);
    uint32_t val = mem_read(fsel);

    val &= clear_mask;
    val |= (uint32_t)func << shift;

    mem_write(fsel, val);
}

void gpio_write(uint32_t const pin_nr, bool const high)
{
    uint32_t const pin_mask = get_pin_mask(pin_nr);

    if(high)
    {
        mem_write(get_set(pin_nr), pin_mask);
        return;
    }
    mem_write(get_clr(pin_nr), pin_mask);
}

bool gpio_read(uint32_t const pin_nr)
{
    return (mem_read(get_lev(pin_nr)) & get_pin_mask(pin_nr)) != 0;
}

void gpio_wait_for_low(uint32_t const pin_nr)
{
    while(gpio_read(pin_nr))
    {
        // Pin is HIGH.
    }

    // Pin is LOW.
}

void gpio_wait_for_high(uint32_t const pin_nr)
{
    while(!gpio_read(pin_nr))
    {
        // Pin is LOW.
    }

    // Pin is HIGH.
}

void gpio_wait_for(
    uint32_t const pin_nr,
    bool const val,
    uint32_t const max_change_microseconds)
{
    while(true)
    {
        if(val)
        {
            gpio_wait_for_high(pin_nr);
        }
        else
        {
            gpio_wait_for_low(pin_nr);
        }

        if(max_change_microseconds == 0)
        {
            break; // No making-sure wanted. Done.
        }

        // Make sure that line is really at wanted value and not at a voltage
        // level between defined low and high regions:

        s_wait_microseconds(max_change_microseconds);

        if(gpio_read(pin_nr) == val)
        {
            break; // Still at wanted value. Done.
        }

        // At other level. Try again..
    }
}

void gpio_set_output(uint32_t const pin_nr, bool const high)
{
    gpio_set_func(pin_nr, gpio_func_output);
    gpio_write(pin_nr, high);
}

void gpio_set_input_pull_off(uint32_t const pin_nr)
{
    gpio_set_func(pin_nr, gpio_func_input);
    gpio_set_pud(pin_nr, gpio_pud_off);
}

void gpio_set_input_pull_up(uint32_t const pin_nr)
{
    gpio_set_func(pin_nr, gpio_func_input);
    gpio_set_pud(pin_nr, gpio_pud_up);
}

void gpio_set_input_pull_down(uint32_t const pin_nr)
{
    gpio_set_func(pin_nr, gpio_func_input);
    gpio_set_pud(pin_nr, gpio_pud_down);
}

void gpio_init(struct gpio_params const p)
{
#ifdef MT_LINUX
    s_addr_base = get_addr_base_for_linux(p.peri_base);
#else //MT_LINUX
    s_addr_base = p.peri_base + s_offset_from_peribase;
#endif //MT_LINUX

    s_wait_microseconds = p.wait_microseconds;
}
