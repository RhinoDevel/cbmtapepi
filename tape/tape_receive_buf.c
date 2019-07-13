
// Marcel Timm, RhinoDevel, 2019jul11

#include <stdbool.h>
#include <stdint.h>

#include "tape_receive_buf.h"
#include "tape_symbol.h"
#include "../baregpio/baregpio.h"
#include "../console/console.h"
#include "../armtimer/armtimer.h"
#include "../assert.h"

// TODO: Replace:
//
static uint32_t const micro_short = 176;
static uint32_t const micro_medium = 256;
static uint32_t const micro_long = 336;

// Set by tape_receive_buf() calling set_tick_limits():
//
static uint32_t tick_lim_short_medium;
static uint32_t tick_lim_medium_long;

static void set_tick_limits(uint32_t const ticks_short)
{
    uint32_t const ticks_medium = (ticks_short * micro_medium) / micro_short,
        ticks_long = (ticks_short * micro_long) / micro_short;

    tick_lim_short_medium = ticks_short + (ticks_medium - ticks_short) / 2;
    tick_lim_medium_long = ticks_medium + (ticks_long - ticks_medium) / 2;
}

static uint32_t get_pulse_type(uint32_t const tick_count)
{
    if(tick_count <= tick_lim_short_medium)
    {
        return micro_short; // Short pulse detected.
    }
    if(tick_count <= tick_lim_medium_long)
    {
        return micro_medium; // Medium pulse detected.
    }
    return micro_long; // Long pulse detected.
}

static enum tape_symbol get_symbol(
    uint32_t const f, uint32_t const l)
{
    if(f == micro_short && l == micro_medium)
    {
        return tape_symbol_zero;
    }
    if(f == micro_medium && l == micro_short)
    {
        return tape_symbol_one;
    }
    if(f == micro_short && l == micro_short)
    {
        return tape_symbol_sync;
    }
    if(f == micro_long && l == micro_medium)
    {
        return tape_symbol_new;
    }
    if(f == micro_long && l == micro_short)
    {
        return tape_symbol_end;
    }

    // Unhandled error (must not happen):
    //
    assert(false);
    console_write("get_symbol: Error: Unsupported pulse combination (");
    console_write_dword_dec(f);
    console_write(", ");
    console_write_dword_dec(l);
    console_writeline(")!");
    return tape_symbol_done; // Misused as error indicator.
}

bool tape_receive_buf(
    uint32_t const gpio_pin_nr_motor,
    uint32_t const gpio_pin_nr_write,
    uint8_t * const buf)
{
    static int const sync_count = 32; // (there are more than 1000 sync pulses)
    static uint32_t const ticks_timeout = 3000000; // 3 seconds.

    assert(sync_count % 2 == 0);

    uint32_t start_tick,
        ticks_short = 0,
        pulse_type[2];
    int pos = 0,
        pulse_type_index = 0,
        sync_workaround_count = 0;

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

        // Not necessary, but for completeness:
        //
        if(i % 2 == 1)
        {
            buf[pos] = tape_symbol_sync; // Each symbol represents TWO pulses.
            ++pos;
        }
    }

    assert(pos == 16);

    ticks_short = ticks_short / sync_count; // Calculate average value.

    set_tick_limits(ticks_short);

    while(true)
    {
        bool timeout_reached = false;

        // LOW, wait for start of (next) SAVE pulse:

        start_tick = armtimer_get_tick();
        while(true)
        {
            if(baregpio_read(gpio_pin_nr_write))
            {
                break; // HIGH
            }
            if(armtimer_get_tick() - start_tick >= ticks_timeout)
            {
                timeout_reached = true;
                break; // Timeout reached.
            }
        }
        if(timeout_reached)
        {
            break; // Done (or unhandled error).
        }

        // HIGH <=> A SAVE pulse has started.

        start_tick = armtimer_get_tick();

        baregpio_wait_for_low(gpio_pin_nr_write);

        // HIGH half of pulse finished.

        pulse_type[pulse_type_index] = get_pulse_type(
            armtimer_get_tick() - start_tick);

        if(pulse_type_index == 1)
        {
            if(pulse_type[0] == micro_short && pulse_type[1] == micro_long)
            {
                // Odd count of short (sync) pulses. => Abandon last sync pulse:

                pulse_type[0] = micro_long;
                // (pulse_type_index stays the same)
                ++sync_workaround_count;
                continue;
            }

            buf[pos] = get_symbol(pulse_type[0], pulse_type[1]);
            if(buf[pos] == tape_symbol_done) // Misused as error indicator.
            {
                return false;
            }
            ++pos;
        }

        pulse_type_index = 1 - pulse_type_index;
    }

    console_write("tape_receive_buf: Short tick count: ");
    console_write_dword_dec(ticks_short);
    console_writeline("");

    console_write("tape_receive_buf: Short/medium limit: ");
    console_write_dword_dec(tick_lim_short_medium);
    console_writeline("");
    console_write("tape_receive_buf: Medium/long limit: ");
    console_write_dword_dec(tick_lim_medium_long);
    console_writeline("");

    console_write("tape_receive_buf: Symbols read: ");
    console_write_dword_dec((uint32_t)pos);
    console_writeline("");

    if(sync_workaround_count > 0)
    {
        console_write("tape_receive_buf: Sync workaround applied ");
        console_write_dword_dec((uint32_t)sync_workaround_count);
        console_writeline(" times.");
    }

    return true;
}
