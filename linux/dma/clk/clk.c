// Marcel Timm, RhinoDevel, 2021oct31

// Pages mentioned in source code comments can be found in the document
// "BCM2835 ARM Peripherals".
//
// Also you MUST USE https://elinux.org/BCM2835_datasheet_errata
// together with that document!
//
// The clock used here is also mentioned and explained better in the PDF file
// "BCM2835 Audio & PWM clocks" by G.J. van Loo, 6 February 2013. 

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>

#include "clk.h"
#include "../reg.h"
#include "../mem/mem.h"

#define CLK_OFFSET_PWM_CTL ((uint32_t)0xa0) // "CM_PWMCTL" (see https://elinux.org/BCM2835_registers).
#define CLK_OFFSET_PWM_DIV ((uint32_t)0xa4) // "CM_PWMDIV" (see https://elinux.org/BCM2835_registers).
//
#define CLK_PWM_ID 0xa
#define CLK_PASSWD ((uint32_t)0x5a000000) // (already shifted..)
#define CLK_PLL_FREQ_HZ ((uint32_t)250000000) // TODO: Hard-coded for Raspberry Pi 0 - 3, seems to be 375 MHz for 3-4!

static void* s_mapped_clk = NULL; // See clk_init() & clk_deinit().

void clk_deinit()
{
    if(s_mapped_clk == NULL)
    {
        return; // Do nothing, if not initialized.
    }

    mem_free_mapped_clk(s_mapped_clk);
    s_mapped_clk = NULL;
}

bool clk_init(uint32_t const freq_hz)
{
    assert(s_mapped_clk == NULL);

    // Get access to clock via mapping it into virtual address space:
    //
    s_mapped_clk = mem_create_mapped_clk();
    if(s_mapped_clk == NULL)
    {
        return false;
    }

    // Incomplete:
    //
    // Bits  | Field  | Description                                       | R/W 
    // ------|--------|-------------------------------------------------- |-----
    // 31-24 | PASSWD | Clock manager password 0x5A.                      | W   
    //       |        |                                                   |
    //     7 | BUSY   | 1 = Clock generator is running.                   | R
    //       |        |                                                   |
    //     5 | KILL   | Kill the clock generator.                         | R/W
    //       |        | 0 = No action.                                    |
    //       |        | 1 = Stop and reset clock generator.               |

    // Stop the clock generator:
    //
    *REG_PTR(s_mapped_clk, CLK_OFFSET_PWM_CTL) = CLK_PASSWD | (1 << 5);

    // Wait for the clock generator to have stopped:
    //
    while((*REG_PTR(s_mapped_clk, CLK_OFFSET_PWM_CTL) & (1 << 7)) != 0)
    {
        ; // Busy wait..
    }

    // Example:
    // --------
    //
    // PLL (source) frequency = 250 MHz
    // Given frequency        = 100 kHz  
    //
    // Divisor                = 250 MHz        / 100 kHz
    //                        = 1000 * 250 kHz / 100 kHz
    //                        = 2500
    //                          ====
    //
    //  => Clock frequency    = 250 MHz / 2500
    //                        = 100 kHz
    //                          =======
    //
    // - The minimum divisor without MASH filter is 1.
    // - The maximum divisor is 4095 / 0x0FFF (also see below).
    // - We are not using a fractional part, so a MASH filter makes no sense
    //   anyway (if I understand correctly..).
    //
    uint32_t const div = CLK_PLL_FREQ_HZ / freq_hz;

    // Set the integer part of the divisor (bits 12 to 23, 12 bits) to the value
    // and the fractional part of the divisor to 0:
    //
    *REG_PTR(s_mapped_clk, CLK_OFFSET_PWM_DIV) = CLK_PASSWD | (div << 12);

    // Enable the clock generator (bit 4) and set the clock source to 6 <=>
    // "PLLD per" (bits 0 to 3):
    //
    *REG_PTR(s_mapped_clk, CLK_OFFSET_PWM_CTL) =
        CLK_PASSWD | (1 << 4) | (1 << 2) | (1 << 1);

    // Wait for the clock generator having started:
    //
    while((*REG_PTR(s_mapped_clk, CLK_OFFSET_PWM_CTL) & (1 << 7)) == 0)
    {
        ; // Busy wait..
    }

    usleep(100);

    return true;
}

