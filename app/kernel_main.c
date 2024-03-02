
// Marcel Timm, RhinoDevel, 2018dec11

// Pages mentioned in source code comments can be found in the document
// "BCM2835 ARM Peripherals".
//
// Also you MUST USE https://elinux.org/BCM2835_datasheet_errata
// together with that document!

#include <stdint.h>

#include "../hardware/peribase.h"
#include "../hardware/mailbox/mailbox.h"
#include "../hardware/armtimer/armtimer.h"
#include "../hardware/irqcontroller/irqcontroller.h"
#include "../hardware/barrier.h"
#include "../hardware/miniuart/miniuart.h"
//#include "../hardware/pl011uart/pl011uart.h"
#include "../hardware/gpio/gpio_params.h"
#include "../hardware/gpio/gpio.h"
#ifndef NDEBUG
    #include "../hardware/systimer/systimer.h"
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

// #if PERI_BASE == PERI_BASE_PI0AND1
//     #define VIDEO_SUPPORT 1
// #else //PERI_BASE == PERI_BASE_PI0AND1
//     #define VIDEO_SUPPORT 0
// #endif //PERI_BASE == PERI_BASE_PI0AND1
// #if VIDEO_SUPPORT
//     #include "../lib/video/video.h"
// #endif //VIDEO_SUPPORT

extern uint32_t __heap; // See memmap.ld.

void _start(); // See boot.S.

enum led_state
{
    led_state_off = 0,
    led_state_on = 1,
    led_state_blink = 2
};

// For fast mode installer ready-signal detection (it has a frequency):
//
#define s_t_cycle 500 // Microseconds <=> 2kHz fastmode freq.
#define s_per_cycle 5 // Count of measure points per fastmode
                      // installer signal cycle.
#define s_t_measure (s_t_cycle / s_per_cycle) // <=> 10kHz sampling.
#define s_cycles 10 // Count of fastmode cycles for measuring.
#define s_measurements (s_cycles * s_per_cycle) // Count of samples.
#define s_min_toggles 10 // Count of min. level changes.
//
static bool s_measure_signal = false;
//
// Will be intialized in init_signal_stuff():
//
static uint32_t s_toggles[s_measurements];
static uint32_t s_toggle_index = 0;
static uint32_t s_last_signal_val = 0;
static uint32_t s_toggle_count = 0;
static bool s_signal_existed = false;

static enum led_state s_led_state = led_state_off;

static void init_signal_stuff()
{
    s_measure_signal = false; // Disable measuring.

    s_toggle_index = 0;
    s_last_signal_val = 0;
    s_toggle_count = 0;

    s_signal_existed = false;

    // For the debug output, below:
    //
#ifndef NDEBUG
    uint32_t const deb_beg = systimer_get_tick();
#endif //NDEBUG

    for(int i = 0; i < s_measurements; ++i)
    {
        s_toggles[i] = 0;
    }

#ifndef NDEBUG
    uint32_t const deb_end = systimer_get_tick();

    console_write("init_signal_stuff: Loop over ");
    console_write_dword_dec(s_measurements);
    console_write(" items took 0x");
    console_write_dword(deb_end - deb_beg);
    console_writeline(" (1 MHz) ticks.");
#endif //NDEBUG
}

// /**
//  * - To test, if starting secondary cores worked (it works on Raspi 2).
//  * - See commented-out test code in boot.S!
//  */
// void act_led_blink_loop()
// {
//     bool state = false;
//
//     while(true)
//     {
//         barrier_datasync();
//
//         // Does not work for Pi 3 this way (should not even compile):
//         //
//         gpio_set_output(GPIO_PIN_NR_ACT, state);
//
//         armtimer_busywait_microseconds(250000);
//         state = !state;
//
//         barrier_datasync();
//     }
// }

/** IRQ interrupt handler.
 */
void __attribute__((interrupt("IRQ"))) handler_irq()
{
    barrier_datasync();

//     // For the debug output, below:
//     //
// #ifndef NDEBUG
//     static uint32_t deb_last_beg = 0;
//     uint32_t const deb_beg = systimer_get_tick();
//
//     static int const deb_count_lim = 25;
//     static int deb_count = 0;
// #endif //NDEBUG

    static const uint32_t blink_interval = 250000; // 0.25s
    static const uint32_t blink_count = blink_interval / s_t_measure;
    static const uint32_t act_interval = 500000; // 0.5s
    static const uint32_t act_count = act_interval / s_t_measure;

    static bool act_state = false;
    static bool blink_state = false;
    static uint32_t counter = 0;

    armtimer_irq_clear();

    ++counter;

    // It is OK to call gpio_write() from ISR ("atomic" considerations)
    // - see implementation of gpio_write()!

#if PERI_BASE_PI_VER < 3
    act_state ^= counter % act_count == 0;

    gpio_write( // Overdone, but should be OK and to avoid branching.
        GPIO_PIN_NR_ACT, act_state);
#else //PERI_BASE_PI_VER < 3
    bool const new_act_state = act_state ^ (counter % act_count == 0);

    if(new_act_state != act_state)
    {
        act_state = new_act_state;

        // EVIL: Waiting loops (see mailbox.c) take too long and this dirty hack
        //       seems to work:
        //
        //mailbox_write_gpio_actled(act_state); // (ignoring return value)
        //
        static volatile uint32_t msg_buf[8] __attribute__((aligned (16)));
        msg_buf[0] = sizeof *msg_buf * 8;
        msg_buf[1] = 0;
	    msg_buf[2] = 0x00038041;
	    msg_buf[3] = 2 * sizeof *msg_buf;
        msg_buf[4] = 0;
        msg_buf[5] = 130;
	    msg_buf[6] = (uint32_t)act_state;
        msg_buf[7] = 0;
        mem_write(
            PERI_BASE + 0xB880 + 0x20,
            (((0x40000000 + (uint32_t)msg_buf) >> 4) << 4) | 8);
        mem_read(PERI_BASE + 0xB880);
    }
#endif //PERI_BASE_PI_VER < 3

    blink_state ^= counter % blink_count == 0;

    gpio_write( // Overdone, but should be OK and to avoid branching.
        MT_GPIO_PIN_NR_LED, 
        s_led_state == led_state_on
            || (s_led_state == led_state_blink && blink_state));

    // *** SIGNAL DETECTION: ***
    //
    // If there are at least the minimum amount of level changes between the
    // last measurements, set a flag.
    //
    // It is OK to call gpio_read() from ISR ("atomic" considerations)
    // - see implementation of gpio_read()!
    //
    if(s_measure_signal)
    {   
        uint32_t const cur_signal_val = (uint32_t)gpio_read(
                                            MT_TAPE_GPIO_PIN_NR_WRITE);

        uint32_t const is_toggled = s_last_signal_val ^ cur_signal_val;
        
        // Wanted:
        //
        //  s_toggles[s_toggle_index]| is_toggled ||   s_toggle_count
        // -----------------------------------------------------------
        //                         0 |          0 || = s_toggle_count        
        //                         0 |          1 || += 1
        //                         1 |          0 || -= 1
        //                         1 |          1 || = s_toggle_count
        //
        // Implementation:
        //
        // s_toggle_count = s_toggle_count - (s_toggles[s_toggle_index] - is_toggled)
        //
        // E.g. with s_toggle_count == 0xFFFFFFFE:
        //
        // s_toggle_count = 0xFFFFFFFE - (0 - 0) = 0xFFFFFFFE
        // s_toggle_count = 0xFFFFFFFE - (0 - 1) = 0xFFFFFFFE - 0xFFFFFFFF
        //                                     = 0xFFFFFFFF
        // s_toggle_count = 0xFFFFFFFE - (1 - 0) = 0xFFFFFFFD
        // s_toggle_count = 0xFFFFFFFE - (1 - 1) = 0xFFFFFFFE
        //
        s_toggle_count = s_toggle_count - (s_toggles[s_toggle_index] - is_toggled);

        assert(s_toggle_count == 0 || s_toggle_count <= s_measurements);

        s_signal_existed = s_signal_existed || s_toggle_count >= s_min_toggles;

        s_last_signal_val = cur_signal_val;
        s_toggles[s_toggle_index] = is_toggled;
        s_toggle_index = (s_toggle_index + 1) % s_measurements;
    }

//     // Use this via video output and do not output too much to influence
//     // second measurement result (second number output must equal the wanted
//     // interrupt interval in ticks - will be more, if serial console is used).
//     //
// #ifndef NDEBUG
//     if(s_measure_signal)
//     {
//         uint32_t const deb_end = systimer_get_tick();
//
//         if(deb_count < deb_count_lim)
//         {
//             ++deb_count;
//
//             console_write_dword(deb_end - deb_beg);
//             console_write(" ");
//             console_write_dword(deb_beg - deb_last_beg);
//             console_writeline("");
//
//             deb_last_beg = deb_beg;
//         }
//     }
//     else
//     {
//         deb_count = 0;
//     }
// #endif //NDEBUG

    barrier_datasync();
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

    /** Wait for SAVE from CBM (in compatibility mode).
     * 
     *  - Sets LED state to blinking on error.
     *  - Logs error message in debug mode.
     * 
     *  - Caller takes ownership of return value.
     *  - Returns retrieved data or 0 on error.
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
            // Indicates an error occ., but still in 
            // waiting-for-command-from-CBM "mode".
        }
        return ret_val;
    }

    /** Wait for command from Commodore machine (in compatibility or any of the
     *  fast modes).
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
            case mode_type_pet1: // (falls through)
            case mode_type_pet1tom: // (falls through)
            case mode_type_pet2: // (falls through)
            case mode_type_pet2tom: // (falls through)
            case mode_type_pet4: // (falls through)
            case mode_type_pet4tom: // (falls through)
            case mode_type_vic20tom: // (falls through)
            case mode_type_c64tof: // (falls through)
            case mode_type_c64tom:
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
        // Indicates an error occurred, but still in 
        // waiting-for-command-from-CBM "mode".

        // Get CBM out of waiting-for-response mode:
        //
        if(mode == mode_type_pet1 || mode == mode_type_pet1tom
            || mode == mode_type_pet2 || mode == mode_type_pet2tom
            || mode == mode_type_pet4 || mode == mode_type_pet4tom
            || mode == mode_type_vic20tom
            || mode == mode_type_c64tof || mode == mode_type_c64tom)
        {
            petload_send_nop();
        }
    }

    static bool is_mode_supported(enum mode_type mode)
    {
        switch(mode)
        {
            case mode_type_save: // (falls through)
            case mode_type_pet1: // (falls through)
            case mode_type_pet1tom: // (falls through)
            case mode_type_pet2: // (falls through)
            case mode_type_pet2tom: // (falls through)
            case mode_type_pet4: // (falls through)
            case mode_type_pet4tom: // (falls through)
            case mode_type_vic20tom: // (falls through)
            case mode_type_c64tof: // (falls through)
            case mode_type_c64tom:
            {
                return true;
            }

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

        if(str_starts_with(name, "pet1tom"))
        {
            return mode_type_pet1tom;
        }
        if(str_starts_with(name, "pet1"))
        {
            return mode_type_pet1;
        }

        if(str_starts_with(name, "pet2tom"))
        {
            return mode_type_pet2tom;
        }
        if(str_starts_with(name, "pet2"))
        {
            return mode_type_pet2;
        }

        if(str_starts_with(name, "pet4tom"))
        {
            return mode_type_pet4tom;
        }
        if(str_starts_with(name, "pet4"))
        {
            return mode_type_pet4;
        }

        if(str_starts_with(name, "vic20"))
        {
            return mode_type_vic20tom;
        }

        if(str_starts_with(name, "c64tom"))
        {
            return mode_type_c64tom;
        }
        if(str_starts_with(name, "c64"))
        {
            return mode_type_c64tof;
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
    static enum mode_type get_mode_to_use()
    {
        enum mode_type mode = mode_load();
    
        if(is_mode_supported(mode))
        {
            return mode;
        }
        else
        {
            return mode_type_save; // The default (compatibility) mode.
        }
    }

    // This is done by IRQ handler, now:
    //
/*
    [TAPE: Tmin = 352us, Tmax = 672us]
    WRITE: Trise < 2us, Tfall < 4us
    SCNKEY (at VIC 20 via ISR): T60Hz = 16667us, T50Hz = 20000us
    =>
    SIGNAL (from fast mode installer to server):
    Tape does not matter, because there must be no SAVE attempt from CBM during fast
    mode installer sequence.
    => Write << Signal << scnkey.
    => T = ~500us,
    CBM must toggle every ~250us (fast mode installer PRG, the timing is not
    precise, if just counting cycles, because some machines use 50Hz,
    others 60Hz),
    server's ARM timer based ISR can be entered every Tarm = ~500us / 5.

    * TEST, if this ISR-calling does not interfere with compatible mode tape LOAD
        and/or save!

    Each time server's ISR is called it checks, if WRITE got toggled.
    If there are at least x toggles in Tx, the signal is detected.

    Tscnkey >> Tx > T

    E.g.: T    =                          ~500us
            Tarm = T / 5  =                  100us (Tarm >> Trise and Tfall)
            Tx   = 10 * T = 10 * 5 * Tarm = 5000us
            x    =                            10
*/
    ///
//     /** Tries to detect a 50% duty cycle, logic level changing signal on GPIO
//      *  port with given nr.
//      * 
//      * - Takes almost 5ms (depends on values below).
//      *
//      * * TODO: Simplify (see ISR implementation)!
//      */
//     static bool is_signal_detected(uint32_t const gpio_pin_nr)
//     {
//         uint32_t toggles = 0; // Level changes counted in measurement timespan.
//         bool last = gpio_read(gpio_pin_nr); // Store last read logic level.
//
//         for(int i = 1;i < s_measurements; ++i) // (first sample taken above)
//         {
//             armtimer_busywait_microseconds(s_t_measure);
//
//             if(gpio_read(gpio_pin_nr) != last)
//             {
//                 ++toggles;
//                 last = !last;
//             }
//         }
//       
// #ifndef NDEBUG
//         console_write("is_fastmode_installer_waiting : Counted ");
//         console_write_dword_dec(toggles);
//         console_writeline(" level changes.");
// #endif //NDEBUG
//
//         return toggles >= s_min_toggles;
//     }

    static bool did_signal_exist()
    {
        return s_signal_existed;
    }

    /**
     * - Returns false, if fastmode installer ran on CBM.
     */
    static bool send_petload(enum mode_type const mode)
    {
        assert(mode == mode_type_pet1 || mode == mode_type_pet1tom
            || mode == mode_type_pet2 || mode == mode_type_pet2tom
            || mode == mode_type_pet4 || mode == mode_type_pet4tom
            || mode == mode_type_vic20tom
            || mode == mode_type_c64tof || mode == mode_type_c64tom);

        struct tape_input * ti = 0;
    
        switch(mode)
        {
            case mode_type_pet1:
            {
                ti = petload_create_v1();
                break;
            }
            case mode_type_pet1tom:
            {
                ti = petload_create_v1tom();
                break;
            }

            case mode_type_pet2:
            {
                ti = petload_create_v2();
                break;
            }
            case mode_type_pet2tom:
            {
                ti = petload_create_v2tom();
                break;
            }
            
            case mode_type_pet4:
            {
                ti = petload_create_v4();
                break;
            }
            case mode_type_pet4tom:
            {
                ti = petload_create_v4tom();
                break;
            }

            case mode_type_vic20tom:
            {
                ti = petload_create_vic20tom();
                break;
            }

            case mode_type_c64tof:
            {
                ti = petload_create_c64tof();
                break;
            }
            case mode_type_c64tom:
            {
                ti = petload_create_c64tom();
                break;
            }

            default: // Must not happen.
            {
                assert(false);
                break;
            }
        }

        s_led_state = led_state_off; // Indicates sending-to-CBM "mode".
    
        // Kind of bad: False is never interpreted as error, here.
        //              It is assumed that the only reason for this function to
        //              return false is that the fastmode installer signal got
        //              retrieved by the interrupt handler:
        //
        bool const ret_val = cbm_send_data(ti, did_signal_exist);
    
        tape_input_free(ti);
        
        s_led_state = led_state_on; 
        //
        // Indicates waiting-for-command-from-CBM "mode".

        return ret_val;
    }
    static void send_petload_loop(enum mode_type const mode)
    {
        // (a VIC's WRITE line will change between HIGH and LOW infinitely,
        //  because VIA's port used for tape WRITE is used for keyboard scan,
        //  too..)

        // (Re-)start measuring of signal:
        //
        init_signal_stuff();
        s_measure_signal = true;

        while(true)
        {
            if(!send_petload(mode))
            {
                s_measure_signal = false; // Stop measuring.
                console_deb_writeline("send_petload_loop : Breaking loop..");
                return;
            }
            
            console_deb_writeline(
                "send_petload_loop : No signal detected, continuing loop..");

            // If LOAD routine ran on CBM (does not matter, if successful or
            // quit via RUN/STOP keypress) the MOTOR is getting disabled by
            // Commmodore OS, even if SENSE is still LOW.
            //
            // The MOTOR will be restarted by CBM's ISR only after SENSE got
            // toggled from LOW to HIGH and then back again to LOW
            // (with a real tape the user would have to release all buttons
            // and press the play button again).
            //
            // Wait long enough to let CBM detect that SENSE got toggled to
            // HIGH state [done by send_petload() called above]:
            //
            // - 50 Hz machine. => ISR called every 20ms.
            //
            armtimer_busywait_microseconds(10 * 20 * 1000); // 10 * 20ms.
        }
    }

    static void cmd_enter(enum mode_type const mode)
    {
        console_deb_writeline("cmd_enter : Entered function.");

        cmd_reinit(save_mode_by_name, MT_FILESYS_ROOT);

        s_led_state = led_state_on;
        //
        // Indicates waiting-for-command-from-CBM "mode".
        
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

            if(cmd_exec(mode, name, ti, &o))
            {
                if(o != 0) // Something to send back to CBM.
                {
                    s_led_state = led_state_off;
                    //
                    // Indicated sending-data-to-CBM "mode".

                    switch(mode)
                    {
                        case mode_type_save:
                        {
                            armtimer_busywait_microseconds(1 * 1000 * 1000); // 1s

                            cbm_send(o->bytes, o->name, o->count, 0);

                            // How to toggle mode by loading file directly:
                            //
                            // if(str_are_equal(o->name, "petpi4ba.prg"))
                            // {
                            //     console_deb_writeline("cmd_enter : Switching to PET 4 fastload..");
                            //     mode = mode_type_pet4;
                            //     //
                            //     // Wait for motor signal at least to be on its way to LOW
                            //     // (SENSE set to HIGH will trigger CBM OS's IRQ routine to set
                            //     // MOTOR signal to LOW again):
                            //     //
                            //     petload_wait_for_data_ready_val(false, false, 0);
                            // }

                            break;
                        }

                        case mode_type_pet1: // (falls through)
                        case mode_type_pet1tom: // (falls through)
                        case mode_type_pet2: // (falls through)
                        case mode_type_pet2tom: // (falls through)
                        case mode_type_pet4: // (falls through)
                        case mode_type_pet4tom: // (falls through)
                        case mode_type_vic20tom: // (falls through)
                        case mode_type_c64tof: // (falls through)
                        case mode_type_c64tom:
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
                    if(mode == mode_type_pet1 || mode == mode_type_pet1tom
                        || mode == mode_type_pet2 || mode == mode_type_pet2tom
                        || mode == mode_type_pet4 || mode == mode_type_pet4tom
                        || mode == mode_type_vic20tom
                        || mode == mode_type_c64tof || mode == mode_type_c64tom)
                    {
                        // Get CBM out of waiting-for-response mode:
                        //
                        petload_send_nop();
                    }
                }

                s_led_state = led_state_on;
                //
                // Indicates waiting-for-command-from-CBM mode.
            }
            else
            {
                on_failed_cmd(mode); // (sets LED to blinking)
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
    
// #if VIDEO_SUPPORT
//     p.write_byte = video_write_byte;
// #else //VIDEO_SUPPORT
//     p.write_byte = dummy_write;
// #endif //VIDEO_SUPPORT
    
 //   p.write_newline_with_cr = false;

    miniuart_init();

    // // Initialize console via PL011 UARTGPIO_PIN_NR_LED (use this for QEMU):
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
    // Setup LED GPIO ports [to be able to just call gpio_write() from ISR]:
    //
#if PERI_BASE_PI_VER < 3
    gpio_set_func(GPIO_PIN_NR_ACT, gpio_func_output);
#endif //PERI_BASE_PI_VER < 3
    gpio_set_func(MT_GPIO_PIN_NR_LED, gpio_func_output);

    irqcontroller_irq_src_enable_armtimer();

    irqcontroller_irq_enable();

    armtimer_start((uint32_t)s_t_measure, 250);
    //
    assert(s_t_measure == 100);
    //
    // 100us / 10kHz, hard-coded for 250MHz clock [see explanation at function
    // definition of start_timer()].
}

static void init_secondary_cores()
{
#if PERI_BASE != PERI_BASE_PI0AND1
    for(uint32_t core_nr = 1;core_nr <= 3;++core_nr)
    {
        static uint32_t const arm_local_base = 0x40000000; // Hard-coded!
        //
        // ARM_LOCAL_BASE = 0x40000000 for Pi 2 and 3.
        // ARM_LOCAL_BASE = 0xFF800000 for Pi 4.

        // See QA7_rev3.4.pdf, page 6 (Quad-A7 control):
        //
        static uint32_t const arm_local_mailbox3_set0 = arm_local_base + 0x8C;
        static uint32_t const arm_local_mailbox3_clr0 = arm_local_base + 0xCC;
        //
        uint32_t const set_addr = arm_local_mailbox3_set0 + 16 * core_nr;
        uint32_t const clr_addr = arm_local_mailbox3_clr0 + 16 * core_nr;

        barrier_datasync();

        while(mem_read(clr_addr) != 0)
        {
            // No timeout..

            armtimer_busywait_microseconds(1000);
        }

        mem_write(set_addr, (uint32_t)(&_start));

        asm volatile ("sev"); // Send event hint to all CPUs.

        // Wait for CPU to start:

        while(mem_read(clr_addr) != 0)
        {
            // No timeout..

            armtimer_busywait_microseconds(1000);
        }
    }
#endif //PERI_BASE != PERI_BASE_PI0AND1
}

#ifndef NDEBUG

static void print_sys_infos()
{
    uint32_t buf = UINT32_MAX,
        full_mem_mb = 0;

    console_write("print_sys_infos: VideoCore firmware rev.: ");
    console_write_dword_dec(mailbox_read_vcfirmwarerev());
    console_writeline("");

    console_write("print_sys_infos: Board model: ");
    console_write_dword_dec(mailbox_read_boardmodel());
    console_writeline("");

    console_write("print_sys_infos: Board rev.: ");
    console_write_dword_dec(mailbox_read_boardrev());
    console_writeline("");

    console_write("print_sys_infos: ARM memory base address: 0x");
    console_write_dword(mailbox_read_armmemory(&buf));
    console_writeline("");
    console_write("print_sys_infos: ARM memory byte count: ");
    console_write_dword_dec(buf);
    console_write(" / ");
    buf = buf / 1024 / 1024;
    full_mem_mb += buf;
    console_write_dword_dec(buf);
    console_writeline(" MB");

    console_write("print_sys_infos: VideoCore memory base address: 0x");
    console_write_dword(mailbox_read_vcmemory(&buf));
    console_writeline("");
    console_write("print_sys_infos: VideoCore memory byte count: ");
    console_write_dword_dec(buf);
    console_write(" / ");
    buf = buf / 1024 / 1024;
    full_mem_mb += buf;
    console_write_dword_dec(buf);
    console_writeline(" MB");

    console_write("print_sys_infos: Full memory count: ");
    console_write_dword_dec(full_mem_mb);
    console_writeline(" MB");

    console_write("print_sys_infos: Current/max. SoC temperature: ");
    console_write_dword_dec(mailbox_read_soctemp());
    console_write(" / ");
    console_write_dword_dec(mailbox_read_socmaxtemp());
    console_writeline(" (1/1000 degree celsius)");

    // Must be at 250MHz (and stay there..!) as long as we are using the ARM
    // timer(-s) instead of the system timer:
    //
    console_write("print_sys_infos: Core min./max./current clockrates: ");
    console_write_dword_dec(mailbox_read_minclockrate(mailbox_id_clockrate_core));
    console_write(" / ");
    console_write_dword_dec(mailbox_read_maxclockrate(mailbox_id_clockrate_core));
    console_write(" / ");
    console_write_dword_dec(mailbox_read_clockrate(mailbox_id_clockrate_core));
    console_writeline("");

    console_write("print_sys_infos: ARM min./max./current clockrates: ");
    console_write_dword_dec(mailbox_read_minclockrate(mailbox_id_clockrate_arm));
    console_write(" / ");
    console_write_dword_dec(mailbox_read_maxclockrate(mailbox_id_clockrate_arm));
    console_write(" / ");
    console_write_dword_dec(mailbox_read_clockrate(mailbox_id_clockrate_arm));
    console_writeline("");

    console_write("print_sys_infos: EMMC min./max./current clockrates: ");
    console_write_dword_dec(mailbox_read_minclockrate(mailbox_id_clockrate_emmc));
    console_write(" / ");
    console_write_dword_dec(mailbox_read_maxclockrate(mailbox_id_clockrate_emmc));
    console_write(" / ");
    console_write_dword_dec(mailbox_read_clockrate(mailbox_id_clockrate_emmc));
    console_writeline("");
}

#endif //NDEBUG

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

    armtimer_start_one_mhz(); // (not sure, if necessary, here..)

    // Initialize for tape transfer:
    //
    tape_init(
        armtimer_start_one_mhz,
        armtimer_get_tick,
        armtimer_busywait_microseconds);

    init_signal_stuff();

    irq_armtimer_init(); // Needs GPIO, initializes LED GPIOs, too.
                         // Also needs tape_init() and init_signal_stuff()
                         // to already have been called.

    init_secondary_cores();

#ifndef NDEBUG
    print_sys_infos();
#endif //NDEBUG

#ifdef MT_INTERACTIVE
    // Start user interface (via console):
    //
    ui_enter();
#else //MT_INTERACTIVE
    {
        enum mode_type mode = get_mode_to_use();

#ifndef NDEBUG
        console_write("kernel_main : Mode to use has value ");
        console_write_byte_dec((uint8_t)mode);
        console_writeline(".");
#endif //NDEBUG
        if(mode == mode_type_pet1 || mode == mode_type_pet1tom
            || mode == mode_type_pet2 || mode == mode_type_pet2tom
            || mode == mode_type_pet4 || mode == mode_type_pet4tom
            || mode == mode_type_vic20tom
            || mode == mode_type_c64tof || mode == mode_type_c64tom)
        {
            // Wait some time for user to power-on CBM and push
            // <shift> + <run/stop> for loading:
            //
            armtimer_busywait_microseconds(5 * 1000 * 1000); // 5s

            send_petload_loop(mode);
        }

        // "File system and control" mode:
        //
        cmd_enter(mode);
    }
#endif //MT_INTERACTIVE
}
