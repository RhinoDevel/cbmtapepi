
// Marcel Timm, RhinoDevel, 2018dec12

#ifndef MT_WATCHDOG
#define MT_WATCHDOG

#include <stdint.h>

// Broadcom BCM2708 watchdog timer.
//
// Original source code:
//
// https://github.com/dwelch67/raspberrypi/blob/master/blinker06/wdog.c

/** Convenience function to trigger system reset by letting the watchdog timer
 *  run out.
 */
void watchdog_reset_system();

#endif //MT_WATCHDOG
