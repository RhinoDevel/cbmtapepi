
// Marcel Timm, RhinoDevel, 2019jan05

#include <stdint.h>

#include "str.h"

uint32_t str_get_len(char const * const s)
{
    uint32_t ret_val = 0;

    while(s[ret_val] != '\0')
    {
        ++ret_val;
    }
    return ret_val;
}
