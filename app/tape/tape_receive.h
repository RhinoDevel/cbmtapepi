
// Marcel Timm, RhinoDevel, 2019jul11

#ifndef MT_TAPE_RECEIVE
#define MT_TAPE_RECEIVE

#include <stdbool.h>

#include "tape_receive_params.h"
#include "tape_input.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Requirements: - Sense line must be configured as output and set to HIGH.
 *               - Motor line must be configured as input with internal pull-
 *                 down resistor usage.
 *               - Write line must be configured as input with internal pull-
 *                 down resistor usage.
 *
 * - Caller takes ownership of return value.
 * - Returns NULL on error.
 *
 * <=> Call tape_init() before calling this function!
 */
struct tape_input * tape_receive(struct tape_receive_params const * const p);

#ifdef __cplusplus
}
#endif

#endif //MT_TAPE_RECEIVE
