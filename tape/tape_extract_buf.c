
// Marcel Timm, RhinoDevel, 2019jul13

#include <stdbool.h>
#include <stdint.h>

#include "../alloc/alloc.h"
#include "../console/console.h"
#include "../mem/mem.h"
#include "../lib/assert.h"
#include "tape_extract_buf.h"
#include "tape_input.h"
#include "tape_symbol.h"
#include "tape_filetype.h"

static void consume_sync(uint8_t const * const buf, int * const pos)
{
    while(buf[*pos] == tape_symbol_sync)
    {
        ++(*pos);
    }
}

static bool extract_byte(
    uint8_t const * const buf, int * const pos, uint8_t * const byte)
{
    uint8_t byte_buf = 0,
        parity = 1;

    // New data marker:

    if(buf[*pos] != tape_symbol_new)
    {
#ifndef NDEBUG
        console_write("extract_byte : Error: Symbol new not found, but ");
        console_write_byte_dec(buf[*pos]);
        console_writeline(" instead!");
#endif //NDEBUG
        return false;
    }
    ++(*pos);

    // Payload bits:

    for(int i = 0;i < 8;++i)
    {
        if(buf[*pos] == tape_symbol_zero)
        {
            parity ^= 0;

            ++(*pos);
            continue;
        }
        if(buf[*pos] == tape_symbol_one)
        {
            byte_buf |= (1 << i);
            parity ^= 1;

            ++(*pos);
            continue;
        }

        console_deb_writeline("extract_byte : Error: Unsupported symbol found!");
        return false;
    }

    // Parity bit:
    //
    switch(buf[*pos])
    {
        case tape_symbol_zero:
        {
            if(parity != 0)
            {
                console_deb_writeline(
                    "extract_byte : Error: Parity zero read, but not counted!");
                return false;
            }
            break;
        }
        case tape_symbol_one:
        {
            if(parity != 1)
            {
                console_deb_writeline(
                    "extract_byte : Error: Parity one read, but not counted!");
                return false;
            }
            break;
        }

        default:
        {
            console_deb_writeline(
                "extract_byte : Error: Unexpected parity symbol read!");
            return false; // Unexpected symbol.
        }
    }
    ++(*pos);

    *byte = byte_buf;
    return true;
}

static bool consume_countdown(
    bool const second, uint8_t const * const buf, int * const pos)
{
    uint8_t c = second ? 9 : 0x89, // TODO: Replace!
        lim = second ? 0 : 0x80; // TODO: Replace!

    while(c > lim)
    {
        uint8_t byte;

        if(!extract_byte(buf, pos, &byte))
        {
            console_deb_writeline(
                "consume_countdown : Error: Failed to extract byte value!");
            return false;
        }

        if(byte != c)
        {
            console_deb_writeline(
                "consume_countdown : Error: Unexpected byte value received!");
            return false;
        }
        --c;
    }
    return true;
}

/**
 * - Caller takes ownership of return value.
 */
static uint8_t* get_payload_from_transmit_data(
    uint8_t const * const buf, int * const pos, int * const len)
{
    static int const byte_symbol_count = 10;
    //
    // Hard-coded, see extract_byte().

    int offset_end = 0,
        payload_byte_count;
    uint8_t *payload,
        byte,
        checksum = 0;

    while(true)
    {
        uint8_t const cur = buf[*pos + offset_end];

        if(cur == tape_symbol_done)
        {
            return 0; // Expected symbol could not be found.
        }

        if(cur == tape_symbol_end)
        {
            // Found optional end-of-data marker or transmit block gap start.

            break;
        }

        ++offset_end;
    }

    if(offset_end % byte_symbol_count != 0)
    {
        // Must not happen!

        console_deb_writeline(
            "get_payload_from_transmit_data : Error: Offset end must be multiple of symbol per byte count!");
        return 0;
    }

    // offset_end == symbol count for payload bytes plus parity byte:
    //
    payload_byte_count = offset_end / byte_symbol_count - 1;

    // Payload:

    payload = alloc_alloc(payload_byte_count * sizeof *payload);
    if(payload == 0)
    {
        return 0;
    }

    for(int i = 0;i < payload_byte_count;++i)
    {
        if(!extract_byte(buf, pos, &byte))
        {
            alloc_free(payload);
            return 0;
        }
        payload[i] = byte;
        checksum ^= byte;
    }

    // Checksum:

    if(!extract_byte(buf, pos, &byte))
    {
        alloc_free(payload);
        return 0;
    }
    if(byte != checksum)
    {
        alloc_free(payload);
        return 0;
    }

    // Optional end-of-data marker:

    if(buf[*pos] != tape_symbol_end)
    {
        // Must not happen:

        console_deb_writeline(
            "get_payload_from_transmit_data : Error: Expected symbol end!");
        alloc_free(payload);
        return 0;
    }

    if(buf[*pos + 1] == tape_symbol_end)
    {
        // This is the optional end-of-data marker:

        ++(*pos); // Consume optional end-of-data marker.
    }
    //
    // Otherwise: This is the start of transmit block gap.

    *len = payload_byte_count;
    return payload;
}

static bool consume_transmit_block_gap(
    uint8_t const * const buf, int * const pos)
{
    if(buf[*pos] != tape_symbol_end)
    {
        return false;
    }

    ++(*pos); // Consumes transmit block gap start.

    consume_sync(buf, pos);

    return true;
}

/**
 * - Caller takes ownership of return value.
 */
static uint8_t* get_data_from_transmit(
    bool const second,
    uint8_t const * const buf,
    int * const pos,
    int * const len)
{
    uint8_t* payload;

    // First countdown sequence:

    if(!consume_countdown(second, buf, pos))
    {
        return 0;
    }

    // Data:

    payload = get_payload_from_transmit_data(buf, pos, len);
    if(payload == 0)
    {
        return 0;
    }

    // Transmit block gap:

    if(!consume_transmit_block_gap(buf, pos))
    {
        alloc_free(payload);
        return 0;
    }

    return payload;
}

/**
 * - Consumes leading sync symbols.
 * - Caller takes ownership of return value.
 */
static uint8_t* get_data_following_sync(
    uint8_t const * const buf, int * const pos, int * const len)
{
    uint8_t *data_first,
        *data_second;
    int len_first,
        len_second;

    // Synchronization:

    consume_sync(buf, pos);

    // Data:

    // 1st data transmit:
    //
    data_first = get_data_from_transmit(false, buf, pos, &len_first);
    if(data_first == 0)
    {
        return 0;
    }

    // 2nd data transmit:
    //
    data_second = get_data_from_transmit(true, buf, pos, &len_second);
    if(data_second == 0)
    {
        alloc_free(data_first);
        return 0;
    }

    if(len_first != len_second)
    {
        alloc_free(data_first);
        alloc_free(data_second);
        return 0;
    }
    if(!mem_cmp_byte(data_first, data_second, len_first))
    {
        alloc_free(data_first);
        alloc_free(data_second);
        return 0;
    }

    alloc_free(data_second);
    *len = len_first;
    return data_first;
}

/**
 * - Also sets input->len.
 * - Properties of input may also be changed, if returning with error.
 */
static bool extract_headerdatablock(
    uint8_t const * const buf, int * const pos, struct tape_input * const input)
{
    static int const header_data_byte_count = 192, // TODO: Replace!
        name_len = (int)(sizeof input->name / sizeof *input->name),
        add_bytes_len =
            (int)(sizeof input->add_bytes / sizeof *input->add_bytes);

    int len, // This is the HEADER data length.
        i = 0;
    uint8_t* data;
    uint16_t end_addr_plus_one;

    data = get_data_following_sync(buf, pos, &len);
    if(data == 0)
    {
        return false;
    }
    if(len != header_data_byte_count)
    {
        alloc_free(data);
        return false;
    }

    // File type:

    input->type = (enum tape_filetype)data[0];
    if(input->type != tape_filetype_relocatable
        && input->type != tape_filetype_non_relocatable)
    {
        console_deb_writeline(
            "extract_headerdatablock: Error: Unsupported file type!");

        alloc_free(data);
        return false;
    }

    // Destination infos:

    input->addr = ((uint16_t)data[2] << 8) | (uint16_t)data[1];
    end_addr_plus_one = ((uint16_t)data[4] << 8) | (uint16_t)data[3];
    input->len = end_addr_plus_one - input->addr;

    // File name:

    while(i < name_len)
    {
        input->name[i] = data[5 + i];

        ++i;
    }

    // Additional bytes:

    for(i = 0;i < add_bytes_len;++i)
    {
        input->add_bytes[i] = data[5 + name_len + i];
    }

    return true;
}

/**
 * - Caller takes ownership of filled input->bytes.
 * - input->len must already be set. Will be checked, here.
 */
static bool extract_contentdatablock(
    uint8_t const * const buf, int * const pos, struct tape_input * const input)
{
    int len;
    uint8_t * const data = get_data_following_sync(buf, pos, &len);
    if(data == 0)
    {
        return false;
    }

    if(input->len != (uint16_t)len)
    {
        console_deb_writeline(
            "extract_contentdatablock: Error: Lengths mismatch!");

        alloc_free(data);
        return false;
    }

    input->bytes = data;

    return true;
}

struct tape_input * tape_extract_buf(uint8_t const * const buf)
{
    struct tape_input * const input = alloc_alloc(sizeof *input);
    int i = 0;

    if(input == 0)
    {
        console_deb_writeline(
            "tape_extract_buf: Error: Failed to allocate input memory!");
        return 0;
    }

    if(!extract_headerdatablock(buf, &i, input))
    {
        alloc_free(input);
        return 0;
    }

    // For compatibility with tape_fill_buf():
    //
    if(buf[i] == tape_symbol_motor_wait_off)
    {
        ++i;
    }

    if(!extract_contentdatablock(buf, &i, input))
    {
        alloc_free(input);
        return 0;
    }

    assert(buf[i] == tape_symbol_done);

    return input;
}
