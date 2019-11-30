
// Marcel Timm, RhinoDevel, 2019jan05

#ifndef MT_STR
#define MT_STR

#include <stdint.h>
#include <stdbool.h>

uint32_t str_get_len(char const * const s);

/** Return character count of all strings in given string array.
 *
 *  Returns 0, if given count is 0.
 */
uint32_t str_get_len_sum(
    char const * const * const s_arr, uint32_t const s_count);

/** Return character count of longest string in given string array.
 *
 *  Returns 0, if given count is 0.
 */
uint32_t str_get_len_max(
    char const * const * const s_arr, uint32_t const s_count);

/**
 * - In and output strings may be equal.
 */
void str_to_upper(char * const s_out, char const * const s_in);

/**
 * - In and output strings may be equal.
 */
void str_to_lower(char * const s_out, char const * const s_in);

/**
 * - In and output strings may be equal.
 */
void str_copy(char * const s_out, char const * const s_in);

/**
 * - Caller takes ownership of returned object.
 */
char* str_create_copy(char const * const s);

/**
 * - Returns true, if both strings are empty.
 */
bool str_starts_with(char const * const s, char const * const start);

#endif //MT_STR
