
// Marcel Timm, RhinoDevel, 2020dec20

#ifndef MT_MODE
#define MT_MODE

#include "mode_type.h"

#include <stdbool.h>

/**
 *  - Returns mode_type_err on error.
 */
enum mode_type mode_load();

/**
 *  - Won't save mode_type_err, because that value indicates an error on load
 *    via mode_load().
 */
bool mode_save(enum mode_type const type);

#endif //MT_MODE
