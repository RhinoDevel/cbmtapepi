
// RhinoDevel, MT, 2018nov19

#ifndef MT_ALLOC
#define MT_ALLOC

#include <stdint.h>

#include "allocconf.h"

void alloc_free(void * const block_addr);

void* alloc_alloc(MT_USIGN const wanted_len);

void alloc_init(void * const mem, MT_USIGN const mem_len);

#endif //MT_ALLOC
