
// Marcel Timm, RhinoDevel, 2019sep12

#include "cbm_send.h"

#include "../config.h"
#include "../../lib/alloc/alloc.h"
#include "../../lib/str/str.h"
#include "../tape/tape_send_params.h"
#include "../tape/tape_filetype.h"
#include "../tape/tape_send.h"
#include "../tape/tape_input.h"

#ifndef NDEBUG
    #include "../../lib/console/console.h"
#endif //NDEBUG

#include <stdint.h>
#include <stdbool.h>

static void fill_name(uint8_t * const name_out, char const * const name_in)
{
    // static uint8_t const sample_name[] = {
    //     'R', 'H', 'I', 'N', 'O', 'D', 'E', 'V', 'E', 'L',
    //     0x20, 0x20, 0x20, 0x20, 0x20, 0x20
    // };

    int i = 0;
    char * const buf = alloc_alloc(str_get_len(name_in) + 1);

    str_to_upper(buf, name_in);

    while(i < MT_TAPE_INPUT_NAME_LEN && buf[i] != '\0')
    {
        name_out[i] = (uint8_t)buf[i]; // TODO: Implement real conversion to PETSCII.
        ++i;
    }
    while(i < MT_TAPE_INPUT_NAME_LEN)
    {
        name_out[i] = 0x20;
        ++i;
    }

    alloc_free(buf);
}

static void fill_add_bytes(uint8_t * const add_bytes)
{
    // Additional bytes (to be stored in header):
    //
    for(int i = 0;i<MT_TAPE_INPUT_ADD_BYTES_LEN;++i)
    {
        add_bytes[i] = 0x20;
    }
}

bool cbm_send(
    uint8_t /*const*/ * const bytes,
    char const * const name,
    uint32_t const count,
    bool (*is_stop_requested)())
{
    bool ret_val = false;
    struct tape_send_params p;
    uint32_t * const mem_addr = alloc_alloc(4 * 1024 * 1024); // Hard-coded

    p.is_stop_requested = is_stop_requested;
    p.gpio_pin_nr_read = MT_TAPE_GPIO_PIN_NR_READ;
    p.gpio_pin_nr_sense = MT_TAPE_GPIO_PIN_NR_SENSE;
    p.gpio_pin_nr_motor = MT_TAPE_GPIO_PIN_NR_MOTOR;
    p.data = alloc_alloc(sizeof *(p.data));

    fill_name(p.data->name, name);

    // BUG: TODO: Does NOT seem to work for (all) machine language files,
    //       at least not on the C64! Fix:
    //
    p.data->type = tape_filetype_relocatable; // (necessary for PET PRG file)
    //
    // Hard-coded - maybe not always correct, but works for C64 and PET,
    // both with machine language and BASIC PRG files.

    // First two bytes hold the start address:
    //
    p.data->addr = (((uint16_t)bytes[1]) << 8) | (uint16_t)bytes[0];
    p.data->bytes = bytes + 2;
    p.data->len = count - 2;

    fill_add_bytes(p.data->add_bytes);

    ret_val = tape_send(&p, mem_addr);

    alloc_free(mem_addr);
    alloc_free(p.data);
    return ret_val;
}
