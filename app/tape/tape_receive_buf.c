
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

    return tape_symbol_done; // Misused as error indicator.
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

bool tape_receive_buf(
    uint32_t const gpio_pin_nr_motor,
    uint32_t const gpio_pin_nr_write,
    uint8_t * const buf,
    bool (*is_stop_requested)())
{
    static int const skip_count = 128; //
    static int const sync_count = 64; // (there are more than 1000 sync pulses)
    static uint32_t const ticks_timeout = 3000000; // 3 seconds.

    assert(sync_count % 2 == 0);

    uint32_t ticks_short = 0,
        pulse_type[2];
    int pos = 0,
        pulse_type_index = 0,
        sync_workaround_count = 0;
    bool flipLevel = false;

#ifndef NDEBUG
    tick_short_min = UINT32_MAX;
    tick_long_max = 0;
#endif //NDEBUG

    if(!gpio_read(gpio_pin_nr_motor))
    {
        console_deb_writeline("tape_receive_buf: Motor is OFF, waiting..");

        if(!wait_for(gpio_pin_nr_motor, HIGH, false, is_stop_requested))
        {
            return false;
        }

        console_deb_writeline("tape_receive_buf: Motor is ON, starting..");
    }

    // Wait for HIGH at CBM (not always HIGH, here - e.g. if former error):
    //
    if(!wait_for(gpio_pin_nr_write, HIGH, true, is_stop_requested))
    {
        return false;
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

    // *************
    // *** Sync: ***
    // *************

    // Wait for LOW at CBM (must be HIGH at CBM, here - see above):
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

        // Not necessary, but for completeness:
        //
        if(i % 2 == 1)
        {
            buf[pos] = tape_symbol_sync; // Each symbol represents TWO pulses.
            ++pos;
        }
    }
    assert(pos == sync_count / 2);

    ticks_short = ticks_short / sync_count; // Calculate average value.

    set_tick_limits(ticks_short);

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
#endif //NDEBUG
    
    // Still LOW at CBM.

    // TODO: Debugging: This does not work to determine, if flip is necessary:
    //
#if 0
    {
        uint32_t pulse_type = 0,
            low_start_tick = 0;

        assert(gpio_read(gpio_pin_nr_write));
        //
        // Must be still LOW at CBM (circuit inverts signal from CBM).

        do
        {
            // Wait for HIGH at CBM:
            //
            if(!wait_for(gpio_pin_nr_write, HIGH, true, is_stop_requested))
            {
                return false;
            }

            // Just got HIGH at CBM. <=> A SAVE pulse has started.

            uint32_t const high_start_tick = s_timer_get_tick();

            // Wait for LOW at CBM:
            //
            if(!wait_for(gpio_pin_nr_write, LOW, true, is_stop_requested))
            {
                return false;
            }

            // Just got LOW at CBM. <=> HIGH half of pulse at CBM finished.

            low_start_tick = s_timer_get_tick();

            uint32_t const high_to_low_ticks = low_start_tick - high_start_tick;

            if(high_to_low_ticks > tick_long_max)
            {
                // console_write("tape_receive_buf: Skipping high pulse of length ");
                // console_write_dword_dec(high_to_low_ticks);
                // console_writeline(" (a pause?)..");
                pulse_type = micro_short; // (just to get to next iteration)
                continue;
            }

            pulse_type = get_pulse_type(high_to_low_ticks);
        }while(pulse_type == micro_short);

        assert(pulse_type == micro_long);
        //
        // Beginning of new-data marker (part 1).

        // Still just got LOW at CBM.

        // Wait for HIGH at CBM:
        //
        if(!wait_for(gpio_pin_nr_write, HIGH, true, is_stop_requested))
        {
            return false;
        }

        // Just got HIGH at CBM.

        uint32_t const high_start_tick = s_timer_get_tick(), 
            inverse_follower_pulse_type = get_pulse_type(
                high_start_tick - low_start_tick);

        flipLevel = inverse_follower_pulse_type != micro_long;

        if(flipLevel)
        {
            console_deb_writeline("tape_receive_buf: Flipping!");

            // Measured the wrong part of the first pulse.

            // Flipping to measure from high->low to low->high changes.

            assert(inverse_follower_pulse_type == micro_medium);
            //
            // Beginning of new-data marker (part 2).

            buf[pos] = get_symbol(pulse_type, inverse_follower_pulse_type);

            assert(buf[pos] == tape_symbol_new);

            ++pos;
        }
        else
        {
            console_deb_writeline("tape_receive_buf: NOT flipping.");

            // Measuring from low->high to high->low changes is correct!

            // Assuming new-data symbol (see assert, above):
            //
            buf[pos] = tape_symbol_new;
            ++pos;

            // Wait for LOW at CBM:
            //
            if(!wait_for(gpio_pin_nr_write, LOW, true, is_stop_requested))
            {
                return false;
            }

            assert(
                get_pulse_type(s_timer_get_tick() - high_start_tick)
                    == micro_medium);
        }
    }
#endif //0
    
    // TODO: Debugging: This still needs to be enabled for supporting PET BASIC v1:
    //
// #ifndef NDEBUG
//     flipLevel = true;
//     // Wait for HIGH at CBM:
//     //
//     if(!wait_for(gpio_pin_nr_write, HIGH, true, is_stop_requested))
//     {
//         return false;
//     }
// #endif //NDEBUG

    while(true)
    {
        bool timeout_reached = false;

        // Still LOW at CBM.

        // Wait for start of (next) SAVE pulse:

        uint32_t const low_start_tick = s_timer_get_tick();
        //
        // (the low level maybe was reached a while ago,
        //  but that does not matter, because it is just used to
        //  stop the reading, when the very long timeout is reached)

        while(true)
        {
            // (circuit inverts signal from CBM)
            //
            if(gpio_read(gpio_pin_nr_write) == flipLevel)
            {
                break; // HIGH at CBM (circuit inverts signal from CBM).
            }

            // Still LOW at CBM.

            uint32_t const cur_tick = s_timer_get_tick();

            if(cur_tick - low_start_tick >= ticks_timeout)
            {
#ifndef NDEBUG
                console_write("tape_receive_buf: Static LOW timeout 0x");
                console_write_dword(ticks_timeout);
                console_write(" reached between low \"start\" tick at 0x");
                console_write_dword(low_start_tick);
                console_write(" and current tick at 0x");
                console_write_dword(cur_tick);
                console_writeline(" while LOW at CBM.");
#endif //NDEBUG
                timeout_reached = true;
                break; // Timeout reached.
            }
        }
        if(timeout_reached)
        {
            break; // DONE (or unhandled error).
        }

        // Just got HIGH at CBM. <=> A SAVE pulse has started.

        uint32_t const high_start_tick = s_timer_get_tick();

        // Wait for LOW at CBM:
        //
        while(true)
        {
            // (circuit inverts signal from CBM)
            //
            if(gpio_read(gpio_pin_nr_write) != flipLevel)
            {
                break; // LOW at CBM (circuit inverts signal from CBM).
            }

            // Still HIGH at CBM.
        }

        // Just got LOW at CBM. <=> HIGH half of pulse at CBM finished.

        pulse_type[pulse_type_index] = get_pulse_type(
            s_timer_get_tick() - high_start_tick);

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

    buf[pos] = tape_symbol_done;
    //++pos;

#ifndef NDEBUG
    console_write("tape_receive_buf: Min. tick count that was interpreted as short pulse: ");
    console_write_dword_dec(tick_short_min);
    console_writeline("");
    console_write("tape_receive_buf: Max. tick count that was interpreted as long pulse: ");
    console_write_dword_dec(tick_long_max);
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
#endif //NDEBUG

    return true;
}

void tape_receive_buf_init(
    void (*timer_start_one_mhz)(), uint32_t (*timer_get_tick)())
{
    assert(s_timer_start_one_mhz == 0);
    assert(s_timer_get_tick == 0);

    s_timer_start_one_mhz = timer_start_one_mhz;
    s_timer_get_tick = timer_get_tick;
}
