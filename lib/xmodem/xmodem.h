
// Marcel Timm, RhinoDevel, 2018dec18

// Linux commands to send files:
//
// stty -F /dev/ttyUSB0 115200
// sx kernel.img < /dev/ttyUSB0 > /dev/ttyUSB0

#ifndef MT_XMODEM
#define MT_XMODEM

#include <stdbool.h>

#include "xmodem_receive_params.h"

bool xmodem_receive(
    struct xmodem_receive_params const * const p, uint32_t * const count);

#endif //MT_XMODEM
