
// Marcel Timm, RhinoDevel, 2018dec11

// Pages mentioned in source code comments can be found in the document
// "BCM2835 ARM Peripherals".
//
// Also you MUST USE https://elinux.org/BCM2835_datasheet_errata
// together with that document!

#include <stdint.h>

#include "miniuart/miniuart.h"
//#include "pl011uart/pl011uart.h"

#include "console/console.h"
#include "ui/ui_terminal_to_commodore.h"
#include "ui/ui_commodore_to_terminal.h"
#include "ui/ui.h"
#include "config.h"
#include "alloc/alloc.h"
#include "tape/tape_init.h"
#include "statetoggle/statetoggle.h"
#include "video/video.h"

extern uint32_t __heap; // See memmap.ld.

/** Connect console (singleton) to wanted in-/output.
 */
static void init_console()
{
    struct console_params p;

    // Initialize console via MiniUART:
    //
    p.read_byte = miniuart_read_byte;
    p.write_byte = miniuart_write_byte;
    miniuart_init();
    //
    // // Initialize console via PL011 UART (also use this for QEMU):
    // //
    // p.read_byte = pl011uart_read_byte;
    // p.write_byte = pl011uart_write_byte;
    // pl011uart_init(); // (don't do this while using QEMU)

    console_init(&p);
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

    // Initialize console in-/output:
    //
    init_console();

    // Initialize video (test):
    //
    video_init();

    // Initialize memory (heap) manager for dynamic allocation/deallocation:
    //
    alloc_init(&__heap, MT_HEAP_SIZE);

    // Initialize for tape transfer:
    //
    tape_init();

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
            ui_commodore_to_terminal(false);
        }
        else
        {
            ui_terminal_to_commodore(false);
        }

        if(statetoggle_is_requested())
        {
            statetoggle_toggle();
        }
    }
#endif //MT_INTERACTIVE
}
