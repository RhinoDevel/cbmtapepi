
// Marcel Timm, RhinoDevel, 2019jan05

#ifndef MT_STR
#define MT_STR

#include <stdint.h>

uint32_t str_get_len(char const * const s);

/**
 * - In and output strings may be equal.
 */
void str_to_upper(char * const s_out, char const * const s_in);

/**
 * - In and output strings may be equal.
 */
void str_to_lower(char * const s_out, char const * const s_in);

#endif //MT_STR
