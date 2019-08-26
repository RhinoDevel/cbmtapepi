
// RhinoDevel, Marcel Timm, 2019apr23

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>
//#include <dirent.h>
#include <sys/stat.h>

#include "../hardware/peribase.h"
#include "../lib/str/str.h"
#include "../lib/mem/mem.h"
#include "../lib/alloc/alloc.h"
#include "../lib/console/console_params.h"
#include "../lib/console/console.h"
#include "../hardware/baregpio/baregpio.h"
#include "../hardware/baregpio/baregpio_params.h"
#include "../app/config.h"
#include "../app/tape/tape_init.h"
#include "../app/tape/tape_send_params.h"
#include "../app/tape/tape_input.h"
#include "../app/tape/tape_send.h"

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

static void fill_name(uint8_t * const name_out, char const * const name_in)
{
    // static uint8_t const sample_name[] = {
    //     'R', 'H', 'I', 'N', 'O', 'D', 'E', 'V', 'E', 'L',
    //     0x20, 0x20, 0x20, 0x20, 0x20, 0x20
    // };

    int i = 0;
    char * const buf = alloc_alloc(str_get_len(name_in) + 1);

    str_to_upper(buf, name_in);

    while(i < MT_TAPE_INPUT_NAME_LEN && buf[i] != '\0')
    {
        name_out[i] = (uint8_t)buf[i]; // TODO: Implement real conversion to PETSCII.
        ++i;
    }
    while(i < MT_TAPE_INPUT_NAME_LEN)
    {
        name_out[i] = 0x20;
        ++i;
    }

    alloc_free(buf);
}

static void fill_add_bytes(uint8_t * const add_bytes)
{
    // Additional bytes (to be stored in header):
    //
    for(int i = 0;i<MT_TAPE_INPUT_ADD_BYTES_LEN;++i)
    {
        add_bytes[i] = 0x20;
    }
}

static bool send_to_commodore(
    uint8_t /*const*/ * const bytes,
    char const * const name,
    uint32_t const count)
{
    bool ret_val = false;
    struct tape_send_params p;
    uint32_t * const mem_addr = alloc_alloc(4 * 1024 * 1024); // Hard-coded

    p.is_stop_requested = NULL;
    p.gpio_pin_nr_read = MT_TAPE_GPIO_PIN_NR_READ;
    p.gpio_pin_nr_sense = MT_TAPE_GPIO_PIN_NR_SENSE;
    p.gpio_pin_nr_motor = MT_TAPE_GPIO_PIN_NR_MOTOR;
    p.data = alloc_alloc(sizeof *(p.data));

    fill_name(p.data->name, name);

    p.data->type = tape_filetype_relocatable; // (necessary for PET PRG file)
    //
    // Hard-coded - maybe not always correct, but works for C64 and PET,
    // both with machine language and BASIC PRG files.

    // First two bytes hold the start address:
    //
    p.data->addr = *((uint16_t const *)bytes);
    p.data->bytes = bytes + 2;
    p.data->len = count - 2;

    fill_add_bytes(p.data->add_bytes);

#ifndef NDEBUG
    console_write("Start address is 0x");
    console_write_word(p.data->addr);
    console_write(" (");
    console_write_word_dec(p.data->addr);
    console_writeline(").");
#endif //NDEBUG

    ret_val = tape_send(&p, mem_addr);

    alloc_free(mem_addr);
    alloc_free(p.data);
    return ret_val;
}

/** Source: http://stackoverflow.com/questions/8236/how-do-you-determine-the-size-of-a-file-in-c
 */
off_t get_file_size(char const * const path)
{
    struct stat s;

    //assert(path != NULL);

    if(stat(path, &s) == 0)
    {
        return s.st_size;
    }
    return -1;
}

/** Return content of file at given path.
 *
 *  - Returns NULL on error.
 *  - Caller takes ownership of return value.
 */
static unsigned char * load_file(char const * const path, off_t * const out_size)
{
    *out_size = -1;

    off_t const signed_size = get_file_size(path);

    if(signed_size == -1)
    {
        console_writeline("Error: Failed to get size of file!");
        return NULL;
    }

    FILE * const file = fopen(path, "rb");

    if(file == NULL)
    {
        console_writeline("Error: Failed to open source file!");
        return NULL;
    }

    size_t const size = (size_t)signed_size;
    unsigned char * const buf = malloc(size * sizeof *buf);

    if(fread(buf, sizeof(*buf), size, file) != size)
    {
        console_writeline("Error: Failed to completely load file content!");
        return NULL;
    }

    fclose(file);
    *out_size = signed_size;
    return buf;
}

static bool send(char * const file_name)
{
    // Assuming uint8_t being equal to unsigned char.

    off_t size = 0;
    uint8_t /*const*/ * const bytes = load_file(file_name, &size);

    if(bytes == NULL)
    {
        return false;
    }

    char * const name = file_name; // TODO: Remove eventually trailing path!
    uint32_t const count = (uint32_t)size;

    if(!send_to_commodore(bytes, name, count))
    {
        free(bytes);
        return false;
    }
    free(bytes);
    return true;
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
