
// Marcel Timm, RhinoDevel, 2021oct23

#ifndef MT_MBO
#define MT_MBO

/** Lock buffer in place and return the bus address.
 * 
 *  - Returns 0 on error.
 */
uint32_t mbo_lock_mem(uint32_t const addr); 

bool mbo_unlock_mem(uint32_t const addr);

uint32_t mbo_alloc_mem(
    uint32_t const size, uint32_t const alignment, uint32_t const flags);

bool mbo_free_mem(uint32_t const addr);

#endif //MT_MBO
