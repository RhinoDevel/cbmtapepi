
// Marcel Timm, RhinoDevel, 2019jul13

#ifndef MT_TAPE_EXTRACT_BUF
#define MT_TAPE_EXTRACT_BUF

#include <stdint.h>

#include "tape_input.h"

/** Convert Commodore datassette/datasette symbols in given buffer into
 *  binary tape content (e.g. a PRG file).
 *
 *  - input->bytes must point to memory position with enough
 *    space to store largest tape data (e.g. a PRG file). 
 */
bool tape_extract_buf(
    uint8_t const * const buf, struct tape_input * const input);

#endif //MT_TAPE_EXTRACT_BUF
