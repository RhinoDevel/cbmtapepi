
// Marcel Timm, RhinoDevel, 2018jan24

#ifndef MT_MEM
#define MT_MEM

#include <stdint.h>

/** Read at given address from memory.
 */
uint32_t mem_read(uint32_t const addr);

/** Write value given to memory at given address.
 */
void mem_write(uint32_t const addr, uint32_t const val);

#endif //MT_MEM
