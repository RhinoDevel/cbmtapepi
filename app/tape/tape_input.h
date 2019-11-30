
// Marcel Timm, RhinoDevel, 2018jan27

#ifndef MT_TAPE_INPUT
#define MT_TAPE_INPUT

#include <stdint.h>

#include "tape_filetype.h"

#define MT_TAPE_INPUT_NAME_LEN 16
#define MT_TAPE_INPUT_ADD_BYTES_LEN 171

struct tape_input
{
    uint8_t name[MT_TAPE_INPUT_NAME_LEN]; // PETSCII(!), padded with blanks/$20.
    enum tape_filetype type;
    uint16_t addr;
    uint8_t* bytes;
    uint16_t len;
    uint8_t add_bytes[MT_TAPE_INPUT_ADD_BYTES_LEN]; // Additional bytes (to be stored in header).
};

void tape_input_console_write(struct tape_input const * const d);

/**
 * - Keeps trailing spaces and appends '\0' character.
 * - Caller takes ownership of returned object.
 */
char* tape_input_create_str_from_name(struct tape_input const * const d);

#endif //MT_TAPE_INPUT
