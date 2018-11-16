
// Marcel Timm, RhinoDevel, 2018jan27

#ifndef MT_TAPE_SEND
#define MT_TAPE_SEND

#include <stdbool.h>

#include "tape_send_params.h"

bool tape_send(struct tape_send_params const * const p, uint32_t * const mem);

#endif //MT_TAPE_SEND
