// Marcel Timm, RhinoDevel, 2018jan24

#ifndef MT_BUSYWAIT
#define MT_BUSYWAIT

#include <stdint.h>

/** Busy-wait for given seconds
 *  (valid input values go from 1 to 17179 seconds).
 *
 * - Hard-coded for 250 MHz core clock.
 */
void busywait_seconds(uint32_t const seconds);

/** Busy-wait for given milli seconds
 *  (valid input values go from 1 to 17179869 milliseconds).
 *
 * - Hard-coded for 250 MHz core clock.
 */
void busywait_milliseconds(uint32_t const milliseconds);

/** Busy-wait for given micro seconds
 *  (valid input values go from 1 to 4294967295 microseconds).
 *
 * - Hard-coded for 250 MHz core clock.
 */
void busywait_microseconds(uint32_t const microseconds);

/** Busy-wait for given nano seconds
 *  (valid input values go from 4 to 4294967292 nanoseconds,
 *  where given value must be dividable by 4 to get precise results).
 *
 * - Hard-coded for 250 MHz core clock.
 */
void busywait_nanoseconds(uint32_t const nanoseconds);

/** Busy-wait for given count of core clock cycles.
 *  (valid input values go from 1 to 1073741823 clock cycles).
 *
 * * Does NOT take function time into account.
 * - Hard-coded for 250 MHz core clock.
 */
void busywait_clockcycles(uint32_t const clockcycles);

#endif //MT_BUSYWAIT
