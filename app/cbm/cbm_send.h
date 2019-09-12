
// Marcel Timm, RhinoDevel, 2019sep12

#ifndef MT_CBM_SEND
#define MT_CBM_SEND

#include <stdint.h>
#include <stdbool.h>

/**
 * - Given function is optional.
 */
bool cbm_send(
    uint8_t /*const*/ * const bytes,
    char const * const name,
    uint32_t const count,
    bool (*is_stop_requested)());

#endif //MT_CBM_SEND
