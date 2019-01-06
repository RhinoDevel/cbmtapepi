
// Marcel Timm, RhinoDevel, 2018dec31

// Linux commands to send files:
//
// stty -F /dev/ttyUSB0 115200
// sx --ymodem kernel.img < /dev/ttyUSB0 > /dev/ttyUSB0

#ifndef MT_YMODEM
#define MT_YMODEM

#include <stdbool.h>

#include "ymodem_receive_params.h"
#include "ymodem_receive_err.h"

/**
 * - Given buffer will hold the LAST file retrieved, only (.file_len property
 *   will hold the size of that LAST retrieved file, .name its name's first
 *   characters).
 */
enum ymodem_receive_err ymodem_receive(struct ymodem_receive_params * const p);

#endif //MT_YMODEM
