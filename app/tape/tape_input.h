
// Marcel Timm, RhinoDevel, 2018jan27

#ifndef MT_TAPE_INPUT
#define MT_TAPE_INPUT

#include <stdint.h>
#include <stdbool.h>

#include "tape_filetype.h"

// First five bytes of the 192 bytes long tape buffer are filled with:
//
// 1) 1 byte = Tape filetype (probably).
// 2) 2 byte = PRG start address.
// 3) 2 byte = Address after last byte of PRG [(3) - (2) = PRG byte count].
//
#define MT_TAPE_INPUT_NAME_LEN 16 // (4)
#define MT_TAPE_INPUT_ADD_BYTES_LEN 171 // (5)

#ifdef __cplusplus
extern "C" {
#endif

struct tape_input
{
    uint8_t name[MT_TAPE_INPUT_NAME_LEN]; // PETSCII(!), padded with blanks/$20.
    enum tape_filetype type;
    uint16_t addr;
    uint8_t* bytes;
    uint16_t len;
    uint8_t add_bytes[MT_TAPE_INPUT_ADD_BYTES_LEN]; // Additional bytes (to be stored in header).
};

/**
 * - Also frees d->bytes.
 * - Given pointer will be invalid on return.
 */
void tape_input_free(struct tape_input * const d);

void tape_input_console_write(
    struct tape_input const * const d, bool const write_bytes);

/**
 * - Removes trailing spaces.
 * - Appends '\0' character.
 * - Caller takes ownership of returned object.
 */
char* tape_input_create_str_from_name(struct tape_input const * const d);
char* tape_input_create_str_from_name_only(uint8_t const * const name);

void tape_input_fill_add_bytes(uint8_t * const add_bytes);

#ifdef __cplusplus
}
#endif

#endif //MT_TAPE_INPUT
