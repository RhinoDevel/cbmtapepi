
// Marcel Timm, RhinoDevel, 2019aug08

#ifndef MT_MAILBOX
#define MT_MAILBOX

#include <stdint.h>
#include <stdbool.h>

#include "../peribase.h"

// Source:
//
// https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
//
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
    mailbox_id_clockrate_pwm = 10,
    mailbox_id_clockrate_hevc = 11,
    mailbox_id_clockrate_emmc2 = 12,
    mailbox_id_clockrate_m2mc = 13,
    mailbox_id_clockrate_pixel_bvb = 14
};

/**
 * - Expects 28 bit value.
 */
void mailbox_write(uint32_t const channel, uint32_t const val);

/**
 * - Returns higher 28 bits (without the channel nr.).
 */
uint32_t mailbox_read(uint32_t const channel);

/**
 * - Returns UINT32_MAX on error.
 */
uint32_t mailbox_read_clockrate(enum mailbox_id_clockrate const id);

#if PERI_BASE_PI_VER == 3

/**
 * - Returns UINT32_MAX on error.
 */
uint32_t mailbox_write_gpio_actled(bool const high);

#endif //PERI_BASE_PI_VER == 3

#endif //MT_MAILBOX
