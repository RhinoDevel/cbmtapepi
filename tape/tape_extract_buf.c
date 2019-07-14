
// Marcel Timm, RhinoDevel, 2019jul13

#include <stdbool.h>
#include <stdint.h>

#include "../alloc/alloc.h"
#include "../console/console.h"
#include "../mem/mem.h"
#include "tape_extract_buf.h"
#include "tape_input.h"
#include "tape_symbol.h"

// TODO: Replace!
//
//static int const header_data_byte_count = 192;

static void consume_sync(uint8_t const * const buf, int * const pos)
{
    while(buf[*pos] == tape_symbol_sync)
    {
        ++(*pos);
    }
}

static uint8_t get_byte(uint8_t const * const buf, int * const pos)
{
    // TODO: Implement!
    //
    (void)buf;
    (void)pos;
    return 0;
}

static bool consume_countdown(
    bool const second, uint8_t const * const buf, int * const pos)
{
    uint8_t c = second ? 9 : 0x89,
        lim = second ? 0 : 0x80;

    while(c > lim)
    {
        if(get_byte(buf, pos) != c)
        {
            console_writeline(
                "consume_countdown : Error: Unexpected byte value received!");
            return false;
        }
        --c;
    }
    return true;
}

static uint8_t* get_data_from_transmit(
    bool const second,
    uint8_t const * const buf,
    int * const pos,
    int const * len)
{
    (void)second;
    (void)len; // TODO: Remove!

    consume_countdown(false, buf, pos);

    // Data:

    // TODO: Implement!

    // Transmit block gap:

    // TODO: Implement!
    //
    return 0;
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

static bool extract_headerdatablock(
    uint8_t const * const buf, int * const pos, struct tape_input * const input)
{
    (void)input; // TODO: Remove!

    int len;
    uint8_t* data = get_data_following_sync(buf, pos, &len);

    if(data == 0)
    {
        return false;
    }

    alloc_free(data);

    // TODO: Implement!
    //
    return false;
}

static bool extract_contentdatablock(
    uint8_t const * const buf, int * const pos, struct tape_input * const input)
{
    (void)input; // TODO: Remove!

    int len;
    uint8_t* data = get_data_following_sync(buf, pos, &len);

    if(data == 0)
    {
        return false;
    }

    alloc_free(data);

    // TODO: Implement!
    //
    return false;
}

struct tape_input * tape_extract_buf(uint8_t const * const buf)
{
    struct tape_input * const input = alloc_alloc(sizeof *input);
    int i = 0;

    if(input == 0)
    {
        console_writeline(
            "tape_extract_buf: Error: Failed to allocate input memory!");
        return 0;
    }

    if(!extract_headerdatablock(buf, &i, input))
    {
        alloc_free(input);
        return 0;
    }

    if(!extract_contentdatablock(buf, &i, input))
    {
        alloc_free(input);
        return 0;
    }

    return input;
}
