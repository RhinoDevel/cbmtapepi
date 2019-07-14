
// Marcel Timm, RhinoDevel, 2018jan24

#include "mem.h"

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
