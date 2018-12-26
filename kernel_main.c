
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
#include "ui/ui.h"
#include "config.h"
#include "alloc/alloc.h"
#include "tape/tape_init.h"

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

    // Initialize memory (heap) manager for dynamic allocation/deallocation:
    //
    alloc_init(&__heap, MT_HEAP_SIZE);

    // Initialize for tape transfer:
    //
    tape_init();

    // Start user interface (via console):
    //
    ui_enter();
}

// // WORKS:
// //
// // Raspi1, print to UART1 serial out and blink (internal & external LED):
// //
// baregpio_set_output(16, true); // Internal LED (green one).
// //
// miniuart_init();
// //
// baregpio_set_output(2, false); // GPIO pin via expansion port.
// //
// *buf = 0;
// while(true)
// {
//     // Print to UART1:
//     //
//     miniuart_write_byte(0x30 + ((*buf)++));
//
//     *buf = (*buf)%10;
//
//     // Blink:
//     //
//     baregpio_write(16, false);
//     baregpio_write(2, true);
//     busywait_milliseconds(500); // 0.5 seconds.
//     baregpio_write(16, true);
//     baregpio_write(2, false);
//     busywait_microseconds(500000); // Also 0.5 seconds.
//
//     if(*buf==0)
//     {
//         miniuart_write_byte(miniuart_read_byte());
//     }
// }
// return;

//// WORKS:
////
//// Raspi2: Alternatively toggle two LEDs on/off:
//
// volatile uint32_t * const GPFSEL4 = (uint32_t *)0x3F200010;
// volatile uint32_t * const GPFSEL3 = (uint32_t *)0x3F20000C;
// volatile uint32_t * const GPSET1  = (uint32_t *)0x3F200020;
// volatile uint32_t * const GPCLR1  = (uint32_t *)0x3F20002C;
//
// *GPFSEL4 = (*GPFSEL4 & ~(7 << 21)) | (1 << 21);
// *GPFSEL3 = (*GPFSEL3 & ~(7 << 15)) | (1 << 15);
//
// while(true)
// {
//     *GPSET1 = 1 << (47 - 32);
//     *GPCLR1 = 1 << (35 - 32);
//     busywait_clockcycles(0x100000);
//     *GPCLR1 = 1 << (47 - 32);
//     *GPSET1 = 1 << (35 - 32);
//     busywait_clockcycles(0x200000);
// }
