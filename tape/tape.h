
// Marcel Timm, RhinoDevel, 2018jan26

#ifndef MT_TAPE
#define MT_TAPE

#include <stdbool.h>
#include <stdint.h>

enum tape_filetype
{
    tape_filetype_relocatable = 1, // == BASIC prg.
    //tape_filetype_seq_data_block = 2,
    tape_filetype_non_relocatable = 3, // == Machine language prg.
    //tape_filetype_seq_header = 4,
    //tape_filetype_end_of_tape_marker = 5
};

struct tape_input
{
    uint8_t const name[16]; // PETSCII(!), padded with blanks/$20.
    enum tape_filetype type;
    uint16_t addr;
    uint8_t const * bytes;
    uint16_t len;
    uint8_t const add_bytes[171]; // Additional bytes (to be stored in header).
};

void tape_fill_buf(struct tape_input const * const input, uint8_t * const buf);

/**
 * - Does not care about sense line (must already be set to correct value).
 * - GPIO pin must already be configured as output and set to HIGH.
 */
bool tape_transfer_buf(uint8_t const * const buf, uint32_t const gpio_pin_nr);

#endif //MT_TAPE
