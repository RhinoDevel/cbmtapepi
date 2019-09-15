
// Marcel Timm, RhinoDevel, 2019sep15

#ifndef MT_BASIC
#define MT_BASIC

#include <stdint.h>

/**
 * - Fills given len with length of return value in byte.
 * - Caller takes ownership of return value.
 */
uint8_t* basic_get_sample(uint16_t const addr, uint32_t * const len);

#endif //MT_BASIC
