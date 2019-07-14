
// Marcel Timm, RhinoDevel, 2019jul11

#ifndef MT_TAPE_RECEIVE
#define MT_TAPE_RECEIVE

#include <stdbool.h>

#include "tape_receive_params.h"

/**
 * Requirements: - Sense line must be configured as output and set to HIGH.
 *               - Motor line must be configured as input with internal pull-
 *                 down resistor usage.
 *               - Write line must be configured as input with internal pull-
 *                 down resistor usage.
 *
 *               - p->data->bytes must point to memory position with enough
 *                 space to store largest tape data (e.g. a PRG file).            
 *
 * <=> Call tape_init() before calling this function!
 */
bool tape_receive(
    struct tape_receive_params const * const p, uint32_t * const mem);

#endif //MT_TAPE_RECEIVE
