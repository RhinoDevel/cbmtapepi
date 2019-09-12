
// Marcel Timm, RhinoDevel, 2019sep12

#ifndef MT_CBM_RECEIVE
#define MT_CBM_RECEIVE

#include "../tape/tape_input.h"

#include <stdbool.h>

/**
 * - Given function is optional.
 * - Caller takes ownership of returned object.
 * - Returns NULL on error.
 */
struct tape_input * cbm_receive(bool (*is_stop_requested)());

#endif //MT_CBM_RECEIVE
