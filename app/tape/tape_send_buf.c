
// Marcel Timm, RhinoDevel, 2018jan27

#include <stdbool.h>
#include <stdint.h>

#include "tape_send_buf.h"
#include "tape_symbol.h"
#include "../../lib/console/console.h"

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

// Initialized by tape_send_buf_init():
//
static void (*s_timer_busywait_microseconds)(uint32_t const microseconds) = 0;
static void (*s_gpio_write)(uint32_t const pin_nr, bool const high) = 0;
static bool (*s_gpio_read)(uint32_t const pin_nr) = 0;

static void transfer_pulse(uint32_t const micro, uint32_t const gpio_pin_nr)
{
    s_gpio_write(gpio_pin_nr, !false); // (to-be-inverted by circuit)
    s_timer_busywait_microseconds(micro);
    s_gpio_write(gpio_pin_nr, !true); // (to-be-inverted by circuit)
    s_timer_busywait_microseconds(micro);
}

static void transfer_symbol(
    uint32_t const micro_first,
    uint32_t const micro_last,
    uint32_t const gpio_pin_nr)
{
    transfer_pulse(micro_first, gpio_pin_nr);
    transfer_pulse(micro_last, gpio_pin_nr);
}

bool tape_send_buf(
    uint8_t const * const buf,
    uint32_t const gpio_pin_nr_motor,
    uint32_t const gpio_pin_nr_read,
    bool (*is_stop_requested)())
{
    uint32_t i = 0;
    bool motor_wait = true; // Initially wait for motor to be HIGH.

    // As pulse length detection triggers on descending (negative) edges,
    // GPIO pin's current output value is expected to be set to HIGH.

    while(true)
    {
        uint32_t f = 0, l = 0;

        if(is_stop_requested != 0 && is_stop_requested())
        {
            return false;
        }

        if(!s_gpio_read(gpio_pin_nr_motor))
        {
            if(motor_wait)
            {
                console_deb_writeline("tape_send_buf: Motor is OFF, waiting..");

                while(!s_gpio_read(gpio_pin_nr_motor))
                {
                    // Pause, as long as motor signal from Commodore computer is
                    // LOW.

                    if(is_stop_requested != 0 && is_stop_requested())
                    {
                        return false;
                    }
                }

                console_deb_writeline("tape_send_buf: Motor is ON, resuming..");
            }
            else
            {
                console_deb_writeline("tape_send_buf: Motor is OFF, done.");
                return true; // Done
            }
        }

        if(is_stop_requested != 0 && is_stop_requested())
        {
            return false;
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

            case tape_symbol_motor_wait_off: // (fake symbol)
            {
                console_deb_writeline("tape_send_buf: Disabling motor-wait..");
                motor_wait = false;
                break;
            }
            case tape_symbol_done:
            {
                console_deb_writeline(
                    "tape_send_buf: Done symbol found. Done.");
                return true; // Transfer done (fake symbol).
            }

            default: // Must not happen.
            {
                console_deb_writeline("tape_send_buf: Error: Unknown symbol!");
                return false; // Error!
            }
        }
        if(f != 0) // => No fake symbol.
        {
            transfer_symbol(f, l, gpio_pin_nr_read);
        }

        ++i;
    }
}

void tape_send_buf_init(
    void (*timer_busywait_microseconds)(uint32_t const microseconds),
    void (*gpio_write)(uint32_t const pin_nr, bool const high),
    bool (*gpio_read)(uint32_t const pin_nr))
{
    // assert(s_timer_busywait_microseconds == 0);
    // assert(s_gpio_write == 0);
    // assert(s_gpio_read == 0);

    s_timer_busywait_microseconds = timer_busywait_microseconds;
    s_gpio_write = gpio_write;
    s_gpio_read = gpio_read;
}
