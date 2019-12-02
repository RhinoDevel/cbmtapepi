
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

#include "../lib/console/console.h"
#include "ui/ui_terminal_to_commodore.h"
#include "ui/ui_commodore_to_terminal.h"
#include "ui/ui.h"
#include "config.h"
#include "../lib/alloc/alloc.h"
#include "../lib/assert.h"
#include "../lib/mem/mem.h"
#include "../lib/basic/basic.h"
#include "../lib/basic/basic_addr.h"
#include "tape/tape_init.h"
#include "statetoggle/statetoggle.h"

#include "cbm/cbm_receive.h"
#include "cbm/cbm_send.h"

#if PERI_BASE == PERI_BASE_PI1
    #define VIDEO_SUPPORT 1
#else //PERI_BASE == PERI_BASE_PI1
    #define VIDEO_SUPPORT 0
#endif //PERI_BASE == PERI_BASE_PI1

#if VIDEO_SUPPORT
    #include "../lib/video/video.h"
#endif //VIDEO_SUPPORT

#ifndef NDEBUG
    #include "../lib/ff14/source/ff.h"
    #include "../lib/dir/dir.h"
    #include "../lib/str/str.h"
#endif //NDEBUG

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
            gpio_set_output(pin_nr, !is_high);
            armtimer_busywait_microseconds(microseconds);
            gpio_set_output(pin_nr, is_high);

            ++i;
            if(i == count)
            {
                return;
            }

            armtimer_busywait_microseconds(microseconds);
        }
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

#ifndef NDEBUG
    static void ff_test()
    {
        FATFS fatfs;
        FIL f;
        UINT buf_uint;
        FRESULT res;
        struct dir_entry * * entry_arr = 0;
        int entry_count = -1;

        if(f_mount(&fatfs, "", 0) != FR_OK)
        {
            console_writeline("ff_test : Error: Mount failed!");
            return;
        }

        if(!dir_reinit("/"))
        {
            console_writeline("ff_test : Error: dir singleton init failed!");
            return;
        }

        console_write("ff_test : Folder name   : \"");
        console_write(dir_get_dir_path());
        console_writeline("\".");

        entry_arr = dir_create_entry_arr(&entry_count);
        if(entry_count == -1)
        {
            console_writeline("ff_test : Error: Entry array creation failed!");
            return;
        }

        if(entry_count == 0)
        {
            console_writeline("ff_test : No entries found.");
        }
        else
        {
            for(int i = 0;i < entry_count; ++i)
            {
                console_write(entry_arr[i]->is_dir
                    ? "ff_test : Subfolder name: \""
                    : "ff_test : File name     : \"");
                console_write(entry_arr[i]->name);
                console_writeline("\".");

                if(mem_cmp_byte(
                    (uint8_t const *)entry_arr[i]->name,
                    (uint8_t const *)"RHINODEV.TXT",
                    12 + 1))
                {
                    if(f_unlink("/RHINODEV.TXT") != FR_OK)
                    {
                        console_writeline(
                            "ff_test : Error: Failed to remove file!");
                        return;
                    }
                }
                else if(mem_cmp_byte(
                    (uint8_t const *)entry_arr[i]->name,
                    (uint8_t const *)"CMDLINE.TXT",
                    11 + 1))
                {
                    char stupid_buf[1024];
                    UINT bytes_read;
                    if(f_open(&f, "/CMDLINE.TXT", FA_READ) != FR_OK)
                    {
                        console_writeline("ff_test : Error: Opening file failed!");
                        return;
                    }

                    res = f_read(&f, stupid_buf, 1024, &bytes_read);
                    if(res!= FR_OK)
                    {
                        console_write(
                            "ff_test : Error: Reading from file failed (");
                        console_write_dword_dec((uint32_t)res);
                        console_writeline(")!");
                        return;
                    }
                    if(f_close(&f) != FR_OK)
                    {
                        console_writeline(
                            "ff_test : Error: Closing file failed (1)!");
                        return;
                    }

                    stupid_buf[bytes_read] = '\0';

                    console_write("ff_test : File content: \"");
                    console_write(stupid_buf);
                    console_writeline("\".");
                }
            }

            dir_free_entry_arr(entry_arr, entry_count);
            entry_arr = 0;
            entry_count = -1;
        }

        if(!dir_deinit())
        {
            console_writeline("ff_test : Error: dir singleton deinit. failed!");
            return;
        }

        res = f_open(&f, "/RHINODEV.TXT", FA_CREATE_NEW | FA_WRITE);
        if(res!= FR_OK)
        {
            console_write("ff_test : Error: Creating file failed (");
            console_write_dword_dec((uint32_t)res);
            console_writeline(")!");
            return;
        }

        res = f_write(&f, "RhinoDevel was here.", 20, &buf_uint);
        if(res!= FR_OK)
        {
            console_write(
                "ff_test : Error: Writing to file failed (");
            console_write_dword_dec((uint32_t)res);
            console_writeline(")!");
            return;
        }

        if(f_close(&f) != FR_OK)
        {
            console_writeline("ff_test : Error: Closing file failed (2)!");
            return;
        }

        console_write("ff_test : fs_type = 0x");
        console_write_byte(fatfs.fs_type);
        console_writeline(".");
        console_write("ff_test : n_rootdir = 0x");
        console_write_word(fatfs.n_rootdir);
        console_writeline(".");

        if(f_mount(0, "", 0) != FR_OK)
        {
            console_writeline("ff_test : Error: Unmounting failed!");
            return;
        }
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

#ifndef NDEBUG
    ff_test();

    // uint32_t const sdcard_init_result = (uint32_t)sdcard_init();
    //
    // console_write("SD card init result is: 0x");
    // console_write_dword(sdcard_init_result);
    // console_writeline(".");
    //
    // sdcard_test();
#endif //NDEBUG

#ifdef MT_INTERACTIVE
    // Start user interface (via console):
    //
    ui_enter();
#else //MT_INTERACTIVE
    // "YMODEM plus toggle button" mode:
    //
    statetoggle_init(MT_GPIO_PIN_NR_BUTTON, MT_GPIO_PIN_NR_LED, false);
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

    // TODO: Implement correctly:
    //
    // "File system and SAVE control" mode:
    //
    // // TODO: Fit state toggle (and cancel by user) stuff to this mode!
    // //
    // statetoggle_init(MT_GPIO_PIN_NR_BUTTON, MT_GPIO_PIN_NR_LED, true);
    // while(true)
    // {
    //     // Wait for SAVE (either as control command, or to really save):
    //
    //     static char const * const cur_dir_path = "/"; // TODO: Make dynamic via "cd"!
    //
    //     struct tape_input * const ti = cbm_receive(0);
    //     char* name = 0;
    //
    //     if(ti == 0)
    //     {
    //         console_deb_writeline("kernel_main : Error: Receive from commodore failed!");
    //         continue; // Try again..
    //     }
    //
    //     name = tape_input_create_str_from_name(ti);
    //
    //     console_write("kernel_main : Name from tape input = \"");
    //     console_write(name);
    //     console_write("\" / ");
    //     console_write_bytes((uint8_t const *)name, str_get_len(name));
    //     console_writeline(".");
    //
    //     // Decide, what to do, based on name given:
    //
    //     // - 16 characters available
    //     // - File system support (by choice) limited to 8.3 format.
    //     // => 16 - 8 - 1 - 3 = 4 characters available for commands.
    //     //
    //     //                                      "   THEGREAT.PRG "
    //     static char const * const cmd_ls   =    "LS"; // (no parameters)
    //     // static char const * const cmd_cd   =    "cd "; // Supports "..", too.
    //     static char const * const cmd_rm   =    "RM ";
    //     // static char const * const cmd_cp   =    "cp "; // Outp. file name by Pi.
    //     // static char const * const cmd_mv   =    "mv "; // New file name by Pi.
    //     static char const * const cmd_load =    "./"; // (no space)
    //     // Anything else. => Really save file.
    //
    //     // Toggle mode and wait for 3 seconds:
    //     //
    //     statetoggle_toggle();
    //     toggle_gpio(MT_GPIO_PIN_NR_LED, 6, 500, false); // Hard-coded
    //
    //     if(str_starts_with(name, cmd_ls))
    //     {
    //         FATFS fatfs;
    //
    //         f_mount(&fatfs, "", 0);
    //         dir_reinit("/");
    //
    //         char* name = "/";
    //         int entry_count = -1;
    //         uint32_t len = 0;
    //         struct dir_entry * * const entry_arr =
    //             dir_create_entry_arr(&entry_count);
    //
    //         dir_deinit();
    //         f_mount(0, "", 0);
    //
    //         char const * * const name_arr = alloc_alloc(
    //             entry_count * sizeof *name_arr);
    //
    //         for(int i = 0;i < entry_count;++i)
    //         {
    //             name_arr[i] = entry_arr[i]->name;
    //         }
    //
    //         uint8_t * const bytes = basic_get_prints(
    //             MT_BASIC_ADDR_C64, name_arr, entry_count, &len);
    //
    //         cbm_send(bytes, name, len, 0); // Return value ignored.
    //
    //         alloc_free(name_arr);
    //         alloc_free(bytes);
    //         dir_free_entry_arr(entry_arr, entry_count);
    //
    //         statetoggle_toggle();
    //         toggle_gpio(MT_GPIO_PIN_NR_LED, 3, 200, true); // Hard-coded
    //     }
    //     else if(str_starts_with(name, cmd_load))
    //     {
    //         FATFS fatfs;
    //         FIL fil;
    //
    //         f_mount(&fatfs, "", 0);
    //
    //         char const * const name_only = name + str_get_len(cmd_load);
    //
    //         if(f_open(&fil, name_only, FA_READ) == FR_OK)
    //         {
    //             uint32_t const len = (uint32_t)f_size(&fil);
    //             uint8_t * bytes = alloc_alloc(len * sizeof *bytes);
    //             UINT read_len = 0;
    //
    //             f_read(&fil, bytes, (UINT)len, &read_len); // No error check!
    //             f_close(&fil); // No error check.
    //
    //             cbm_send(bytes, name_only, len, 0); // Return value ignored.
    //
    //             alloc_free(bytes);
    //
    //             statetoggle_toggle();
    //             toggle_gpio(MT_GPIO_PIN_NR_LED, 3, 200, true); // Hard-coded
    //         }
    //         //
    //         // Otherwise: TODO: Error handling (message?).
    //
    //         f_mount(0, "", 0);
    //     }
    //     else if(str_starts_with(name, cmd_rm))
    //     {
    //         FATFS fatfs;
    //         char const * const name_only = name + str_get_len(cmd_rm);
    //         char* full_path = str_create_concat(cur_dir_path, name_only);
    //
    //         f_mount(&fatfs, "", 1);
    //
    //         f_unlink(full_path);
    //
    //         alloc_free(full_path);
    //         full_path = 0;
    //
    //         statetoggle_toggle();
    //         toggle_gpio(MT_GPIO_PIN_NR_LED, 3, 200, true); // Hard-coded
    //
    //         f_mount(0, "", 0);
    //     }
    //     //
    //     // TODO: Add more commands, here.
    //     //
    //     else
    //     {
    //         // TODO: FIX, not working, yet:
    //
    //         // Really save to file:
    //
    //         do
    //         {
    //             FRESULT res;
    //             FATFS fatfs;
    //             FIL fil;
    //             UINT write_count;
    //             uint8_t buf;
    //
    //             res = f_mount(&fatfs, "", 1);
    //             if(res != FR_OK)
    //             {
    //                 console_write("SAVE: Mounting failed (");
    //                 console_write_dword_dec((uint32_t)res);
    //                 console_writeline(")!");
    //                 break;
    //             }
    //
    //             res = f_open(
    //                 &fil, name, FA_CREATE_NEW | FA_WRITE); // No overwrite.
    //             if(res != FR_OK)
    //             {
    //                 console_write("SAVE: Creating failed (");
    //                 console_write_dword_dec((uint32_t)res);
    //                 console_writeline(")!");
    //                 break;
    //             }
    //
    //             console_write("SAVE: Address = 0x");
    //             console_write_word(ti->addr);
    //             console_writeline(".");
    //
    //             buf = (uint8_t)(ti->addr & 0x00FF);
    //
    //             console_write("SAVE: 1. address part = 0x");
    //             console_write_byte(buf);
    //             console_writeline(".");
    //
    //             res = f_write(&fil, &buf, 1, &write_count);
    //             if(res != FR_OK)
    //             {
    //                 console_write("SAVE: 1. writing failed (");
    //                 console_write_dword_dec((uint32_t)res);
    //                 console_writeline(")!");
    //                 break;
    //             }
    //
    //             console_write("SAVE: 1. write wrote ");
    //             console_write_dword_dec(write_count);
    //             console_writeline(" bytes.");
    //
    //             buf = (uint8_t)(ti->addr >> 8);
    //
    //             console_write("SAVE: 2. address part = 0x");
    //             console_write_byte(buf);
    //             console_writeline(".");
    //
    //             res = f_write(&fil, &buf, 1, &write_count);
    //             if(res != FR_OK)
    //             {
    //                 console_write("SAVE: 2. writing failed (");
    //                 console_write_dword_dec((uint32_t)res);
    //                 console_writeline(")!");
    //                 break;
    //             }
    //
    //             console_write("SAVE: 2. write wrote ");
    //             console_write_dword_dec(write_count);
    //             console_writeline(" bytes.");
    //
    //             console_write("SAVE: Length = ");
    //             console_write_word_dec(ti->len);
    //             console_writeline(".");
    //
    //             res = f_write(&fil, ti->bytes, ti->len, &write_count);
    //             if(res != FR_OK)
    //             {
    //                 console_write("SAVE: 3. writing failed (");
    //                 console_write_dword_dec((uint32_t)res);
    //                 console_writeline(")!");
    //                 break;
    //             }
    //
    //             console_write("SAVE: 3. write wrote ");
    //             console_write_dword_dec(write_count);
    //             console_writeline(" bytes.");
    //
    //             res = f_close(&fil);
    //             if(res != FR_OK)
    //             {
    //                 console_write("SAVE: Closing failed (");
    //                 console_write_dword_dec((uint32_t)res);
    //                 console_writeline(")!");
    //                 break;
    //             }
    //
    //             res = f_mount(0, "", 0);
    //             if(res != FR_OK)
    //             {
    //                 console_write("SAVE: Unmounting failed (");
    //                 console_write_dword_dec((uint32_t)res);
    //                 console_writeline(")!");
    //                 break;
    //             }
    //
    //             statetoggle_toggle();
    //             toggle_gpio(MT_GPIO_PIN_NR_LED, 3, 200, true); // Hard-coded
    //         }while(false);
    //     }
    //
    //     alloc_free(name);
    //     alloc_free(ti->bytes);
    //     alloc_free(ti);
    // }
#endif //MT_INTERACTIVE
}
