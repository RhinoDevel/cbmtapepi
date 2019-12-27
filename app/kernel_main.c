
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
#ifndef NDEBUG
    #include "../hardware/sdcard/sdcard.h"
#endif //NDEBUG

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
#include "tape/tape_input.h"
#include "petload/petload.h"

#include "cbm/cbm_receive.h"
#include "cbm/cbm_send.h"

#include "cmd/cmd_output.h"
#include "cmd/cmd.h"

// #if PERI_BASE == PERI_BASE_PI1
//     #define VIDEO_SUPPORT 1
// #else //PERI_BASE == PERI_BASE_PI1
//     #define VIDEO_SUPPORT 0
// #endif //PERI_BASE == PERI_BASE_PI1
//
// #if VIDEO_SUPPORT
//     #include "../lib/video/video.h"
// #endif //VIDEO_SUPPORT

extern uint32_t __heap; // See memmap.ld.

enum led_state
{
    led_state_off,
    led_state_on,
    led_state_blink
};

enum load_mode
{
    load_mode_cbm,
    load_mode_pet2
};

static char const * const s_fastload_pet2 = "!pet2";

static enum led_state s_led_state = led_state_off;

/** IRQ interrupt handler.
 */
void __attribute__((interrupt("IRQ"))) handler_irq()
{
    static bool timer_irq_state = false;
    static bool act_state = false;
    static int act_count = 0;

    armtimer_irq_clear();

    timer_irq_state = !timer_irq_state;

    ++act_count;
    if(act_count == 2)
    {
        act_state = !act_state;
        act_count = 0;
        gpio_set_output(GPIO_PIN_NR_ACT, act_state);
    }

    switch(s_led_state)
    {
        case led_state_on:
        {
            gpio_set_output(MT_GPIO_PIN_NR_LED, true);
            break;
        }
        case led_state_off:
        {
            gpio_set_output(MT_GPIO_PIN_NR_LED, false);
            break;
        }

        case led_state_blink: // (falls through)
        default:
        {
            gpio_set_output(MT_GPIO_PIN_NR_LED, timer_irq_state);
            break;
        }
    }
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

    static void cmd_enter()
    {
        enum load_mode lm = load_mode_cbm;

        cmd_reinit(MT_FILESYS_ROOT);
        s_led_state = led_state_on; // Indicates SAVE mode (IRQ).
        while(true)
        {
            // Wait for SAVE (either as control command, or to really save):

            char* name = 0;
            struct cmd_output * o = 0;
            struct tape_input * const ti = cbm_receive(0);

            if(ti == 0)
            {
                console_deb_writeline(
                    "kernel_main : Error: Receive from commodore failed!");

                s_led_state = led_state_blink;
                //
                // Indicates an error occurred, but still in SAVE mode (IRQ).

                continue; // Try again..
            }

            name = tape_input_create_str_from_name(ti);

#ifndef NDEBUG
            console_write("kernel_main : Name from tape input = \"");
            console_write(name);
            console_writeline("\".");
#endif //NDEBUG

            if(str_starts_with(name, s_fastload_pet2))
            {
                struct tape_input * ti = petload_create();

                lm = load_mode_cbm; // (just for clarification)

                armtimer_busywait_microseconds(1 * 1000 * 1000); // 1s

                s_led_state = led_state_off;
                //
                // Indicates LOAD mode (IRQ).

                assert(lm == load_mode_cbm);
                cbm_send_data(ti, 0);

                tape_input_free(ti);
                ti = 0;

                s_led_state = led_state_on; // Indicates SAVE mode (IRQ).

                lm = load_mode_pet2;
            }
            else if(cmd_exec(name, ti, &o))
            {
                if(o != 0)
                {
                    armtimer_busywait_microseconds(1 * 1000 * 1000); // 1s
                    //
                    // Necessary for fast load modes?

                    s_led_state = led_state_off;
                    //
                    // Indicates LOAD mode (IRQ).

                    switch(lm)
                    {
                        case load_mode_cbm:
                        {
                            cbm_send(o->bytes, o->name, o->count, 0);

                            // TODO: Implement correctly:
                            //
#ifndef NDEBUG
                            if(str_are_equal(o->name, "petpi2ba.prg"))
                            {
                                console_writeline(
                                    "kernel_main : Switching to PET 2 fastload..");
                                lm = load_mode_pet2;
                            }
#endif //NDEBUG
                            break;
                        }
                        case load_mode_pet2:
                        {
                            petload_send(o->bytes, o->count);
                            break;
                        }

                        default:
                        {
                            assert(false);
                            break;
                        }
                    }

                }

                s_led_state = led_state_on; // Indicates SAVE mode (IRQ).
            }
            else
            {
                console_deb_writeline("kernel_main : Error: Command exec. failed!");

                s_led_state = led_state_blink;
                //
                // Indicates an error occurred, but still in SAVE mode (IRQ).
            }

            if(o != 0)
            {
                cmd_free_output(o);
            }
            alloc_free(name);
            alloc_free(ti->bytes);
            alloc_free(ti);
        }
    }
#endif //MT_INTERACTIVE

// #if VIDEO_SUPPORT
// #else //VIDEO_SUPPORT
//     static void dummy_write(uint8_t const byte)
//     {
//         (void)byte; // Doing nothing.
//     }
// #endif //VIDEO_SUPPORT

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

    p.write_byte = miniuart_write_byte;
    //
    // If MiniUART is also used for serial transfer, using UART as console
    // may cause serial transfers to fail (e.g. assertions and debug output)!
    //
    p.write_newline_with_cr = true;
    //
// #if VIDEO_SUPPORT
//     p.write_byte = video_write_byte;
// #else //VIDEO_SUPPORT
//     p.write_byte = dummy_write;
// #endif //VIDEO_SUPPORT
//     //
//     p.write_newline_with_cr = false;

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

static void irq_armtimer_init()
{
    irqcontroller_irq_src_enable_armtimer();

    irqcontroller_irq_enable();

    // Timer counts down 250.000 times in one second (with 250 kHz):
    //
    armtimer_start(250000 * 1, 250);
    //
    // 0.25 seconds, hard-coded for 250MHz clock.
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

//     // Does not work for Raspberry Pi 2, yet:
//     //
//     // Initialize video:
//     //
// #if VIDEO_SUPPORT
//     video_init();
// #endif //VIDEO_SUPPORT

#ifndef NDEBUG
    int const result_sdcard_init = sdcard_init();

    if(result_sdcard_init != SD_OK
        && result_sdcard_init != SD_ALREADY_INITIALIZED)
    {
        console_write("kernel_main : Error: SD card init failed with error ");
        console_write_dword_dec(result_sdcard_init);
        console_writeline("!");
    }
#endif //NDEBUG


    // Initialize memory (heap) manager for dynamic allocation/deallocation:
    //
    alloc_init(&__heap, MT_HEAP_SIZE);

    // Initialize for tape transfer:
    //
    tape_init(
        armtimer_start_one_mhz,
        armtimer_get_tick,
        armtimer_busywait_microseconds);

    irq_armtimer_init(); // Needs GPIO.

#ifdef MT_INTERACTIVE
    // Start user interface (via console):
    //
    ui_enter();
#else //MT_INTERACTIVE
    // "File system and SAVE control" mode:
    //
    cmd_enter();
#endif //MT_INTERACTIVE
}
