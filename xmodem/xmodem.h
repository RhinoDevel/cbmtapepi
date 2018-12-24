
// Marcel Timm, RhinoDevel, 2018dec18

#ifndef MT_XMODEM
#define MT_XMODEM

#include <stdbool.h>

#include "xmodem_receive_params.h"

bool xmodem_receive(
    struct xmodem_receive_params const * const p, uint32_t * const count);

#endif //MT_XMODEM
