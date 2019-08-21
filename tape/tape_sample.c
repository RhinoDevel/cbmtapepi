
// Marcel Timm, RhinoDevel, 2018feb03

#include <stdbool.h>

#include "tape_input.h"
#include "tape_send_params.h"
#include "tape_filetype.h"
#include "tape_sample.h"
#include "tape_sample_type.h"
#include "tape_send.h"
#include "../lib/alloc/alloc.h"
#include "../config.h"

// How to get byte array from (PRG) file:
//
// xxd -i 'pet graphics.prg' > pet_graphics.c

static void fill_name(uint8_t * const name)
{
    static uint8_t const sample_name[] = {
        'A', 'B', 'C', 'D', 'E', 'F',
        '1', '2', '3', '4', '5', '6',
        0x20, 0x20, 0x20, 0x20
    };

    for(int i = 0;i<MT_TAPE_INPUT_NAME_LEN;++i)
    {
        name[i] = sample_name[i];
    }
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

static void pet_fill_buf(struct tape_input * const buf)
{
    static uint8_t /*const*/ bytes[] = {
        169, // Immediate LDA.
        83, // Heart symbol (yes, it is romantic).
        141, // Absolute STA.
        0, // Lower byte of 32768 (0x8000 - PET video RAM start).
        128, // Higher byte of 32768.
        96 // RTS.
    };

    fill_name(buf->name);
    buf->type = tape_filetype_relocatable; // PET (for machine language, too).
    buf->addr = 826; // PET ROM v2 and v3 tape #2 buffer.
    buf->bytes = bytes;
    buf->len = sizeof bytes;
    fill_add_bytes(buf->add_bytes);
}

static void c64_fill_buf(struct tape_input * const buf)
{
    static uint8_t /*const*/ bytes[] = {
        169, // Immediate LDA.
        83, // Heart symbol (yes, it is romantic).
        141, // Absolute STA.
        0, // Lower byte of 1024 (0x0400 - C64 video RAM start).
        4, // Higher byte of 1024.
        96 // RTS.
    };

    fill_name(buf->name);
    buf->type = tape_filetype_non_relocatable; // C64
    buf->addr = 2024; // C64: 16 unused bytes.
    buf->bytes = bytes;
    buf->len = sizeof bytes;
    fill_add_bytes(buf->add_bytes);
}

bool tape_sample_send(enum tape_sample_type const t)
{
    bool ret_val = true; // TRUE by default!
    struct tape_send_params p;
    uint32_t * const mem_addr = alloc_alloc(4 * 1024 * 1024); // Hard-coded

    p.is_stop_requested = 0;
    p.gpio_pin_nr_read = MT_TAPE_GPIO_PIN_NR_READ;
    p.gpio_pin_nr_sense = MT_TAPE_GPIO_PIN_NR_SENSE;
    p.gpio_pin_nr_motor = MT_TAPE_GPIO_PIN_NR_MOTOR;
    p.data = alloc_alloc(sizeof *(p.data));

    switch(t)
    {
        case tape_sample_type_pet:
            pet_fill_buf(p.data);
            break;
        case tape_sample_type_c64:
            c64_fill_buf(p.data);
            break;

        default:
            ret_val = false;
            break;
    }

    if(ret_val)
    {
        ret_val = tape_send(&p, mem_addr);
    }

    alloc_free(mem_addr);
    alloc_free(p.data);
    return ret_val;
}
