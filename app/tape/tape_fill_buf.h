
// Marcel Timm, RhinoDevel, 2018jan27

#ifndef MT_TAPE_FILL_BUF
#define MT_TAPE_FILL_BUF

#include <stdint.h>

#include "tape_input.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Convert given input into Commodore datassette/datasette symbols
 *  and add these symbols to the buffer given.
 *
 *  Returns count of symbols (bytes) added to given buffer.
 */
int tape_fill_buf(struct tape_input const * const input, uint8_t * const buf);

#ifdef __cplusplus
}
#endif

#endif //MT_TAPE_FILL_BUF
