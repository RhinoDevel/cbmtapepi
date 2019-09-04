
// Marcel Timm, RhinoDevel, 2018jan24

#include "mem.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

uint32_t mem_read(uint32_t const addr)
{
    return *(volatile uint32_t const *)addr;
}

void mem_write(uint32_t const addr, uint32_t const val)
{
    *(volatile uint32_t *)addr = val;
}

bool mem_cmp_byte(
    uint8_t const * const a, uint8_t const * const b, uint32_t const len)
{
    uint32_t i = 0;

    while(i < len)
    {
        if(*(a + i) != *(b + i))
        {
            return false;
        }
        ++i;
    }
    return true;
}

#ifndef MT_LINUX
    void* memset(void* ptr, int value, size_t num)
    {
        unsigned char const v = (unsigned char)value;
        unsigned char * const p = (unsigned char *)ptr;

        for(size_t i = 0;i < num; ++i)
        {
            p[i] = v;
        }
        return ptr;
    }

    void* memcpy(void* ptr, void const * src, size_t num)
    {
        unsigned char const * const s = (unsigned char const *)src;
        unsigned char * const p = (unsigned char *)ptr;

        for(size_t i = 0;i < num; ++i)
        {
            p[i] = s[i];
        }
        return ptr;
    }
#endif //MT_LINUX
