
// Marcel Timm, RhinoDevel, 2019dec06

#ifndef MT_SORT
#define MT_SORT

#include <stddef.h>

// Example: Compare function for an array of strings:
//
// static int cmp_str(void const * const a, void const * const b)
// {
//     return str_cmp(*(char const * const *)a, *(char const * const *)b);
// }

/** Sort elements in given array in-place via given compare function and
 *  insertion sort algorithm.
 */
void sort_insertion(
    void * const arr,
    size_t const arr_len,
    size_t const ele_len,
    int (*cmp)(void const *, void const *));

#endif //MT_SORT
