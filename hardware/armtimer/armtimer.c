
// Marcel Timm, RhinoDevel, 2018jan24

// Pages mentioned in source code comments can be found in the document
// "BCM2835 ARM Peripherals".
//
// Also you MUST USE https://elinux.org/BCM2835_datasheet_errata
// together with that document!

#include "armtimer.h"
#include "../peribase.h"
#include "../../lib/mem/mem.h"

#include <stdbool.h>

// ARM Timer (based on an SP804 - NOT an "AP804" -, see page 196):
//
#define ARM_TIMER_BASE (PERI_BASE + 0xB000)
#define ARM_TIMER_LOD (ARM_TIMER_BASE + 0x400) // Load
#define ARM_TIMER_VAL (ARM_TIMER_BASE + 0x404) // Value (read only).
#define ARM_TIMER_CTL (ARM_TIMER_BASE + 0x408) // Control
#define ARM_TIMER_CLI (ARM_TIMER_BASE + 0x40C) // IRQ clear/ACK (write only).
#define ARM_TIMER_RIS (ARM_TIMER_BASE + 0x410) // Raw IRQ (read only).
#define ARM_TIMER_MIS (ARM_TIMER_BASE + 0x414) // Masked IRQ (read only).
#define ARM_TIMER_RLD (ARM_TIMER_BASE + 0x418) // Reload
#define ARM_TIMER_DIV (ARM_TIMER_BASE + 0x41C) // Pre-divider / pre-scaler.
#define ARM_TIMER_CNT (ARM_TIMER_BASE + 0x420) // Free running counter.

static bool s_is_running_at_one_mhz = false;

uint32_t armtimer_get_tick()
{
    return mem_read(ARM_TIMER_CNT);
}

void armtimer_start_one_mhz()
{
    if(s_is_running_at_one_mhz)
    {
        return;
    }

    // 0xF9 = 249 => 250 MHz / ( 249 + 1) = 1 MHz (see page 197)
    //
    mem_write(ARM_TIMER_CTL, 0x00F90000); // Set frequency and disable.
    mem_write(ARM_TIMER_CTL, 0x00F90200); // Start free running counter.

    s_is_running_at_one_mhz = true;
}
//
void armtimer_busywait(uint32_t const start_val, uint32_t const divider)
{
    s_is_running_at_one_mhz = false;

    mem_write(ARM_TIMER_CTL, 0x003E0000); // Disables counter.
    mem_write(ARM_TIMER_LOD, start_val-1); // Initial value to count down from.
    //mem_write(ARM_TIMER_RLD, start_val-1); // Used after count to zero done.
    mem_write(ARM_TIMER_DIV, divider); // [10 bits are avail. (page 199)].
    mem_write(ARM_TIMER_CLI, 0);
    mem_write(ARM_TIMER_CTL, 0x003E0082); // Enables 32-bit counter.
    while(mem_read(ARM_TIMER_RIS)==0) // Polling interrupt flag.
    {
        ;
    }
    //mem_write(ARM_TIMER_CTL, 0x003E0000); // Disables counter.
}

void armtimer_busywait_microseconds(uint32_t const microseconds)
{
    uint32_t rx;

    armtimer_start_one_mhz(); // Starts counter, if not already running.

    rx = armtimer_get_tick();
    while(armtimer_get_tick() - rx < microseconds)
    {
        // (do nothing)
    }
}
