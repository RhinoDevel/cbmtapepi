
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

#include "mode/mode.h"
#include "mode/mode_type.h"

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

    /** Wait for SAVE (either as ctrl. cmd., or to really save).
     * 
     *  - Sets LED state to blinking on error.
     *  - Logs error message in debug mode.
     * 
     *  - Caller takes ownership of return value.
     *  - Returns 0 on error.
     */
    static struct tape_input * wait_for_save()
    {
        struct tape_input * const ret_val = cbm_receive(0);

        if(ret_val == 0)
        {
            console_deb_writeline(
                "wait_for_save : Error: Receive from Commodore failed!");

            s_led_state = led_state_blink;
            //
            // Indicates an error occ., but still in SAVE mode (IRQ).
        }
        return ret_val;
    }

    /** Wait for command from Commodore machine.
     * 
     *  - Caller takes ownership of return value.
     *  - Returns 0 on error.
     */
    static struct tape_input * wait_for_cbm(enum mode_type const mode)
    {
        switch(mode)
        {
            case mode_type_save:
            {
                return wait_for_save();
            }
            case mode_type_pet2: // (falls through)
            case mode_type_pet4:
            {
                return petload_retrieve(); // (must never return 0)
            }

            default:
            {
                assert(false);
                return 0;
            }
        }
    }

    static void on_failed_cmd(enum mode_type const mode)
    {
        console_deb_writeline("on_failed_cmd : Error: Command exec. failed!");

        s_led_state = led_state_blink;
        //
        // Indicates an error occurred, but still in SAVE mode (IRQ).

        // Get CBM out of waiting-for-response mode:
        //
        if(mode == mode_type_pet2 || mode == mode_type_pet4)
        {
            petload_send_nop();
        }
    }

    static bool is_mode_supported(enum mode_type mode)
    {
        switch(mode)
        {
            case mode_type_save: // (falls through)
            case mode_type_pet4:
            {
                return true;
            }

            case mode_type_pet2: // (falls through) // (not supported, yet)
            //
            case mode_type_err: // (falls through)
            default:
            {
                return false;
            }
        }
    }

    /**
     * - Does not take ownership of given object.
     */
    static enum mode_type get_mode(char const * const name)
    {
        if(str_starts_with(name, "save"))
        {
            return mode_type_save;
        }
        if(str_starts_with(name, "pet2"))
        {
            return mode_type_pet2;
        }
        if(str_starts_with(name, "pet4"))
        {
            return mode_type_pet4;
        }
        return mode_type_err;
    }
    static bool save_mode(enum mode_type const mode)
    {
        if(!is_mode_supported(mode))
        {
            return false;
        }
        return mode_save(mode);
    }
    static bool save_mode_by_name(char const * const name)
    {
        return save_mode(get_mode(name));
    }
    // static enum mode_type get_mode_to_use()
    // {
    //     enum mode_type mode = mode_load();
    //
    //     if(is_mode_supported(mode))
    //     {
    //         return mode;
    //     }
    //     else
    //     {
    //         return mode_type_save; // The default mode.
    //     }
    // }

    static void cmd_enter()
    {
        enum mode_type mode = mode_type_save;//get_mode_to_use(); // TODO: Implement correct usage before enabling this!

        cmd_reinit(save_mode_by_name, MT_FILESYS_ROOT);
        s_led_state = led_state_on; // Indicates SAVE mode (IRQ).
        while(true)
        {
            struct cmd_output * o = 0;

            // Get command (or PRG to save) from Commodore machine:

            struct tape_input * const ti = wait_for_cbm(mode);

            if(ti == 0)
            {
                assert(mode == mode_type_save);
                continue; // Try again..
            }

            // Create "command" string to react accordingly:

            char * const name = tape_input_create_str_from_name(ti);

#ifndef NDEBUG
            console_write("cmd_enter : Name from tape input = \"");
            console_write(name);
            console_writeline("\".");
#endif //NDEBUG

            // if(mode == mode_type_save && str_starts_with(name, "!pet4"))
            // {
            //     struct tape_input * ti_create = petload_create();
            //
            //     armtimer_busywait_microseconds(1 * 1000 * 1000); // 1s
            //
            //     s_led_state = led_state_off; // Indicates LOAD mode (IRQ).
            //
            //     cbm_send_data(ti_create, 0);
            //
            //     console_deb_writeline("cmd_enter : Switching to PET 4 fastload..");
            //     mode = mode_type_pet4;
            //     //
            //     // Wait for motor signal at least to be on its way to LOW
            //     // (SENSE set to HIGH will trigger CBM OS's IRQ routine to set
            //     // MOTOR signal to LOW again):
            //     //
            //     petload_wait_for_data_ready_val(false, false);
            //
            //     tape_input_free(ti_create);
            //     ti_create = 0;
            //
            //     s_led_state = led_state_on; // Indicates SAVE mode (IRQ).
            // }
            // else 
            //
            if(cmd_exec(name, ti, &o))
            {
                if(o != 0) // Something to send back to CBM.
                {
                    s_led_state = led_state_off; // Indicates LOAD mode (IRQ).

                    switch(mode)
                    {
                        case mode_type_save:
                        {
                            armtimer_busywait_microseconds(1 * 1000 * 1000); // 1s

                            cbm_send(o->bytes, o->name, o->count, 0);

                            // TODO: Implement correctly:
                            //
                            if(str_are_equal(o->name, "petpi4ba.prg"))
                            {
                                console_deb_writeline("cmd_enter : Switching to PET 4 fastload..");
                                mode = mode_type_pet4;
                                //
                                // Wait for motor signal at least to be on its way to LOW
                                // (SENSE set to HIGH will trigger CBM OS's IRQ routine to set
                                // MOTOR signal to LOW again):
                                //
                                petload_wait_for_data_ready_val(false, false);
                            }

                            break;
                        }
                        case mode_type_pet4:
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
                else // Nothing to send back to CBM.
                {
                    if(mode == mode_type_pet4 || mode == mode_type_pet2)
                    {
                        // Get CBM out of waiting-for-response mode:
                        //
                        petload_send_nop();
                    }
                }

                s_led_state = led_state_on; // Indicates SAVE mode (IRQ).
            }
            else
            {
                on_failed_cmd(mode);
            }

            // Deallocate memory:

            cmd_free_output(o);
            alloc_free(name);
            if(ti != 0)
            {
                alloc_free(ti->bytes);
                alloc_free(ti);
            }
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
    // "File system and control" mode:
    //
    cmd_enter();
#endif //MT_INTERACTIVE
}
