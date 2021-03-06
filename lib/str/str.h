
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
 * - Returns -1, if not found.
 */
int str_get_last_index(char const * const s, char const c);

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
char* str_create_partial_copy(
    char const * const s, uint32_t const index, uint32_t const count);

/**
 * - Caller takes ownership of returned object.
 */
char* str_create_copy(char const * const s);

/**
 * - Returns true, if both strings are empty.
 */
bool str_starts_with(char const * const s, char const * const start);

/** Return first index of given character in given string that is at most
 *  followed by the same character until the '\0'.
 *
 +  Returns -1, if character is not found at end of string.
 */
int str_get_index_of_trailing(char const * const s, char const c);

/** Concatenate both strings into new string to-be-returned.
 *
 *  - Caller takes ownership of returned object.
 */
char* str_create_concat(char const * const first, char const * const last);

/** Concatenate three strings into new string to-be-returned.
 *
 *  - Caller takes ownership of returned object.
 */
char* str_create_concat_three(
    char const * const first,
    char const * const middle,
    char const * const last);

int str_cmp(char const * const a, char const * const b);

bool str_are_equal(char const * const a, char const * const b);

#endif //MT_STR
