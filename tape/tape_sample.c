
// Marcel Timm, RhinoDevel, 2018feb03

#include "tape_input.h"
#include "tape_filetype.h"
#include "tape_sample.h"

// How to get byte array from (PRG) file:
//
// xxd -i 'pet graphics.prg' > pet_graphics.c

void tape_sample_fill_buf(struct tape_input * const buf)
{
    static uint8_t const bytes[] = {
        169, // Immediate LDA.
        83, // Heart symbol (yes, it is romantic).
        141, // Absolute STA.

        0, // Lower byte of 1024 (0x0400 - C64 video RAM start).
        4, // Higher byte of 1024.
        //
        //0, // Lower byte of 32768 (0x8000 - PET video RAM start).
        //128, // Higher byte of 32768.

        96 // RTS.
    };

    {
        uint8_t const name[] = {
            'A', 'B', 'C', 'D', 'E', 'F',
            '1', '2', '3', '4', '5', '6',
            0x20, 0x20, 0x20, 0x20
        };

        for(int i = 0;i<MT_TAPE_INPUT_NAME_LEN;++i)
        {
            buf->name[i] = name[i];
        }
    }

    buf->type = tape_filetype_non_relocatable; // C64
    //buf->type = tape_filetype_relocatable; // PET (for machine language, too).

    buf->addr = 2024; // C64: 16 unused bytes.
    //buf->addr = 0x0401;//826; // PET ROM v2 and v3 tape #2 buffer.

    buf->bytes = bytes;
    buf->len = sizeof bytes;

    // Additional bytes (to be stored in header):
    //
    for(int i = 0;i<MT_TAPE_INPUT_ADD_BYTES_LEN;++i)
    {
        buf->add_bytes[i] = 0x20;
    }
}
