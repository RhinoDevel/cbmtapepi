
// RhinoDevel, MT, 2018nov18

#ifndef MT_MEM
#define MT_MEM

#include <stdint.h>

#include "allocconf.h"

void mem_clear(void * const addr, MT_USIGN const len, uint8_t const val);

#ifndef NDEBUG
void mem_print(void const * const mem, MT_USIGN const mem_len);
#endif //NDEBUG

#endif //MT_MEM
