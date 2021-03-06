
// Marcel Timm, RhinoDevel, 2018jan24

// Pages mentioned in source code comments can be found in the document
// "BCM2835 ARM Peripherals".
//
// Also you MUST USE https://elinux.org/BCM2835_datasheet_errata
// together with that document!

#include "armtimer.h"
#include "../../lib/mem/mem.h"

#include <stdbool.h>

// ARM Timer (based on an SP804 - NOT an "AP804" -, see page 196):
//
// ARM_TIMER_BASE is defined in header file.
#define ARM_TIMER_LOD (ARM_TIMER_BASE + 0x400) // Load
#define ARM_TIMER_VAL (ARM_TIMER_BASE + 0x404) // Value (read only).
#define ARM_TIMER_CTL (ARM_TIMER_BASE + 0x408) // Control
// ARM_TIMER_CLI is defined in header file.
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

// void armtimer_irq_clear()
// {
//     mem_write(ARM_TIMER_CLI, 0);
// }

void armtimer_start_one_mhz()
{
    uint32_t buf, pre_scaler;

    if(s_is_running_at_one_mhz)
    {
        return;
    }

    buf = mem_read(ARM_TIMER_CTL); // Current state.

    // Disable (maybe running) free-running counter by setting bit 9 to zero:
    //
    buf &= 0xFFFDFFFF;
    mem_write(ARM_TIMER_CTL, buf);

    // Prepare free-running counter pre-scaler bits:
    //
    // 0xF9 = 249 => 250 MHz / ( 249 + 1) = 1 MHz (see page 197)
    //
    pre_scaler = 0xF9;
    buf &= 0xFF00FFFF; // Sets pre-scaler bits to zero.
    buf |= (pre_scaler << 16); // Set pre-scaler bits to correct value.

    // Prepare free-running counter enable bit 9:
    //
    buf |= (1 << 9);

    // Setup and start free running counter:
    //
    mem_write(ARM_TIMER_CTL, buf);

    s_is_running_at_one_mhz = true;
}

/**
 * - Does NOT disable maybe running free-running counter, just the timer.
 */
static void disable_timer_and_interrupt()
{
    uint32_t buf = mem_read(ARM_TIMER_CTL); // Current state.

    // Disable (maybe running) timer by setting bit 7 to zero
    // AND disable interrupt by setting bit 5 to zero:
    //
    buf &= 0xFFFFFF7F; // Will disable timer.
    buf &= 0xFFFFFFDF; // Wil disable interrupt.
    mem_write(ARM_TIMER_CTL, buf); // Disable timer and interrupt.
}

/**
 * - Does NOT use free-running counter, but the timer.
 * - Counts with 32-bit.
 * - Runs with clock speed (250MHz assumed).
 */
static void start_timer(
    uint32_t const start_val,
    uint32_t const divider,
    bool const irq_enable,
    bool const busy_wait)
{
    // Example to find out timespan by given values:
    //
    // fcpu      = 250Mhz (assumed)
    // start_val = 250000 (given)
    // divider   = 250    (given)
    //
    // => t = ?
    //
    // fcpu / divider = 250MHz / 250 = 1MHz = 1000000 / 1s = start_val / t
    //
    // => t = start_val * 1s / 1000000 = 250000s / 1000000 = 25s / 100 = 0.25s 
    //
    // t         = 0.25s

    // Example to find out values to setup for a wanted timespan:
    //
    // t         = 100us  (given)
    // fcpu      = 250MHz (assumed)
    // divider   = 250    (given/guessed)
    //
    // => start_val = t * fcpu / divider = 100us * 250MHz / 250
    //
    // start_val = 100

    uint32_t buf;

    disable_timer_and_interrupt();

    buf = mem_read(ARM_TIMER_CTL); // Current state.

    // Set initial value to count down from:
    //
    mem_write(ARM_TIMER_LOD, start_val - 1);

    // Used after count to zero done (not necessary in busy-wait mode):
    //
    mem_write(ARM_TIMER_RLD, start_val - 1);

    mem_write(ARM_TIMER_DIV, divider); // [10 bits are avail. (page 199)].

    armtimer_irq_clear();

    // Will (re-)enable interrupt, if wanted:
    //
    if(irq_enable)
    {
        buf = buf | (1 << 5);
    }

    buf |= (1 << 7); // Will (re-)enable timer.

    buf |= (1 << 1); // Will enable 32-bit mode.

    buf &= 0xFFFFFFF3; // Will disable pre-scaler (run with clock speed).

    mem_write(ARM_TIMER_CTL, buf); // Enables timer.

    if(!busy_wait)
    {
        return;
    }

    while(true) // Polling interrupt flag.
    {
        if(mem_read(ARM_TIMER_RIS) != 0)
        {
            disable_timer_and_interrupt();
            return;
        }
    }
    //assert(false);
}

void armtimer_busywait(uint32_t const start_val, uint32_t const divider)
{
    start_timer(start_val, divider, false, true);
}

void armtimer_start(uint32_t const start_val, uint32_t const divider)
{
    start_timer(start_val, divider, true, false);
}

void armtimer_busywait_microseconds(uint32_t const microseconds)
{
    uint32_t rx;

    // Start free-running counter, if not already running:
    //
    armtimer_start_one_mhz();

    rx = armtimer_get_tick();
    while(true)
    {
        if(armtimer_get_tick() - rx >= microseconds)
        {
            return;
        }
    }
    //assert(false);
}
