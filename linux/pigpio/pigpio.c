
// Marcel Timm, RhinoDevel, 2021oct05

#include <stdbool.h>
#include <assert.h>
#include <pigpio.h>

#include "pigpio.h"
#include "../../lib/console/console.h"
#include "../../app/tape/tape_symbol.h"

// TODO: Replace:
//
static uint32_t const s_micro_short = 176; // us
static uint32_t const s_micro_medium = 256; // us
static uint32_t const s_micro_long = 336; // us

static void fill_pulse_pair_from_micro(
    uint32_t const micro,
    uint32_t const gpio_pin_nr,
    gpioPulse_t * const out_pulses)
{
    out_pulses[0].gpioOn = gpio_pin_nr; // (to-be-inverted by circuit)
    out_pulses[0].gpioOff = 0;          //
    out_pulses[0].usDelay = micro;

    out_pulses[1].gpioOn = 0;            // (to-be-inverted by circuit)
    out_pulses[1].gpioOff = gpio_pin_nr; //
    out_pulses[1].usDelay = micro;
}

static bool fill_pulse_quadruple_from_symbol(
    enum tape_symbol const symbol,
    uint32_t const gpio_pin_nr,
    gpioPulse_t * const out_pulses)
{
    uint32_t f = 0, l = 0;

    switch(symbol)
    {
        case tape_symbol_zero:
        {
            f = s_micro_short;
            l = s_micro_medium;
            break;
        }
        case tape_symbol_one:
        {
            f = s_micro_medium;
            l = s_micro_short;
            break;
        }
        case tape_symbol_sync:
        {
            f = s_micro_short;
            l = s_micro_short;
            break;
        }
        case tape_symbol_new:
        {
            f = s_micro_long;
            l = s_micro_medium;
            break;
        }
        case tape_symbol_end: // Used for transmit block gap start, only.
        {
            f = s_micro_long;
            l = s_micro_short;
            break;
        }

        case tape_symbol_err: // (falls through)
        default: // Must not happen.
        {
            console_deb_writeline(
                "fill_pulse_quadruple_from_symbol: Error: Unknown symbol!");
            assert(false);
            return false;
        }
    }

    fill_pulse_pair_from_micro(f, gpio_pin_nr, out_pulses); // First pulse pair.
    fill_pulse_pair_from_micro(l, gpio_pin_nr, out_pulses + 2); // 2nd pair.
    return true;
}

int pigpio_create_wave(
        uint32_t const gpio_pin_nr,
        uint8_t const * const symbols,
        int const symbol_count)
{
    gpioPulse_t p[4];

    if(gpioWaveAddNew() != 0)
    {
        return -1;
    }

    for(int i = 0; i < symbol_count; ++i)
    {
        // Symbol to pulses:

        if(!fill_pulse_quadruple_from_symbol(
            (enum tape_symbol)symbols[i], gpio_pin_nr, p))
        {
            return -1;
        }

        // Add pulses to wave:
        //
        for(int j = 0; j < 4; ++j)
        {
            if(gpioWaveAddGeneric(1, p + j) == PI_TOO_MANY_PULSES)
            {
                return -1;
            }
        }
    }

    int const wave_id = gpioWaveCreate();

    if(wave_id < 0)
    {
        return -1;
    }

#ifndef NDEBUG
    console_write("pigpio_create_wave: Wave with ID ");
    console_write_dword_dec((uint32_t)wave_id);
    console_write(" successfully created from ");
    console_write_dword_dec((uint32_t)symbol_count);
    console_writeline(" symbols.");
#endif //NDEBUG

    return wave_id;
}

bool pigpio_init()
{
    int const result = gpioInitialise();

    if(result == PI_INIT_FAILED)
    {
        console_writeline("pigpio_init : Error: Initialization failed!");
        return false;
    }
    return true;
}
