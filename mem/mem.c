
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
