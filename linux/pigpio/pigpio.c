
// Marcel Timm, RhinoDevel, 2021oct05

#include <stdbool.h>
#include <assert.h>
#include <pigpio.h>

#include "pigpio.h"
#include "../../lib/console/console.h"
#include "../../app/tape/tape_symbol.h"

static int const s_pulses_per_symbol = 4;

// TODO: Replace:
//
static uint32_t const s_micro_short = 176; // us
static uint32_t const s_micro_medium = 256; // us
static uint32_t const s_micro_long = 336; // us

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

    assert(s_pulses_per_symbol == 4);
    assert(gpio_pin_nr >= 1 && gpio_pin_nr <= 31);

    uint32_t const gpio_mask = (1 << gpio_pin_nr);

    // First pulse pair:

    out_pulses[0].gpioOn = gpio_mask; // (to-be-inverted by circuit)
    out_pulses[0].gpioOff = 0;        //
    out_pulses[0].usDelay = f;
    
    out_pulses[1].gpioOn = 0;          // (to-be-inverted by circuit)
    out_pulses[1].gpioOff = gpio_mask; //
    out_pulses[1].usDelay = f;

    // Second pulse pair:

    out_pulses[2].gpioOn = gpio_mask; // (to-be-inverted by circuit)
    out_pulses[2].gpioOff = 0;        //
    out_pulses[2].usDelay = l;
    
    out_pulses[3].gpioOn = 0;          // (to-be-inverted by circuit)
    out_pulses[3].gpioOff = gpio_mask; //
    out_pulses[3].usDelay = l;

    return true;
}

gpioPulse_t const * pigpio_create_pulses(
    uint32_t const gpio_pin_nr,
    enum tape_symbol const * const symbols,
    int const symbol_count,
    int * const out_pulse_count)
{
    gpioPulse_t * const pulses = malloc(
        s_pulses_per_symbol * symbol_count * (sizeof *pulses));

    *out_pulse_count = 0;

    for(int i = 0;i < symbol_count; ++i)
    {
        if(!fill_pulse_quadruple_from_symbol(
                symbols[i], gpio_pin_nr, pulses + s_pulses_per_symbol * i))
        {
            free(pulses);
            return NULL;
        }
    }
    out_pulse_count = s_pulses_per_symbol * symbol_count;
    return pulses;
}

int pigpio_create_wave(gpioPulse_t const * const pulses, int const pulse_count)
{
    // if(gpioWaveClear() != 0) // Clears ALL waveform data!
    // {
    //     return -1;
    // }

    // if(gpioWaveAddNew() != 0) // Not necessary.
    // {
    //     return -1;
    // }

    // Add pulses to wave:
    //
    if(gpioWaveAddGeneric(pulse_count, pulses) == PI_TOO_MANY_PULSES)
    {
        return -1;
    }

    int const wave_id = gpioWaveCreate();

    if(wave_id < 0)
    {
        return -1;
    }

#ifndef NDEBUG
    console_write("pigpio_create_wave: Wave with ID ");
    console_write_dword_dec((uint32_t)wave_id);
    console_write(" successfully created with ");
    console_write_dword_dec((uint32_t)pulse_count);
    console_writeline(" given pulses.");

    console_write("pigpio_create_wave: Waveform size is ");
    console_write_dword_dec((uint32_t)gpioWaveGetMicros());
    console_write("us / ");
    console_write_dword_dec((uint32_t)gpioWaveGetPulses());
    console_writeline(" pulses.");
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

#ifndef NDEBUG
    console_write("pigpio_init: Max. waveform size is ");
    console_write_dword_dec((uint32_t)gpioWaveGetMaxMicros());
    console_write("us / ");
    console_write_dword_dec((uint32_t)gpioWaveGetMaxPulses());
    console_writeline(" pulses.");
#endif //NDEBUG

    return true;
}
