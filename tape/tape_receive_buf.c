
// Marcel Timm, RhinoDevel, 2019jul11

#include <stdbool.h>
#include <stdint.h>

#include "tape_receive_buf.h"
#include "../baregpio/baregpio.h"
#include "../console/console.h"
#include "../armtimer/armtimer.h"

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

bool tape_receive_buf(
    uint32_t const gpio_pin_nr_motor,
    uint32_t const gpio_pin_nr_write,
    uint8_t * const buf)
{
    (void)buf;

    static int const sync_count = 32; // (there are more than 1000 sync pulses)

    uint32_t start_tick,
        //tick_count,
        ticks_short = 0,
        ticks_medium,
        ticks_long;

    if(!baregpio_read(gpio_pin_nr_motor))
    {
        console_writeline("tape_receive_buf: Motor is OFF, waiting..");

        while(!baregpio_read(gpio_pin_nr_motor))
        {
            // Pause, as long as motor signal from Commodore computer is
            // LOW.
        }

        console_writeline("tape_receive_buf: Motor is ON, starting..");
    }

    // 1 MHz <=> 1,000,000 ticks per second.
    //
    // 32 bit wide counter <=> 2^32 values.
    //
    // => More than 71 minutes until wrap-around.
    //
    // Fastest Commodore pulse frequency is 2840 Hz. <=> T = 352 microseconds.
    //
    // => At least 352 ticks for each Commodore pulse, 176 for half a pulse.
    //
    armtimer_start_one_mhz();

    // *************
    // *** Sync: ***
    // *************

    baregpio_wait_for_low(gpio_pin_nr_write);
    //
    // => Next high will be the start of a pulse.

    // Sum up the tick lengths of some half sync pulses:
    //
    for(int i = 0;i < sync_count;++i)
    {
        // LOW, wait for start of (next) SAVE sync pulse:

        baregpio_wait_for_high(gpio_pin_nr_write);

        // HIGH <=> A SAVE sync pulse has started.

        start_tick = armtimer_get_tick();

        baregpio_wait_for_low(gpio_pin_nr_write);

        // HIGH half of sync pulse finished.

        ticks_short += armtimer_get_tick() - start_tick;
    }

    // Calculate actual pulse half lengths in ticks:
    //
    ticks_short = ticks_short / sync_count;
    ticks_medium = (ticks_short * micro_medium) / micro_short;
    ticks_long = (ticks_short * micro_long) / micro_short;

    //tick_count = armtimer_get_tick() - start_tick;

    console_write("tape_receive_buf: Short tick count: ");
    console_write_dword_dec(ticks_short);
    console_writeline("");
    console_write("tape_receive_buf: Medium tick count: ");
    console_write_dword_dec(ticks_medium);
    console_writeline("");
    console_write("tape_receive_buf: Long tick count: ");
    console_write_dword_dec(ticks_long);
    console_writeline("");

    return false; // TODO: Implement!
}
