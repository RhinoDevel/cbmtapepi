
// Marcel Timm, RhinoDevel, 2018jan24

#ifndef MT_MEM
#define MT_MEM

#include <stdint.h>
#include <stdbool.h>

/** Read at given address from memory.
 */
uint32_t mem_read(uint32_t const addr);

/** Write value given to memory at given address.
 */
void mem_write(uint32_t const addr, uint32_t const val);

/** Return true, if len bytes from given addresses on are equal,
 *  otherwise false.
 */
bool mem_cmp_byte(
    uint8_t const * const a, uint8_t const * const b, uint32_t const len);

#endif //MT_MEM
