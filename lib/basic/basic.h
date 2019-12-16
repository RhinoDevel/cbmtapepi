
// Marcel Timm, RhinoDevel, 2019sep15

#ifndef MT_BASIC
#define MT_BASIC

#include <stdint.h>
#include <stdbool.h>

#define MT_BASIC_MAX_CHAR_PER_LOGICAL_LINE 80

/** Return BASIC PRG binary printing each string given in one line.
 *
 * - Fills given len with length of return value in byte.
 * - Caller takes ownership of return value.
 * - Returns 0, if at least one string in given array is longer than
 *   MT_BASIC_MAX_CHAR_PER_LOGICAL_LINE.
 */
uint8_t* basic_get_prints(
    uint16_t const addr,
    char const * const * const str_arr,
    uint32_t const str_count,
    char const petscii_not_found_replacer,
    uint32_t * const len);

/**
 * - Fills given len with length of return value in byte.
 * - Caller takes ownership of return value.
 */
uint8_t* basic_get_sample(uint16_t const addr, uint32_t * const len);

/**
 * - Fills given len with length of return value in byte.
 * - Caller takes ownership of return value.
 */
uint8_t* basic_get_sys(
    uint16_t const addr,
    uint16_t const sys_addr,
    bool const skip_addr,
    uint32_t * const len);

#endif //MT_BASIC
