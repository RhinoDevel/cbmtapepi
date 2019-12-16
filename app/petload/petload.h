
// Marcel Timm, RhinoDevel, 2019dec16

#ifndef MT_PETLOAD
#define MT_PETLOAD

#include "../tape/tape_input.h"

/** Return PET fast loader.
 *
 * - Caller takes ownership of returned object.
 */
struct tape_input * petload_create();

#endif //MT_PETLOAD
