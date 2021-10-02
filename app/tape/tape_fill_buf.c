
// Marcel Timm, RhinoDevel, 2018jan27

#include <stdbool.h>
#include <stdint.h>

#include "tape_fill_buf.h"
#include "tape_input.h"
#include "tape_symbol.h"

static int const sync_pulse_count = 1500;
static int const transmit_block_gap_pulse_count = 60;
static int const header_data_byte_count = 192;

static void add_symbol(
    enum tape_symbol const sym, uint8_t * const buf, int * const pos)
{
    buf[*pos] = sym;
    ++(*pos);
}

static void add_byte(uint8_t const byte, uint8_t * const buf, int * const pos)
{
    uint8_t parity_bit = 1;

    // New-data marker:

    add_symbol(tape_symbol_new, buf, pos);

    // Payload bits:

    for(int i = 0;i<8;++i)
    {
        uint8_t const bit = (byte >> i) & 1;

        add_symbol((enum tape_symbol)bit, buf, pos);

        parity_bit ^= bit;
    }

    // Parity bit:

    add_symbol((enum tape_symbol)parity_bit, buf, pos);
}

static void add_countdown(
    bool const second, uint8_t * const buf, int * const pos)
{
    uint8_t c = second ? 9 : 0x89,
        lim = second ? 0 : 0x80;

    while(c > lim)
    {
        add_byte(c, buf, pos);
        --c;
    }
}

static void add_transmitblockgap(uint8_t * const buf, int * const pos)
{
    add_symbol(tape_symbol_end, buf, pos);

    // Divide by 2, because a symbol has two pulses:
    //
    for(int c = 0;c < transmit_block_gap_pulse_count/2;++c)
    {
        add_symbol(tape_symbol_sync, buf, pos);
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
        add_symbol(tape_symbol_sync, buf, pos);
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
        end_addr_plus_one = input->addr + input->len,
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

    for(c = 0;c < MT_TAPE_INPUT_NAME_LEN;++c)
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

int tape_fill_buf(struct tape_input const * const input, uint8_t * const buf)
{
    int ret_val = 0;

    add_headerdatablock(input, buf, &ret_val);

    add_symbol(tape_symbol_motor_wait_off, buf, &ret_val);

    add_contentdatablock(input, buf, &ret_val);

    add_symbol(tape_symbol_done, buf, &ret_val);

    return ret_val;
}
