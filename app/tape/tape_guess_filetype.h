
// Marcel Timm, RhinoDevel, 2019oct06

#ifndef MT_TAPE_GUESS_FILETYPE
#define MT_TAPE_GUESS_FILETYPE

#include "tape_filetype.h"

/** Guess filetype based on given binary data of PRG file.
 */
enum tape_filetype tape_guess_filetype(
    uint8_t const * const bytes, uint16_t const len);

#endif //MT_TAPE_GUESS_FILETYPE
