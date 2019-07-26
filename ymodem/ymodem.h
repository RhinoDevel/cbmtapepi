
// Marcel Timm, RhinoDevel, 2018dec31

// Linux commands to send files:
//
// stty -F /dev/ttyUSB0 115200
// sx --ymodem kernel.img < /dev/ttyUSB0 > /dev/ttyUSB0

#ifndef MT_YMODEM
#define MT_YMODEM

#include <stdbool.h>

#include "ymodem_send_params.h"
#include "ymodem_send_err.h"

#include "ymodem_receive_params.h"
#include "ymodem_receive_err.h"

/**
 * - Supports sending one file, only.
 * - No file date or other attributes in block 0 (just file name and length).
 */
enum ymodem_send_err ymodem_send(
    struct ymodem_send_params * const p,
    uint8_t * const debug_buf,
    uint32_t * const debug_buf_len);

/**
 * - Given buffer will hold the LAST file retrieved, only (.file_len property
 *   will hold the size of that LAST retrieved file, .name its name's first
 *   characters).
 */
enum ymodem_receive_err ymodem_receive(struct ymodem_receive_params * const p);

#endif //MT_YMODEM
