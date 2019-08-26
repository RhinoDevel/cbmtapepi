
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

static uint8_t * s_mem = NULL; // [see init() and deinit()]

static void timer_start_one_mhz()
{
    // Nothing to do.
}

static uint32_t timer_get_tick()
{
    // 1 MHz <=> 1,000,000 ticks per second.
    //
    // 32 bit wide counter <=> 2^32 values.
    //
    // => More than 71 minutes until wrap-around.

    struct timeval tv;

    gettimeofday(&tv, NULL); // (return value assumed to be zero)

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

static void deinit()
{
    free(s_mem);
    s_mem = NULL;
}

static void init()
{
    init_console();

    init_gpio();

    // Initialize memory (heap) manager for dynamic allocation/deallocation:
    //
    s_mem = malloc(MT_HEAP_SIZE * sizeof *s_mem);
    alloc_init((void*)s_mem, MT_HEAP_SIZE);

    // Initialize for tape transfer:
    //
    tape_init(timer_start_one_mhz, timer_get_tick, timer_wait_microseconds);
}

static bool send(char * const file_name)
{
    console_writeline("Send mode is not implemented, yet.");

    return false;
}

static bool receive()
{
    console_writeline("Receive mode is not implemented, yet.");

    return false;
}

static bool exec(int const argc, char * const argv[])
{
    if(argc != 1 && argc != 2)
    {
        console_writeline("Add no parameter to receive or one parameter (the file name) to send.");
        return false;
    }

    if(argc == 1)
    {
        return receive();
    }

    //assert(argc == 2);

    return send(argv[1]);
}

int main(int argc, char* argv[])
{
    bool success = false;

    init();

    success = exec(argc, argv);

    deinit();

    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
