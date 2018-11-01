
// Marcel Timm, RhinoDevel, 2018jan27

#include <stdbool.h>
#include <stdint.h>

#include "tape_transfer_buf.h"
#include "tape_symbol.h"
#include "../busywait/busywait.h"
#include "../baregpio/baregpio.h"
#include "../console/console.h"

// Short: 2840 Hz
//
// 1 / (2840 Hz) = 352 microseconds
//
// 352 microseconds / 2 = 176 microseconds
//
static uint32_t const micro_short = 176;

// Medium: 1953 Hz
//
// 1 / (1953 Hz) = 512 microseconds
//
// 512 microseconds / 2 = 256 microseconds
//
static uint32_t const micro_medium = 256;

// Long: 1488 Hz
//
// 1 / (1488 Hz) = 672 microseconds
//
// 672 microseconds / 2 = 336 microseconds
//
static uint32_t const micro_long = 336;

// Pause:
//
static uint32_t const milli_pause = 5000; // 5 seconds.

static void transfer_pause(uint32_t const gpio_pin_nr)
{
    baregpio_write(gpio_pin_nr, false);
    busywait_milliseconds(milli_pause);
}

static void transfer_pulse(uint32_t const micro, uint32_t const gpio_pin_nr)
{
    baregpio_write(gpio_pin_nr, false);
    busywait_microseconds(micro);
    baregpio_write(gpio_pin_nr, true);
    busywait_microseconds(micro);
}

static void transfer_symbol(
    uint32_t const micro_first,
    uint32_t const micro_last,
    uint32_t const gpio_pin_nr)
{
    transfer_pulse(micro_first, gpio_pin_nr);
    transfer_pulse(micro_last, gpio_pin_nr);
}

bool tape_transfer_buf(
    uint8_t const * const buf,
    uint32_t const gpio_pin_nr_motor,
    uint32_t const gpio_pin_nr_read)
{
    int i = 0;

    // As pulse length detection triggers on descending (negative) edges,
    // GPIO pin's current output value is expected to be set to HIGH.

    while(true)
    {
        uint32_t f = 0, l = 0;

        if(buf[i] == tape_symbol_done)
        {
            console_writeline("Done pseudo-symbol found. Stopping transfer.");
            return true; // Transfer done (fake symbol, don't care about motor).
        }

        while(!baregpio_read(gpio_pin_nr_motor))
        {
            // Pause, as long as motor signal from Commodore computer is LOW.

            console_writeline("Motor is OFF, waiting..");
        }

        switch(buf[i])
        {
            case tape_symbol_zero:
            {
                f = micro_short;
                l = micro_medium;
                break;
            }
            case tape_symbol_one:
            {
                f = micro_medium;
                l = micro_short;
                break;
            }
            case tape_symbol_sync:
            {
                f = micro_short;
                l = micro_short;
                break;
            }
            case tape_symbol_new:
            {
                f = micro_long;
                l = micro_medium;
                break;
            }
            case tape_symbol_end: // Used for transmit block gap start, only.
            {
                f = micro_long;
                l = micro_short;
                break;
            }

            case tape_symbol_pause:
            {
                break; // Handled below!
            }
            case tape_symbol_done:
            {
                return false; // Error (really handled, above)!
            }

            default: // Must not happen.
            {
                return false; // Error!
            }
        }
        if(buf[i] == tape_symbol_pause)
        {
            transfer_pause(gpio_pin_nr_read);
        }
        else
        {
            transfer_symbol(f, l, gpio_pin_nr_read);
        }
        ++i;
    }
}
