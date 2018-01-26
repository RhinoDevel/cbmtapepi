
// Marcel Timm, RhinoDevel, 2018jan26

#ifndef MT_TAPE
#define MT_TAPE

#include <stdbool.h>
#include <stdint.h>

enum tape_filetype
{
    tape_filetype_relocatable = 1,
    //tape_filetype_seq_data_block = 2,
    tape_filetype_non_relocatable = 3,
    //tape_filetype_seq_header = 4,
    //tape_filetype_end_of_tape_marker = 5
};

struct tape_input
{
    char const * name;
    enum tape_filetype type;
    uint16_t addr;
    uint8_t const * bytes;
    uint32_t len;
};

void tape_fill_buf(struct tape_input const * const input, uint8_t * const buf);
void tape_transfer_buf(uint8_t const * const buf);

#endif //MT_TAPE
