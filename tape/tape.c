
// Marcel Timm, RhinoDevel, 2018jan26

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "tape.h"
#include "../busywait/busywait.h"
#include "../baregpio/baregpio.h"

// - Each symbol holds two pulses.
//
enum symbol
{
    symbol_zero = 0,
    symbol_one = 1,
    symbol_sync = 2, // Represents TWO sync. pulses!
    symbol_new = 4,
    symbol_end = 8, // (also for transmit block gap start)

    symbol_done = 0xFF // Pseudo-symbol to stop transfer.
};

static int const sync_pulse_count = 1500;
static int const transmit_block_gap_pulse_count = 60;

static int const header_data_byte_count = 192;

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

static void add_symbol(
    enum symbol const sym, uint8_t * const buf, int * const pos)
{
    buf[*pos] = sym;
    ++(*pos);
}

static void add_byte(uint8_t const byte, uint8_t * const buf, int * const pos)
{
    uint8_t parity_bit = 1;

    // New-data marker:

    add_symbol(symbol_new, buf, pos);

    // Payload bits:

    for(int i = 0;i<8;++i)
    {
        uint8_t const bit = (byte >> i) & 1;

        add_symbol((enum symbol)bit, buf, pos);

        parity_bit ^= bit;
    }

    // Parity bit:

    add_symbol((enum symbol)parity_bit, buf, pos);
}

static void add_countdown(
    bool const second, uint8_t * const buf, int * const pos)
{
    uint8_t c = second ? 9 : 0x89;

    while(c > 0)
    {
        add_byte(c, buf, pos);
        --c;
    }
}

static void add_transmitblockgap(uint8_t * const buf, int * const pos)
{
    add_symbol(symbol_end, buf, pos);

    // Divide by 2, because a symbol has two pulses:
    //
    for(int c = 0;c < transmit_block_gap_pulse_count/2;++c)
    {
        add_symbol(symbol_sync, buf, pos);
    }
}

static void add_data(
    uint8_t const * const data,
    uint32_t const len,
    uint8_t * const buf,
    int * const pos)
{
    uint8_t parity_byte = 0;

    // Payload:

    for(uint32_t i = 0;i < len;++i)
    {
        add_byte(data[i], buf, pos);

        parity_byte ^= data[i];
    }

    // Checksum:

    add_byte(parity_byte, buf, pos);

    // (optional end-of-data marker is omitted)
}

static void add_data_transmit(
    bool const second,
    uint8_t const * const data,
    uint32_t const len,
    uint8_t * const buf,
    int * const pos)
{
    // Countdown sequence:

    add_countdown(second, buf, pos);

    // Data:

    add_data(data, len, buf, pos);

    // Transmit block gap:

    add_transmitblockgap(buf, pos);
}

static void add_sync(uint8_t * const buf, int * const pos)
{
    // Divide by 2, because a symbol has two pulses:
    //
    for(int c = 0;c < sync_pulse_count/2;++c)
    {
        add_symbol(symbol_sync, buf, pos);
    }
}

static void add_data_following_sync(
    uint8_t const * const data,
    uint32_t const len,
    uint8_t * const buf,
    int * const pos)
{
    // 1st data transmit:

    add_data_transmit(false, data, len, buf, pos);

    // 2nd data transmit:

    add_data_transmit(true, data, len, buf, pos);
}

static void add_headerdatablock(
    struct tape_input const * const input, uint8_t * const buf, int * const pos)
{
    int i = 0, c = 0;
    uint16_t const addr_high = input->addr / 256,
        end_addr_plus_one = input->addr + input->len + 1,
        end_addr_plus_one_high = end_addr_plus_one / 256;
    uint8_t header_data[header_data_byte_count];

    // Synchronization:

    add_sync(buf, pos);

    // Header data:

    // - File type:

    header_data[i] = input->type;
    ++i;

    // - Start address:

    header_data[i] = input->addr - 256 * addr_high; // Low part of start addr.
    ++i;
    header_data[i] = addr_high; // High part of start address.
    ++i;

    // - End address +1:

    header_data[i] = end_addr_plus_one - 256 * end_addr_plus_one_high;
    ++i;
    header_data[i] = end_addr_plus_one_high;
    ++i;

    // - File name:

    for(c = 0;c < 16;++c) // Hard-coded
    {
        header_data[i] = input->name[c];
        ++i;
    }

    // - Additional bytes:

    for(c = 0;c < 171;++c) // Hard-coded
    {
        header_data[i] = input->add_bytes[c];
        ++i;
    }

    add_data_following_sync(header_data, header_data_byte_count, buf, pos);
}

static void add_contentdatablock(
    struct tape_input const * const input, uint8_t * const buf, int * const pos)
{
    // Synchronization:

    add_sync(buf, pos);

    // Content data:

    add_data_following_sync(input->bytes, input->len, buf, pos);
}

void tape_fill_buf(struct tape_input const * const input, uint8_t * const buf)
{
    int i = 0;

    add_headerdatablock(input, buf, &i);
    add_contentdatablock(input, buf, &i);
}

bool tape_transfer_buf(uint8_t const * const buf, uint32_t const gpio_pin_nr)
{
    int i = 0;

    // As pulse length detection triggers on descending (negative) edges,
    // GPIO pin's current output value is expected to be set to HIGH.

    while(true)
    {
        uint32_t f = 0, l = 0;

        switch(buf[i])
        {
            case symbol_zero:
            {
                f = micro_short;
                l = micro_medium;
                break;
            }
            case symbol_one:
            {
                f = micro_medium;
                l = micro_short;
                break;
            }
            case symbol_sync:
            {
                f = micro_short;
                l = micro_short;
                break;
            }
            case symbol_new:
            {
                f = micro_long;
                l = micro_medium;
                break;
            }
            case symbol_end: // Used for transmit block gap start, only.
            {
                f = micro_long;
                l = micro_short;
                break;
            }

            case symbol_done:
            {
                return true; // Transfer done.
            }

            default: // Must not happen.
            {
                return false; // Error!
            }
        }
        ++i;
        transfer_symbol(f, l, gpio_pin_nr);
    }
}
