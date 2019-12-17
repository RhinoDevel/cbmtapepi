
// Marcel Timm, RhinoDevel, 2019sep12

#ifndef MT_CBM_SEND
#define MT_CBM_SEND

#include "../tape/tape_input.h"

#include <stdint.h>
#include <stdbool.h>

void cbm_send_fill_name(
    uint8_t * const name_out, char const * const name_in);

/**
 * - Given function is optional.
 */
bool cbm_send_data(struct tape_input * const data, bool (*is_stop_requested)());

/**
 * - Given function is optional.
 */
bool cbm_send(
    uint8_t /*const*/ * const bytes,
    char const * const name,
    uint32_t const count,
    bool (*is_stop_requested)());

#endif //MT_CBM_SEND
