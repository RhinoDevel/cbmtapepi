
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
#include "../lib/ff14/source/ff.h"
#include "../lib/dir/dir.h"
#include "../lib/filesys/filesys.h"
#include "../lib/str/str.h"

// #if PERI_BASE == PERI_BASE_PI0AND1
//     #define VIDEO_SUPPORT 1
// #else //PERI_BASE == PERI_BASE_PI0AND1
//     #define VIDEO_SUPPORT 0
// #endif //PERI_BASE == PERI_BASE_PI0AND1
// #if VIDEO_SUPPORT
//     #include "../lib/video/video.h"
// #endif //VIDEO_SUPPORT

extern uint32_t __heap; // See memmap.ld.

static uint32_t const s_irq_interval_us = 100;

void _start(); // See boot.S.

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

    static const uint32_t act_interval = 500000; // 0.5s
    static const uint32_t act_count = act_interval / s_irq_interval_us;

    static bool act_state = false;
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

    armtimer_start((uint32_t)s_irq_interval_us, 250);
    //
    assert(s_irq_interval_us == 100);
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
    while(true)
    {
        ;
    }
#endif //MT_INTERACTIVE
}
