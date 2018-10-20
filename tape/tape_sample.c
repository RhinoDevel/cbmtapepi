
// Marcel Timm, RhinoDevel, 2018feb03

#include "tape_input.h"
#include "tape_filetype.h"
#include "tape_sample.h"

tape_sample_fill_buf(struct tape_input * const buf)
{
    *buf = (struct tape_input){
        .name = {
            'A', 'B', 'C', 'D', 'E', 'F',
            '1', '2', '3', '4', '5', '6',
            0x20, 0x20, 0x20, 0x20
        },
        type = tape_filetype_non_relocatable,
        addr = 826, // ROM v2 and v3 tape #2 buffer.
        bytes = {
            169, // Immediate LDA.
            83, // Heart symbol (yes, it is romantic).
            141, // Absolute STA.
            0, // Lower byte of 32768 (0x8000 - video RAM start).
            128, // Higher byte of 32768.
            96 // RTS.
        }
    };
    // uint16_t len;
    // uint8_t add_bytes[171]; // Additional bytes (to be stored in header).
}
