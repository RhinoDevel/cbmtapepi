
// Marcel Timm, RhinoDevel, 2019dec16

#ifndef MT_PETLOAD
#define MT_PETLOAD

#include "../tape/tape_input.h"

/** Return PET fast loader.
 *
 * - Caller takes ownership of returned object.
 */
struct tape_input * petload_create();

/**
 * - petload_retrieve() must already be waiting BEFORE CBM machine's first
 *   fast mode command to-be-retrieved.
 *
 * - This uses tape_input struct, but this is just by convention and for
 *   convenience.
 *
 * - Caller takes ownership of returned object.
 */
struct tape_input * petload_retrieve();

/**
 * - Must be called some time after petload_retrieve() without anything else
 *   using the GPIO pins connected to the CBM.
 */
void petload_send(uint8_t const * const bytes, uint32_t const count);

#endif //MT_PETLOAD
