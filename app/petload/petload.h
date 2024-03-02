
// Marcel Timm, RhinoDevel, 2019dec16

#ifndef MT_PETLOAD
#define MT_PETLOAD

#include "../tape/tape_input.h"

/** Wait for given value on data-ready line (motor signal).
 *  Make sure that signal is really set to given value, if wanted (second
 *  parameter).
 */
void petload_wait_for_data_ready_val(
    bool const wait_for_val, 
    bool const do_make_sure,
    bool (*is_stop_requested)());

/** Return C64 fast loader, stored at top of highest free memory.
 *
 * - Caller takes ownership of returned object.
 */
struct tape_input * petload_create_c64tof();

/** Return C64 fast loader, stored at top of (BASIC) memory.
 *
 * - Caller takes ownership of returned object.
 */
struct tape_input * petload_create_c64tom();

/** Return VIC 20 fast loader, stored at top of memory.
 *
 * - Caller takes ownership of returned object.
 */
struct tape_input * petload_create_vic20tom();

/** Return PET fast loader for BASIC v4, stored at top of memory.
 *
 * - Caller takes ownership of returned object.
 */
struct tape_input * petload_create_v4tom();

/** Return PET fast loader for BASIC v4.
 *
 * - Caller takes ownership of returned object.
 */
struct tape_input * petload_create_v4();

/** Return PET fast loader for BASIC v2, stored at top of memory.
 *
 * - Caller takes ownership of returned object.
 */
struct tape_input * petload_create_v2tom();

/** Return PET fast loader for BASIC v2.
 *
 * - Caller takes ownership of returned object.
 */
struct tape_input * petload_create_v2();

/** Return PET fast loader for BASIC v1, stored at top of memory.
 *
 * - Caller takes ownership of returned object.
 */
struct tape_input * petload_create_v1tom();

/** Return PET fast loader for BASIC v1.
 *
 * - Caller takes ownership of returned object.
 */
struct tape_input * petload_create_v1();

/**
 * - It does NOT matter, if CBM is sending data and waiting for ACK first or if
 *   Pi is waiting for data [petload_retrieve() already called] first.
 *
 * - MOTOR line (data-ready from PET) must be at least on its way to LOW when
 *   calling function and will be at least on its way to LOW on return from
 *   function.
 *
 * - READ line (data-ack. to PET) does not toggle, but uses a pulse,
 *   because that line triggers a flag bit to be set to 1 in PET on HIGH-to-LOW.
 *
 * - READ line will be on its default level (HIGH) on exit of this function
 *   [made sure, because also used by petload_send()].
 *
 * - Makes sure that first data-ack. from PET level that is to-be-expected
 *   in petload_send() call that is necessarily done after calling this function
 *   (but maybe with some timespan between both calls) is set correctly.
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
 *
 * - It does NOT matter, if CBM is waiting for data first or if Pi is waiting
 *   to send data first.
 *
 * - SENSE line (data to PET) will be HIGH on return from function.
 *
 * - WRITE line (data-ack. from PET) value is (indirectly) defined by
 *   last bit's value retrieved during petload_retrieve() run done before
 *   calling this function.
 *
 * - READ line (data-ready to PET) does not toggle, but uses a pulse,
 *   because that line triggers a flag bit to be set to 1 in PET on HIGH-to-LOW.
 *
 * - READ line is expected to be on its default level (HIGH) when calling this
 *   function. Preceding call of petload_retrieve() makes sure of that.
 */
void petload_send(uint8_t const * const bytes, uint32_t const count);

void petload_send_nop();

#endif //MT_PETLOAD
