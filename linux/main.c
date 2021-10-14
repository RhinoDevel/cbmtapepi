
// Marcel Timm, RhinoDevel, 2019apr23

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>
#include <signal.h>
#include <pigpio.h>

#include "../lib/str/str.h"
#include "../lib/alloc/alloc.h"
#include "../lib/console/console_params.h"
#include "../lib/console/console.h"
#include "../app/config.h"
#include "../app/tape/tape_input.h"
#include "../app/tape/tape_fill_buf.h"
#include "../app/tape/tape_defines.h"
#include "../app/petload/petload_c64tom.h"
#include "pigpio/pigpio.h"

static uint8_t * s_mem = NULL; // [see init() and deinit()]

static int const s_max_file_size = 64 * 1024; // 64 KB.
static int const s_mem_buf_size = 4 * 1024 * 1024; // 4 MB.

static volatile sig_atomic_t s_stop = 0;

static void signal_handler(int p)
{
    s_stop = 1;
}

static uint8_t dummy_read()
{
    assert(false); // Do not use this.

    return 0;
}
static void write_byte(uint8_t const byte)
{
    putchar((int)byte);
}

off_t get_file_size(char const * const path)
{
    assert(path != NULL);

    struct stat s;

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
static unsigned char * load_file(
    char const * const path, off_t * const out_size)
{
    *out_size = -1;

    off_t const signed_size = get_file_size(path);

    if(signed_size == -1)
    {
        console_writeline("load_file : Error: Failed to get size of file!");
        return NULL;
    }
    if(signed_size > (off_t)s_max_file_size)
    {
        console_writeline("load_file : Error: File is too big!");
        return NULL;
    }

    FILE * const file = fopen(path, "rb");

    if(file == NULL)
    {
        console_writeline("load_file : Error: Failed to open source file!");
        return NULL;
    }

    size_t const size = (size_t)signed_size;
    unsigned char * const buf = alloc_alloc(size * sizeof *buf);

    if(fread(buf, sizeof(*buf), size, file) != size)
    {
        console_writeline(
            "load_file : Error: Failed to completely load file content!");
        return NULL;
    }

    fclose(file);
    *out_size = signed_size;
    return buf;
}

/** Connect console (singleton) to wanted in-/output.
 */
static void init_console()
{
    struct console_params p;

    // No read:
    //
    p.read_byte = dummy_read;

    p.write_byte = write_byte;
    p.write_newline_with_cr = true;

    console_init(&p);
}

static void init_gpio()
{
    pigpio_init();

    console_deb_writeline(
        "init_gpio: Setting sense output line to HIGH at CBM..");
    gpioSetMode(MT_TAPE_GPIO_PIN_NR_SENSE, PI_OUTPUT);
    gpioWrite(MT_TAPE_GPIO_PIN_NR_SENSE, (unsigned)(!true));
    //
    // (inverted, because circuit inverts signal to CBM)

    console_deb_writeline(
        "init_gpio: Setting motor line to input with pull-down..");
    gpioSetMode(MT_TAPE_GPIO_PIN_NR_MOTOR, PI_INPUT);
    gpioSetPullUpDown(MT_TAPE_GPIO_PIN_NR_MOTOR, PI_PUD_DOWN);

    console_deb_writeline(
        "init_gpio: Setting tape read output line to HIGH at CBM..");
    gpioSetMode(MT_TAPE_GPIO_PIN_NR_READ, PI_OUTPUT);
    gpioWrite(MT_TAPE_GPIO_PIN_NR_READ, (unsigned)(!true));
    //
    // (inverted, because circuit inverts signal to CBM)

    console_deb_writeline(
        "init_gpio: Setting tape write line to input with pull-down..");
    gpioSetMode(MT_TAPE_GPIO_PIN_NR_WRITE, PI_INPUT);
    gpioSetPullUpDown(MT_TAPE_GPIO_PIN_NR_WRITE, PI_PUD_DOWN);
}

static void deinit()
{
    gpioTerminate();

    free(s_mem);
    s_mem = NULL;
}

static void init()
{
    init_console();

    init_gpio();

    // Initialize memory (heap) manager for dynamic allocation/deallocation:
    //
    assert(s_mem_buf_size < MT_HEAP_SIZE);
    s_mem = malloc(MT_HEAP_SIZE * sizeof *s_mem);
    alloc_init((void*)s_mem, MT_HEAP_SIZE);
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
        // TODO: Implement real conversion to PETSCII!
        //
        name_out[i] = (uint8_t)buf[i];
        ++i;
    }
    while(i < MT_TAPE_INPUT_NAME_LEN)
    {
        name_out[i] = 0x20;
        ++i;
    }

    alloc_free(buf);
}

/**
 * - Caller takes ownership of returned object.
 */
static struct tape_input * create_tape_input(
    uint8_t * const bytes, uint32_t const count, char const * const name)
{
    struct tape_input * const ret_val = alloc_alloc(sizeof *ret_val);

    fill_name(ret_val->name, name);

    // Hard-coded for PET PRG files. C64 (and other) machines need to load
    // PRGs that are not starting at BASIC start address / are not relocatable
    // as non-relocatable because of this (e.g.: LOAD"",1,1 on C64):
    //
    ret_val->type = tape_filetype_relocatable;

    // First two bytes hold the start address:
    //
    ret_val->addr = (((uint16_t)bytes[1]) << 8) | (uint16_t)bytes[0];
    ret_val->bytes = bytes + 2;
    ret_val->len = count - 2;

    tape_input_fill_add_bytes(ret_val->add_bytes);

#ifndef NDEBUG
    console_write("create_tape_input: Start address is 0x");
    console_write_word(ret_val->addr);
    console_write(" (");
    console_write_word_dec(ret_val->addr);
    console_writeline(").");
#endif //NDEBUG

    return ret_val;
}

/**
 * - Caller takes ownership of returned object.
 */
static uint8_t* create_symbols_from_bytes(
    uint8_t * const bytes,
    uint32_t const byte_count,
    char const * const name,
    int * const out_symbol_count)
{
    struct tape_input * const t = create_tape_input(bytes, byte_count, name);
    uint32_t * const ret_val = alloc_alloc(s_mem_buf_size);

    *out_symbol_count = tape_fill_buf(t, (uint8_t * const)ret_val);

    alloc_free(t);
    return (uint8_t*)ret_val;
}

/**
 * - Caller takes ownership of returned object.
 */
static uint8_t* create_symbols_from_file(
    char * const file_name, int * const out_symbol_count)
{
    // Assuming uint8_t being equal to unsigned char.

    off_t size = 0;
    uint8_t * const bytes = load_file(file_name, &size);

    if(bytes == NULL)
    {
        return false;
    }

    uint8_t * const symbols = create_symbols_from_bytes(
        bytes,
        (uint32_t)size,
        file_name, // TODO: Remove maybe preceding path!
        out_symbol_count);

    alloc_free(bytes);
    return symbols;
}

static bool send_waves(int const header_wave_id, int const content_wave_id)
{
    assert(header_wave_id >= 0 && content_wave_id >= 0);

    int send_result = -1;
    
    console_deb_writeline("send_waves: Sending..");

    send_result = gpioWaveTxSend(header_wave_id, PI_WAVE_MODE_ONE_SHOT_SYNC);
    if(send_result == PI_BAD_WAVE_ID || send_result == PI_BAD_WAVE_MODE)
    {
        return false;
    }
    console_deb_writeline("send_waves: Waiting for sending header to finish..");
    while(gpioWaveTxBusy() == 1 && s_stop == 0)
    {
        ;
    }
    if(s_stop != 0)
    {
        console_deb_writeline("\nsend_waves: Stopping (1)..");
        return gpioWaveTxStop() == 0;
    }

    // TODO: Take motor signal stop/restart into account!

    send_result = gpioWaveTxSend(content_wave_id, PI_WAVE_MODE_ONE_SHOT_SYNC);
    if(send_result == PI_BAD_WAVE_ID || send_result == PI_BAD_WAVE_MODE)
    {
        return false;
    }
    console_deb_writeline(
        "send_waves: Waiting for sending content to finish..");

    while(gpioWaveTxBusy() == 1 && s_stop == 0)
    {
        ;
    }
    if(s_stop != 0)
    {
        console_deb_writeline("\nsend_waves: Stopping (2)..");
        return gpioWaveTxStop() == 0;
    }

    console_deb_writeline("send_waves: Sending done.");
    return true;
}

/** Send bytes given via compatibility mode.
 */
static bool send_bytes(
    uint8_t * const bytes,
    uint32_t const byte_count,
    char const * const name,
    bool const infinitely)
{
    int symbol_count = 0;
    uint8_t* symbols = create_symbols_from_bytes(
        bytes, byte_count, name, &symbol_count);

    if(symbols == NULL)
    {
        return false;
    }

    assert(symbol_count > MT_HEADERDATABLOCK_LEN);

    int const header_wave_id = pigpio_create_wave_from_symbols(
            MT_TAPE_GPIO_PIN_NR_READ,
            symbols,
            MT_HEADERDATABLOCK_LEN);

    if(header_wave_id < 0)
    {
        alloc_free(symbols);
        return false;
    }

    int const content_wave_id = pigpio_create_wave_from_symbols(
            MT_TAPE_GPIO_PIN_NR_READ,
            symbols + MT_HEADERDATABLOCK_LEN,
            symbol_count - MT_HEADERDATABLOCK_LEN);

    alloc_free(symbols);
    symbols = NULL;
    symbol_count = 0;

    if(content_wave_id < 0)
    {
        gpioWaveClear();
        return false;
    }

    console_deb_writeline(
        "send_bytes: Setting sense output line to LOW at CBM..");
    gpioWrite(MT_TAPE_GPIO_PIN_NR_SENSE, (unsigned)(!false));
    //
    // (inverted, because circuit inverts signal to CBM)

    if(infinitely)
    {
        console_writeline(
            "send_bytes: Starting infinite sending (press CTRL+C to exit/stop)..");

        s_stop = 0;
        if(signal(SIGINT, signal_handler) == SIG_ERR)
        {
            gpioWaveClear();
            return false;
        }
        
        do
        {
            if(!send_waves(header_wave_id, content_wave_id))
            {
                gpioWaveClear();
                return false;
            }
        }while(s_stop == 0);

        console_writeline("send_bytes: Stopping send loop and exiting..");
        
        if(gpioWaveTxStop() != 0)
        {
            gpioWaveClear();
            return false;
        }
    }
    else
    {
        if(!send_waves(header_wave_id, content_wave_id))
        {
            gpioWaveClear();
            return false;
        }
    }

    gpioWaveClear();
    return true;
}

static bool send_petload_c64tom(bool const infinitely)
{
    return send_bytes(
        (uint8_t*)s_petload_c64tom, // *** CONST CAST ***
        sizeof s_petload_c64tom  / sizeof *s_petload_c64tom,
        "c64tom", // TODO: Use correct name (to help user)!
        infinitely);
}

/** Send file with given name/path via compatibility mode.
 */
static bool send_file(char * const file_name, bool const infinitely)
{
    // Assuming uint8_t being equal to unsigned char.

    off_t size = 0;
    uint8_t * const bytes = load_file(file_name, &size);

    if(bytes == NULL)
    {
        return false;
    }

    bool const success = send_bytes(
        bytes, 
        (uint32_t)size,
        file_name, // TODO: Remove maybe preceding path!
        infinitely);

    alloc_free(bytes);
    return success;
}

/** Load file from given file name/path, convert it to compatibility mode
 *  symbols and write these to standard output.
 */
static bool print_file_as_symbols(char * const file_name)
{
    int symbol_count = 0;
    uint8_t* symbols = create_symbols_from_file(file_name, &symbol_count);

    if(symbols == NULL)
    {
        return false;
    }

    for(int i = 0;i < symbol_count;++i)
    {
        write_byte(symbols[i]);
    }

    return true;
}

static bool exec(int const argc, char * const argv[])
{
    do
    {
        if(argc < 2) // At least a command must be given.
        {
            break;
        }
        if(strnlen(argv[1], 2) != 1) // Single letter commands, only.
        {
            break;
        }

        char const cmd = argv[1][0];

        switch(cmd)
        {
            case 's':
            {
                if(argc != 3)
                {
                    break;
                }
                return send_file(argv[2], false);
            }
            case 'i':
            {
                if(argc != 3)
                {
                    break;
                }
                return send_file(argv[2], true);
            }
            case 't':
            {
                if(argc != 2)
                {
                    break;
                }
                return send_petload_c64tom(false);
            }
            case 'u':
            {
                if(argc != 2)
                {
                    break;
                }
                return send_petload_c64tom(true);
            }
            case 'y':
            {
                if(argc != 3)
                {
                    break;
                }
                return print_file_as_symbols(argv[2]);
            }

            default:
            {
                break;
            }
        }
    }while(false);

    console_writeline(
        "s <filename> = Send file via compatibility mode once."
        "\n"
        "i <filename> = Send file via compatibility mode in a loop."
        "\n"
        "t = Send C64 TOM wedge via compatibility mode once."
        "\n"
        "u = Send C64 TOM wedge via compatibility mode in a loop."
        "\n"
        "y <filename> = Output file as compatibility mode symbols.");
    return false;
}

int main(int argc, char* argv[])
{
    bool success = false;

    init();
    success = exec(argc, argv);
    deinit();

    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
