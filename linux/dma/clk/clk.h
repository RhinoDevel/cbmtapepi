
// Marcel Timm, RhinoDevel, 2021nov01

#ifndef MT_CLK
#define MT_CLK

#include <stdbool.h>
#include <stdint.h>

// The clock used is explained in the PDF file "BCM2835 Audio & PWM clocks"
// by G.J. van Loo, 6 February 2013. 

// This is NOT a universal interface to the (a) clock, but to be used by the PWM
// setup for paced DMA access.

void clk_deinit();

/**
 * - Delays for 100 us.
 */
bool clk_init(uint32_t const freq_hz);

#endif //MT_CLK
