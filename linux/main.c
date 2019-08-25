
// RhinoDevel, Marcel Timm, 2019apr23

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

#include "../hardware/peribase.h"
#include "../lib/mem/mem.h"
#include "../lib/console/console_params.h"
#include "../lib/console/console.h"
#include "../hardware/baregpio/baregpio.h"
#include "../hardware/baregpio/baregpio_params.h"
#include "../app/config.h"

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

static void wait_microseconds(uint32_t const microseconds)
{
    usleep(microseconds); // (implicit cast & return value ignored)
}

static void init_gpio()
{
    // Initialize bare GPIO singleton:
    //
    baregpio_init((struct baregpio_params){
        .wait_microseconds = wait_microseconds,

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
    init_gpio();

    init_console();

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

    return 0;
}
