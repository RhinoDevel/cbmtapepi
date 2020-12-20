
// Marcel Timm, RhinoDevel, 2020dec19

#ifndef MT_CFG
#define MT_CFG

#include <stdint.h>
#include <stdbool.h>

#define CFG_ERR 0xFF

/**
 * - Returns CFG_ERR on error.
 */
uint8_t cfg_load(char const * const dir_path, char const * const filename);

/**
 * - Won't save CFG_ERR, because this is also the value for a load error.
 */
bool cfg_save(
    char const * const dir_path,
    char const * const filename,
    uint8_t const mode);

#endif //MT_CFG
