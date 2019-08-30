
// Marcel Timm, RhinoDevel, 2018dec11

// Pages mentioned in source code comments can be found in the document
// "BCM2835 ARM Peripherals".
//
// Also you MUST USE https://elinux.org/BCM2835_datasheet_errata
// together with that document!

#include <stdint.h>

#include "../hardware/peribase.h"
#include "../hardware/armtimer/armtimer.h"

#include "../hardware/miniuart/miniuart.h"
//#include "../hardware/pl011uart/pl011uart.h"

#include "../hardware/baregpio/baregpio_params.h"
#include "../hardware/baregpio/baregpio.h"

#include "../lib/console/console.h"
#include "ui/ui_terminal_to_commodore.h"
#include "ui/ui_commodore_to_terminal.h"
#include "ui/ui.h"
#include "config.h"
#include "../lib/alloc/alloc.h"
#include "../lib/assert.h"
#include "../lib/mem/mem.h"
#include "tape/tape_init.h"
#include "statetoggle/statetoggle.h"
//#include "../lib/video/video.h"

extern uint32_t __heap; // See memmap.ld.

#ifndef MT_INTERACTIVE
/**
 * - Must not be called. Just for error handling..
 */
static uint8_t dummy_read()
{
    assert(false);

    return 0;
}
#endif //MT_INTERACTIVE

static void dummy_write(uint8_t const byte)
{
    (void)byte; // Doing nothing.
}

/** Connect console (singleton) to wanted in-/output.
 */
static void init_console()
{
    struct console_params p;

    // Initialize console via MiniUART to read and video to write:

    // No read in non-interactive mode:
    //
#ifdef MT_INTERACTIVE
    p.read_byte = miniuart_read_byte;
#else //MT_INTERACTIVE
    p.read_byte = dummy_read;
#endif //MT_INTERACTIVE

    // p.write_byte = miniuart_write_byte;
    // //
    // // If MiniUART is also used for serial transfer, using UART as console
    // // may cause serial transfers to fail (e.g. assertions and debug output)!
    // //
    // p.write_newline_with_cr = true;
    //
    p.write_byte = dummy_write/*video_write_byte*/;
    p.write_newline_with_cr = false;

    miniuart_init();

    // // Initialize console via PL011 UART (use this for QEMU):
    //
    // p.read_byte = pl011uart_read_byte;
    //
    // p.write_byte = pl011uart_write_byte;
    // //
    // // If PL011 UART is also used for serial transfer, using UART as console
    // // may cause serial transfers to fail (e.g. assertions and debug output)!
    // //
    // p.write_newline_with_cr = true;
    //
    // pl011uart_init(); // (don't do this while using QEMU)

    console_init(&p);
}

static void toggle_gpio(
    uint32_t const pin_nr,
    int const count,
    uint32_t const milliseconds,
    bool const is_high)
{
    uint32_t const microseconds = 1000 * milliseconds;
    int i = 0;

    while(true)
    {
        baregpio_set_output(pin_nr, !is_high);
        armtimer_busywait_microseconds(microseconds);
        baregpio_set_output(pin_nr, is_high);

        ++i;
        if(i == count)
        {
            return;
        }

        armtimer_busywait_microseconds(microseconds);
    }
}

/**
 * - Entry point (see boot.S).
 */
void kernel_main(uint32_t r0, uint32_t r1, uint32_t r2)
{
    // To be able to compile, although these are not used (stupid?):
    //
    (void)r0;
    (void)r1;
    (void)r2;

    // Initialize bare GPIO singleton:
    //
    baregpio_init((struct baregpio_params){
        .wait_microseconds = armtimer_busywait_microseconds,

        .mem_read = mem_read,
        .mem_write = mem_write,

        .peri_base = PERI_BASE
    });

    // Initialize console in-/output:
    //
    init_console();

    // Does not work for Raspberry Pi 2, yet:
    //
    // // Initialize video:
    // //
    // video_init();

    // Initialize memory (heap) manager for dynamic allocation/deallocation:
    //
    alloc_init(&__heap, MT_HEAP_SIZE);

    // Initialize for tape transfer:
    //
    tape_init(
        armtimer_start_one_mhz,
        armtimer_get_tick,
        armtimer_busywait_microseconds);

    statetoggle_init(MT_GPIO_PIN_NR_BUTTON, MT_GPIO_PIN_NR_LED, false);

#ifdef MT_INTERACTIVE
    // Start user interface (via console):
    //
    ui_enter();
#else //MT_INTERACTIVE
    while(true)
    {
        if(statetoggle_get_state())
        {
            toggle_gpio(MT_GPIO_PIN_NR_LED, 3, 200, true); // Hard-coded

            ui_commodore_to_terminal(false); // (return value ignored)
        }
        else
        {
            toggle_gpio(MT_GPIO_PIN_NR_LED, 3, 200, false); // Hard-coded

            ui_terminal_to_commodore(false); // (return value ignored)
        }

        if(statetoggle_is_requested())
        {
            statetoggle_toggle();
        }
    }
#endif //MT_INTERACTIVE
}
