
// Marcel Timm, RhinoDevel, 2020dec19

#ifndef MT_CFG
#define MT_CFG

#include <stdint.h>
#include <stdbool.h>

/**
 * - Returns 0xFF on error.
 */
uint8_t cfg_load_mode(char const * const dir_path, char const * const filename);

/**
 * - Won't save 0xFF, becuase this is also the magic value for a load error.
 */
bool cfg_save_mode(
    char const * const dir_path,
    char const * const filename,
    uint8_t const mode);

#endif //MT_CFG