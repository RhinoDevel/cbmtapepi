
// Marcel Timm, RhinoDevel, 2019aug08

#ifndef MT_MAILBOX
#define MT_MAILBOX

#include <stdint.h>

enum mailbox_id_clockrate
{
    mailbox_id_clockrate_emmc = 1,
    mailbox_id_clockrate_uart = 2,
    mailbox_id_clockrate_arm = 3,
    mailbox_id_clockrate_core = 4,
    mailbox_id_clockrate_v3d = 5,
    mailbox_id_clockrate_h264 = 6,
    mailbox_id_clockrate_isp = 7,
    mailbox_id_clockrate_sdram = 8,
    mailbox_id_clockrate_pixel = 9,
    mailbox_id_clockrate_pwm = 10
};

/**
 * - Expects 28 bit value.
 */
void mailbox_write(uint32_t const channel, uint32_t const val);

/**
 * - Returns higher 28 bits (without the channel nr.).
 */
uint32_t mailbox_read(uint32_t const channel);

uint32_t mailbox_read_clockrate(enum mailbox_id_clockrate const id);

#endif //MT_MAILBOX
