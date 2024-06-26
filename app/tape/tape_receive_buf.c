
// Marcel Timm, RhinoDevel, 2019jul11

#include <stdbool.h>
#include <stdint.h>

#include "tape_receive_buf.h"
#include "tape_symbol.h"
#include "../../hardware/gpio/gpio.h"
#include "../../lib/console/console.h"
#include "../../lib/assert.h"

// TODO: Replace:
//
static uint32_t const micro_short = 176;
static uint32_t const micro_medium = 256;
static uint32_t const micro_long = 336;

// Set by tape_receive_buf() calling set_tick_limits():
//
static uint32_t tick_lim_short_medium;
static uint32_t tick_lim_medium_long;

#ifndef NDEBUG
    static uint32_t tick_short_min;
    static uint32_t tick_long_max;
#endif //NDEBUG

// Initialized by tape_receive_buf_init():
//
static void (*s_timer_start_one_mhz)() = 0;
static uint32_t (*s_timer_get_tick)() = 0;

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
#ifndef NDEBUG
        if(tick_count < tick_short_min)
        {
            tick_short_min = tick_count;
        }
#endif //NDEBUG

        return micro_short; // Short pulse detected.
    }
    if(tick_count <= tick_lim_medium_long)
    {
        return micro_medium; // Medium pulse detected.
    }

#ifndef NDEBUG
    if(tick_count > tick_long_max)
    {
        tick_long_max = tick_count;
    }
#endif //NDEBUG

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

#ifndef NDEBUG
    console_write("get_symbol: Error: Unsupported pulse combination (");
    console_write_dword_dec(f);
    console_write(", ");
    console_write_dword_dec(l);
    console_writeline(")!");
#endif //NDEBUG

    return tape_symbol_err;
}

static bool wait_for(
    uint32_t const pin_nr, 
    bool const level_at_cbm,
    bool const is_inverted,
    bool (*is_stop_requested)())
{
    bool const level = is_inverted ? !level_at_cbm : level_at_cbm;

    // Wait for level:
    //
    while(gpio_read(pin_nr) != level)
    {
        // Pin is still not at wanted level.

        if(is_stop_requested != 0 && is_stop_requested())
        {
            return false;
        }
    }
    return true; // Pin is at wanted level.
}

/**
 * - Timer must already be running [s_timer_start_one_mhz()].
 */
static uint32_t get_ticks_short_average(
    uint32_t const gpio_pin_nr_write, bool (*is_stop_requested)())
{
    static int const skip_count = 128; //
    static int const sync_count = 64;  // (there are more than 1000 sync pulses)

    assert(sync_count % 2 == 0);

    uint32_t ticks_short = 0;

    // Wait for HIGH at CBM (not always HIGH, here - e.g. after former error):
    //
    if(!wait_for(gpio_pin_nr_write, HIGH, true, is_stop_requested))
    {
        return false;
    }

    // *************
    // *** Sync: ***
    // *************

    // Still HIGH at CBM.

    // Wait for LOW at CBM:
    //
    if(!wait_for(gpio_pin_nr_write, LOW, true, is_stop_requested))
    {
        return false;
    }
    //
    // => Enough time before next HIGH at CBM and the next HIGH should be the
    //    start of a(-nother) pulse.

    // Just got LOW at CBM.

    // Skip some initial level changes (because sometimes there is a preceding
    // level change that is NOT a sync pulse, because it is too long):
    //
    for(int i = 0;i < skip_count;++i)
    {
        // Still just got LOW at CBM.

        // Wait for start of (next) SAVE sync pulse:
        //
        if(!wait_for(gpio_pin_nr_write, HIGH, true, is_stop_requested))
        {
            return false;
        }

        // Just got HIGH at CBM. <=> A SAVE sync pulse has started.

        if(!wait_for(gpio_pin_nr_write, LOW, true, is_stop_requested))
        {
            return false;
        }

        // Just got LOW at CBM. <=> HIGH half of sync pulse at CBM is finished.
    }

    // Still just got LOW at CBM.

    // Sum up the tick lengths of some half sync pulses:
    //
    for(int i = 0;i < sync_count;++i)
    {
        // Still just got LOW at CBM.

        // Wait for start of (next) SAVE sync pulse:
        //
        if(!wait_for(gpio_pin_nr_write, HIGH, true, is_stop_requested))
        {
            return false;
        }

        // Just got HIGH at CBM. <=> A SAVE sync pulse has started.

        uint32_t const sync_start_tick = s_timer_get_tick();

        if(!wait_for(gpio_pin_nr_write, LOW, true, is_stop_requested))
        {
            return false;
        }

        // Just got LOW at CBM. <=> HIGH half of sync pulse (at CBM) finished.

        ticks_short += s_timer_get_tick() - sync_start_tick;

        // (do not add sync symbols to output buffer)
    }

    return ticks_short / sync_count; // Calculate and return average value.
}

int tape_receive_buf(
    uint32_t const gpio_pin_nr_motor,
    uint32_t const gpio_pin_nr_write,
    uint8_t * const buf,
    bool (*is_stop_requested)())
{
    static uint32_t const ticks_timeout = 3000000; // 3 seconds.

    uint32_t ticks_short = 0,
        ticks_long_timeout = 0,
        pulse_type[2];
    int ret_val = 0,
        pulse_type_index = 0,
        sync_workaround_count = 0;
    bool dyn_low_timeout_reached_once = false;

#ifndef NDEBUG
    tick_short_min = UINT32_MAX;
    tick_long_max = 0;
#endif //NDEBUG

    if(!gpio_read(gpio_pin_nr_motor))
    {
        console_deb_writeline("tape_receive_buf: Motor is OFF, waiting..");

        if(!wait_for(gpio_pin_nr_motor, HIGH, false, is_stop_requested))
        {
            return -1;
        }

        console_deb_writeline("tape_receive_buf: Motor is ON, starting..");
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
    s_timer_start_one_mhz();
    //
    // Does not reset to zero, but OK, because of modular arithmetic that (e.g.)
    // will be applied when subtracting one unsigned from another unsigned
    // integer (of the same size). This is working like a clock (e.g. 9 o'clock
    // plus 4 hours equals 1 o'clock and 1 o'clock less 4 hours equals
    // 9 o'clock).

    // Using loop, because VIC 20 also uses WRITE line for keyboard scanning
    // (great..!):
    //
    do
    {
        ticks_short = get_ticks_short_average(gpio_pin_nr_write, is_stop_requested);

#ifndef NDEBUG
        console_write("tape_receive_buf: Maybe invalid short tick count: ");
        console_write_dword_dec(ticks_short);
        console_writeline("");
#endif //NDEBUG
    }while(ticks_short >= 2 * micro_short);

    set_tick_limits(ticks_short);

    ticks_long_timeout = 2 * tick_lim_medium_long;

#ifndef NDEBUG
    console_write("tape_receive_buf: Short tick count: ");
    console_write_dword_dec(ticks_short);
    console_writeline("");
    console_write("tape_receive_buf: Short/medium limit: ");
    console_write_dword_dec(tick_lim_short_medium);
    console_writeline("");
    console_write("tape_receive_buf: Medium/long limit: ");
    console_write_dword_dec(tick_lim_medium_long);
    console_writeline("");

    console_write("tape_receive_buf: Dynamic long tick timeout: ");
    console_write_dword_dec(ticks_long_timeout);
    console_writeline("");
#endif //NDEBUG
    
    // Still LOW at CBM.

    // From here on, we are measuring the length of the cycles' LOW parts,
    // and no longer the HIGH parts (was necessary for additional PET BASIC v1
    // support):

    // Wait for HIGH at CBM:
    //
    if(!wait_for(gpio_pin_nr_write, HIGH, true, is_stop_requested))
    {
        return -1;
    }

    assert(ret_val == 0);
    while(true)
    {
        bool timeout_reached = false;

        // Still HIGH at CBM.

        uint32_t const high_start_tick = s_timer_get_tick();
        //
        // (the high level maybe was reached a while ago,
        //  but that does not matter, because it is just used to
        //  stop the reading, when the very long timeout is reached)

        // Wait for start of (next) LOW at CBM:

        while(true)
        {
            if(gpio_read(gpio_pin_nr_write))
            {
                break; // LOW at CBM (circuit inverts signal from CBM).
            }

            // Still HIGH at CBM.

            uint32_t const cur_tick = s_timer_get_tick();

            if(cur_tick - high_start_tick >= ticks_timeout)
            {
#ifndef NDEBUG
                console_write("tape_receive_buf: Static HIGH-at-CBM timeout ");
                console_write_dword_dec(ticks_timeout);
                console_writeline(" reached.");
#endif //NDEBUG
                timeout_reached = true;
                break; // Timeout reached.
            }
        }
        if(timeout_reached)
        {
            break; // DONE (or unhandled error).
        }

        // Just got LOW at CBM. <=> A SAVE pulse's low part has started.

        uint32_t const low_start_tick = s_timer_get_tick();

        // Wait for HIGH at CBM:
        //
        while(true)
        {
            if(!gpio_read(gpio_pin_nr_write))
            {
                break; // HIGH at CBM (circuit inverts signal from CBM).
            }

            // Still LOW at CBM.

            uint32_t const cur_tick = s_timer_get_tick();

            if(cur_tick - low_start_tick >= ticks_long_timeout)
            {
#ifndef NDEBUG
                console_write("tape_receive_buf: Dynamic LOW-at-CBM timeout ");
                console_write_dword_dec(ticks_long_timeout);
                console_writeline(" reached.");
#endif //NDEBUG
                timeout_reached = true;
                break; // Timeout reached.
            }
        }
        if(timeout_reached)
        {
            if(dyn_low_timeout_reached_once)
            {
                console_deb_writeline(
                    "tape_receive_buf: It was the second dynamic LOW timeout, breaking loop (hopefully DONE)..");
                break;
            }
            dyn_low_timeout_reached_once = true;

            // Ignore current LOW level "pulse" (just once):

            // Wait for HIGH at CBM:
            //
            if(!wait_for(gpio_pin_nr_write, HIGH, true, is_stop_requested))
            {
                return -1;
            }
            console_deb_writeline(
                "tape_receive_buf: HIGH at CBM again (ignoring last LOW).");
            continue;
        }

        // Just got HIGH at CBM. <=> LOW half at CBM finished.

        pulse_type[pulse_type_index] = get_pulse_type(
            s_timer_get_tick() - low_start_tick);

        if(ret_val == 0 
            && pulse_type_index == 0 
            && pulse_type[pulse_type_index] == micro_short)
        {
            continue; // Skips all leading sync pulses.
        }

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

            buf[ret_val] = get_symbol(pulse_type[0], pulse_type[1]);
            if(buf[ret_val] == tape_symbol_err)
            {
                return -1;
            }
            ++ret_val;
        }

        pulse_type_index = 1 - pulse_type_index;
    }

#ifndef NDEBUG
    console_write("tape_receive_buf: Min. tick count that was interpreted as short pulse: ");
    console_write_dword_dec(tick_short_min);
    console_writeline("");
    console_write("tape_receive_buf: Max. tick count that was interpreted as long pulse: ");
    console_write_dword_dec(tick_long_max);
    console_writeline("");

    console_write("tape_receive_buf: Symbols read: ");
    console_write_dword_dec((uint32_t)ret_val);
    console_writeline("");

    if(sync_workaround_count > 0)
    {
        console_write("tape_receive_buf: Sync workaround applied ");
        console_write_dword_dec((uint32_t)sync_workaround_count);
        console_writeline(" times.");
    }
#endif //NDEBUG

    return ret_val;
}

void tape_receive_buf_init(
    void (*timer_start_one_mhz)(), uint32_t (*timer_get_tick)())
{
    assert(s_timer_start_one_mhz == 0);
    assert(s_timer_get_tick == 0);

    s_timer_start_one_mhz = timer_start_one_mhz;
    s_timer_get_tick = timer_get_tick;
}
