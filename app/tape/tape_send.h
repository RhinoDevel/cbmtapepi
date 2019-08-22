
// Marcel Timm, RhinoDevel, 2018jan27

#ifndef MT_TAPE_SEND
#define MT_TAPE_SEND

#include <stdbool.h>

#include "tape_send_params.h"

/**
 * Requirements: - Sense line must be configured as output and set to HIGH.
 *               - Motor line must be configured as input with internal pull-
 *                 down resistor usage.
 *               - Read line must be configured as output and set to HIGH.
 *
 * <=> Call tape_init() before calling this function!
 */
bool tape_send(struct tape_send_params const * const p, uint32_t * const mem);

#endif //MT_TAPE_SEND
