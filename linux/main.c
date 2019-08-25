
// RhinoDevel, Marcel Timm, 2019apr23

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>

#include "../hardware/peribase.h"
#include "../lib/mem/mem.h"
#include "../lib/alloc/alloc.h"
#include "../lib/console/console_params.h"
#include "../lib/console/console.h"
#include "../hardware/baregpio/baregpio.h"
#include "../hardware/baregpio/baregpio_params.h"
#include "../app/config.h"
#include "../app/tape/tape_init.h"

static void timer_start_one_mhz()
{
    // Nothing to do.
}

static uint32_t timer_get_tick()
{
    struct timeval tv;

    gettimeofday(&tv, 0); // (return value assumed to be zero)

    return 1000000 * tv.tv_sec + tv.tv_usec;
}

static void timer_wait_microseconds(uint32_t const microseconds)
{
    usleep(microseconds); // (implicit cast & return value ignored)
}

/**
 * - Must not be called. Just for error handling..
 */
static uint8_t dummy_read()
{
    //assert(false);

    return 0;
}

static void write_byte(uint8_t const byte)
{
    putchar((int)byte);
}

static void init_gpio()
{
    // Initialize bare GPIO singleton:
    //
    baregpio_init((struct baregpio_params){
        .wait_microseconds = timer_wait_microseconds,

        .mem_read = mem_read,
        .mem_write = mem_write,

        .peri_base = PERI_BASE
    });
}

/** Connect console (singleton) to wanted in-/output.
 */
static void init_console()
{
    struct console_params p;

    // Initialize console via MiniUART to read and video to write:

    // No read:
    //
    p.read_byte = dummy_read;

    p.write_byte = write_byte;
    p.write_newline_with_cr = true;

    console_init(&p);
}

int main()
{
    uint8_t * const mem = malloc(MT_HEAP_SIZE * sizeof *mem);

    init_gpio();

    init_console();

    // Initialize memory (heap) manager for dynamic allocation/deallocation:
    //
    alloc_init((void*)mem, MT_HEAP_SIZE);

    // Initialize for tape transfer:
    //
    tape_init(timer_start_one_mhz, timer_get_tick, timer_wait_microseconds);

    // Console test:
    //
    // console_writeline("Hello World!");
    // console_writeline("");
    // console_writeline("I am CBM Tape Pi.");

    // Button and LED test:
    //
    // baregpio_set_input_pull_down(MT_GPIO_PIN_NR_BUTTON);
    //
    // baregpio_set_output(MT_GPIO_PIN_NR_LED, true);
    //
    // baregpio_wait_for_high(MT_GPIO_PIN_NR_BUTTON);
    //
    // baregpio_set_output(MT_GPIO_PIN_NR_LED, false);

    free(mem);
    return 0;
}
