
// Marcel Timm, RhinoDevel, 2018jan26

#include <stdbool.h>
#include <stdint.h>

#include "tape.h"

enum tape_pulse
{
    tape_pulse_short = 1, // 2840 Hz.
    tape_pulse_medium = 2, // 1953 Hz.
    tape_pulse_long = 3 // 1488 Hz.
};

// - Each symbol holds two pulses.
//
enum tape_symbol
{
    tape_symbol_done = 0, // Pseudo-symbol to stop transfer.

    tape_symbol_sync = tape_pulse_short & (tape_pulse_short << 4), // (s,s)

    tape_symbol_zero = tape_pulse_short & (tape_pulse_medium << 4), // (s,m)
    tape_symbol_one = tape_pulse_medium & (tape_pulse_short << 4), // (m,s)
    tape_symbol_new = tape_pulse_long & (tape_pulse_medium << 4), // (l,m)
    tape_symbol_end = tape_pulse_long & (tape_pulse_short << 4), // (l,s)
};

void tape_fill_buf(struct tape_input const * const input, uint8_t * const buf)
{
    (void)input;
    (void)buf;
}

void tape_transfer_buf(uint8_t const * const buf)
{
    (void)buf;
}
