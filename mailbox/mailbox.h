
// Marcel Timm, RhinoDevel, 2019aug08

#ifndef MT_MAILBOX
#define MT_MAILBOX

#include <stdint.h>

/**
 * - Expects 28 bit value.
 */
void mailbox_write(uint32_t const channel, uint32_t const val);

/**
 * - Returns higher 28 bits (without the channel nr.).
 */
uint32_t mailbox_read(uint32_t const channel);

#endif //MT_MAILBOX
