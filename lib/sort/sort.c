
// Marcel Timm, RhinoDevel, 2019dec06

#include "sort.h"
#include "../mem/mem.h"
#include "../alloc/alloc.h"

#include <stddef.h>
#include <stdint.h>

void sort_insertion(
    void * const arr,
    size_t const arr_len,
    size_t const ele_len,
    int (*cmp)(void const *, void const *))
{
    uint8_t * const t = alloc_alloc(ele_len * sizeof *t);

    for(size_t i = 1;i < arr_len;++i)
    {
        size_t j = i;

        // Save original element found at index i:
        //
        memcpy(t, ((uint8_t const *)arr + i * ele_len), ele_len);

        while(j >= 1)
        {
            // Pointer to element at index j - 1:
            //
            void const * const s = ((uint8_t const *)arr + (j - 1) * ele_len);

            if(cmp(s, t) <= 0)
            {
                break; // arr[j - 1] <= t
            }

            // arr[j - 1] > t

            memcpy( // arr[j] = arr[j - 1]
                (uint8_t*)arr + j * ele_len,
                (uint8_t const *)arr + (j - 1) * ele_len,
                ele_len);

            --j;
        }
        memcpy((uint8_t*)arr + j * ele_len, t, ele_len); // arr[j] = t
    }

    alloc_free(t);
}
