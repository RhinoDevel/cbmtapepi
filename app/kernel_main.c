
// Marcel Timm, RhinoDevel, 2018dec11

// Pages mentioned in source code comments can be found in the document
// "BCM2835 ARM Peripherals".
//
// Also you MUST USE https://elinux.org/BCM2835_datasheet_errata
// together with that document!

#include <stdint.h>

#include "../hardware/peribase.h"
#include "../hardware/armtimer/armtimer.h"
#include "../hardware/irqcontroller/irqcontroller.h"
#include "../hardware/miniuart/miniuart.h"
//#include "../hardware/pl011uart/pl011uart.h"
#include "../hardware/gpio/gpio_params.h"
#include "../hardware/gpio/gpio.h"

#include "ui/ui.h"

#include "config.h"

#include "../lib/console/console.h"
#include "../lib/alloc/alloc.h"
#include "../lib/assert.h"
#include "../lib/mem/mem.h"
#include "../lib/basic/basic.h"
#include "../lib/basic/basic_addr.h"
#include "../lib/ff14/source/ff.h"
#include "../lib/dir/dir.h"
#include "../lib/filesys/filesys.h"
#include "../lib/str/str.h"

#include "tape/tape_init.h"

#include "cbm/cbm_receive.h"
#include "cbm/cbm_send.h"

#include "cmd/cmd_output.h"
#include "cmd/cmd.h"

#if PERI_BASE == PERI_BASE_PI1
    #define VIDEO_SUPPORT 1
#else //PERI_BASE == PERI_BASE_PI1
    #define VIDEO_SUPPORT 0
#endif //PERI_BASE == PERI_BASE_PI1

#if VIDEO_SUPPORT
    #include "../lib/video/video.h"
#endif //VIDEO_SUPPORT

extern void _enable_interrupts(); // See boot.S.

extern uint32_t __heap; // See memmap.ld.

static bool s_timer_irq_state = false;

/** IRQ interrupt handler.
 */
void __attribute__((interrupt("IRQ"))) handler_irq()
{
    armtimer_irq_clear();

    s_timer_irq_state = !s_timer_irq_state;

    gpio_set_output(GPIO_PIN_NR_ACT, s_timer_irq_state);
}

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

#if VIDEO_SUPPORT
#else //VIDEO_SUPPORT
    static void dummy_write(uint8_t const byte)
    {
        (void)byte; // Doing nothing.
    }
#endif //VIDEO_SUPPORT

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
#if VIDEO_SUPPORT
    p.write_byte = video_write_byte;
#else //VIDEO_SUPPORT
    p.write_byte = dummy_write;
#endif //VIDEO_SUPPORT
    //
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

    // Initialize GPIO singleton:
    //
    gpio_init((struct gpio_params){
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
    // Initialize video:
    //
#if VIDEO_SUPPORT
    video_init();
#endif //VIDEO_SUPPORT

    // Initialize memory (heap) manager for dynamic allocation/deallocation:
    //
    alloc_init(&__heap, MT_HEAP_SIZE);

    // Initialize for tape transfer:
    //
    tape_init(
        armtimer_start_one_mhz,
        armtimer_get_tick,
        armtimer_busywait_microseconds);

    irqcontroller_irq_src_enable_armtimer();
    _enable_interrupts();
    //
    // Timer counts down 250.000 times in one second (with 250 kHz):
    //
    armtimer_start(250000 * 1, 500); // 0.5 seconds, hard-coded for 250MHz clock.

#ifdef MT_INTERACTIVE
    // Start user interface (via console):
    //
    ui_enter();
#else //MT_INTERACTIVE
    // "File system and SAVE control" mode:
    //
    cmd_reinit("/");
    while(true)
    {
        // Wait for SAVE (either as control command, or to really save):

        char* name = 0;
        struct cmd_output * o = 0;

        gpio_set_output(MT_GPIO_PIN_NR_LED, true); // SAVE mode.
        //
        struct tape_input * const ti = cbm_receive(0);

        if(ti == 0)
        {
            console_deb_writeline(
                "kernel_main : Error: Receive from commodore failed!");
            continue; // Try again..
        }

        name = tape_input_create_str_from_name(ti);

#ifndef NDEBUG
        console_write("kernel_main : Name from tape input = \"");
        console_write(name);
        console_write("\" / ");
        console_write_bytes((uint8_t const *)name, str_get_len(name));
        console_writeline(".");
#endif //NDEBUG

        if(cmd_exec(name, ti, &o))
        {
            if(o != 0)
            {
                armtimer_busywait_microseconds(1 * 1000 * 1000); // 1s
                gpio_set_output(MT_GPIO_PIN_NR_LED, false); // LOAD mode.
                cbm_send(o->bytes, o->name, o->count, 0);
            }
        }
        else
        {
            console_deb_writeline("kernel_main : Error: Command exec. failed!");

            // TODO: Implement blinking as error indicator (also above)!
        }

        if(o != 0)
        {
            cmd_free_output(o);
        }
        alloc_free(name);
        alloc_free(ti->bytes);
        alloc_free(ti);
    }
#endif //MT_INTERACTIVE
}
