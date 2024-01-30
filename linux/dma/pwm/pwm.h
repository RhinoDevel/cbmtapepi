
// Marcel Timm, RhinoDevel, 2021oct31

// Pages mentioned in source code comments can be found in the document
// "BCM2835 ARM Peripherals".
//
// Also you MUST USE https://elinux.org/BCM2835_datasheet_errata
// together with that document!

#ifndef MT_PWM
#define MT_PWM

#include <stdbool.h>
#include <stdint.h>

// Page 141 (more in the C file):
//
#define PWM_OFFSET_FIF1 ((uint32_t)0x18) // Channel 1 FIFO input.

// This is NOT a universal interface to the PWM controller, but to be used to
// let the DMA controller get paced by the PWM.

void pwm_deinit();
void pwm_stop();
void pwm_start();

/**
 * - Given frequency divided by range will be the resulting frequency the DMA
 *   controller will get paced by, e.g.: 100000 Hz / 20000 = 5 Hz 
 * - Delays for at least 500 us.
 */
bool pwm_init(uint32_t const clk_freq_hz, uint32_t const range);

#endif //MT_PWM
