
// Marcel Timm, RhinoDevel, 2021oct23

#ifndef MT_REG
#define MT_REG

#include <stdint.h>

// Get a volatile 32-bit unsigned integer pointer to the memory location at the
// given byte offset from the given base pointer.
//
#define REG_PTR(base_ptr, byte_offset) \
    ((volatile uint32_t *)((uint8_t*)(base_ptr) + (int)(byte_offset)))

#endif //MT_REG
