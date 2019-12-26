
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
 * - Expects: MT_TAPE_GPIO_PIN_NR_READ configured as output.
 *            MT_TAPE_GPIO_PIN_NR_WRITE configured as input with pull-down.
 *            MT_TAPE_GPIO_PIN_NR_SENSE configured as output.
 * - Waits for level change on MT_TAPE_GPIO_PIN_NR_WRITE before starting
 *   transfer.
 */
void petload_send(uint8_t const * const bytes, uint32_t const count);

#endif //MT_PETLOAD
