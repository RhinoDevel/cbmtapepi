
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
 * - Expects: MT_TAPE_GPIO_PIN_NR_READ configured as output (data-ready line).
 *            MT_TAPE_GPIO_PIN_NR_WRITE configured as input with pull-down
 *            (date-request line).
 *            MT_TAPE_GPIO_PIN_NR_SENSE configured as output (data line).
 *
 * - Initialization: 1) Sets data-ready line to default level.
 *                   2) Waits for data-request line to get to default level.
 *                   3) Waits for level change FROM default level, then starts
 *                      transfer.
 *
 *                   <=> petload_send() must already be waiting BEFORE CBM
 *                       machine's first data-request.
 */
void petload_send(uint8_t const * const bytes, uint32_t const count);

#endif //MT_PETLOAD
