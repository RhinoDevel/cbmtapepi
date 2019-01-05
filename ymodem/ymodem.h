
// Marcel Timm, RhinoDevel, 2018dec31

// Linux commands to send files:
//
// stty -F /dev/ttyUSB0 115200
// sx --ymodem kernel.img < /dev/ttyUSB0 > /dev/ttyUSB0

#ifndef MT_YMODEM
#define MT_YMODEM

#include <stdbool.h>

#include "ymodem_receive_params.h"

/**
 * - Given buffer will hold the LAST file retrieved, only (count output
 *   parameter will hold the size of that LAST retrieved file).
 */
bool ymodem_receive(
    struct ymodem_receive_params const * const p, uint32_t * const count);

#endif //MT_YMODEM
