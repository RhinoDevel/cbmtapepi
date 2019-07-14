
// Marcel Timm, RhinoDevel, 2019jul13

#include <stdbool.h>
#include <stdint.h>

#include "../alloc/alloc.h"
#include "../console/console.h"
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

/**
 * - Caller takes ownership of return value.
 */
static uint8_t* extract_data_following_sync(
    uint8_t const * const buf, int * const pos, int const * len)
{
    // TODO: Implement!
    //
    (void)buf;
    (void)pos;
    (void)len;
    return 0;
    //
    // // 1st data transmit:
    //
    // add_data_transmit(false, data, len, buf, pos);
    //
    // // 2nd data transmit:
    //
    // add_data_transmit(true, data, len, buf, pos);
}

static bool extract_headerdatablock(
    uint8_t const * const buf, int * const pos, struct tape_input * const input)
{
    (void)input; // TODO: Remove!

    uint8_t* data;
    int len;

    // Synchronization:

    consume_sync(buf, pos);

    // Payload:

    data = extract_data_following_sync(buf, pos, &len);
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

    // Synchronization:

    consume_sync(buf, pos);

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
