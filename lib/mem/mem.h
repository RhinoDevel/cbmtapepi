
// Marcel Timm, RhinoDevel, 2018jan24

#ifndef MT_MEM
#define MT_MEM

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

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

#ifndef MT_LINUX
    /**
     * - Implemented for the linker (e.g. for structs with function pointers set
     *   to NULL or just not set explicitly to anything).
     * - Returns ptr.
     */
    void* memset(void* ptr, int value, size_t num);

    /**
     * - Returns ptr.
     */
    void* memcpy(void* ptr, void const * src, size_t num);
#endif //MT_LINUX

#endif //MT_MEM
