
// Marcel Timm, RhinoDevel, 2019jul13

#ifndef MT_TAPE_EXTRACT_BUF
#define MT_TAPE_EXTRACT_BUF

#include <stdint.h>

#include "tape_input.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Convert Commodore datassette/datasette symbols in given buffer into
 *  binary tape content (e.g. a PRG file).
 *
 *  - Caller takes ownership of return value.
 */
struct tape_input * tape_extract_buf(uint8_t const * const buf);

#ifdef __cplusplus
}
#endif

#endif //MT_TAPE_EXTRACT_BUF
