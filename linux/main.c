
// Marcel Timm, RhinoDevel, 2019apr23

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>
#include <signal.h>

#include "../lib/console/console_params.h"
#include "../lib/console/console.h"
#include "../lib/alloc/alloc.h"
#include "../lib/str/str.h"
#include "../lib/petasc/petasc.h"

#include "../hardware/gpio/gpio_params.h"
#include "../hardware/gpio/gpio.h"
#include "../hardware/peribase.h"

#include "../app/config.h"
#include "../app/tape/tape_fill_buf.h"
#include "../app/tape/tape_input.h"
#include "../app/tape/tape_filetype.h"
#include "../app/tape/tape_defines.h"
#include "../app/petload/petload_c64tom.h"

#include "dma/dma.h"
#include "dma/dma_cb.h"

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

/**
 * - Remember that this will not work precisely! 
 */
static void busywait_microseconds(uint32_t const microseconds)
{
    usleep((useconds_t)microseconds);
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
    // Initialize GPIO singleton:
    //
    gpio_init((struct gpio_params){
        .wait_microseconds = busywait_microseconds,
        .peri_base = PERI_BASE
    });

    console_deb_writeline(
        "init_gpio: Setting sense output line to HIGH at CBM..");
    gpio_set_output(MT_TAPE_GPIO_PIN_NR_SENSE, !true);
    //
    // (inverted, because circuit inverts signal to CBM)

    console_deb_writeline(
        "init_gpio: Setting motor line to input with pull-down..");
    gpio_set_input_pull_down(MT_TAPE_GPIO_PIN_NR_MOTOR);

    console_deb_writeline(
        "init_gpio: Setting tape read output line to HIGH at CBM..");
    gpio_set_output(MT_TAPE_GPIO_PIN_NR_READ, !true);
    //
    // (inverted, because circuit inverts signal to CBM)

    console_deb_writeline(
        "init_gpio: Setting tape write line to input with pull-down..");
    gpio_set_input_pull_down(MT_TAPE_GPIO_PIN_NR_WRITE);
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

/**
 * - Header sending must stop when done, content sending must be an infinite
 *   loop (by concatenating the last CB and the first). 
 */
static bool send_cbs(
    struct dma_cb * const header_cbs,
    int const header_cbs_count,
    struct dma_cb * const content_cbs)
{
    // TODO: Not sure if this is working with motor on/off..!

    bool motor_done = false;

    if(!gpio_read(MT_TAPE_GPIO_PIN_NR_MOTOR))
    {
        console_deb_writeline("send_cbs: Motor off. Waiting (1)..");

        while(!gpio_read(MT_TAPE_GPIO_PIN_NR_MOTOR) && s_stop == 0)
        {
            ;
        }
    }
    console_deb_writeline("send_cbs: Motor on. Sending..");
    dma_start(0); // Sends header once.
    console_deb_writeline("send_cbs: Waiting for sending header to finish..");
    while(dma_is_busy() && s_stop == 0)
    {
        ;
    }
    if(s_stop != 0)
    {
        console_deb_writeline("\nsend_cbs: Stopping (1)..");
        return true;
    }

    dma_start(header_cbs_count); // Infinite loop of CBs.
    console_deb_writeline("send_cbs: Waiting for sending content to finish..");
    while(s_stop == 0)
    {
        if(gpio_read(MT_TAPE_GPIO_PIN_NR_MOTOR))
        {
            continue; // Motor is on. Keep "endless tape" running.
        }

        // Motor is off.

        if(motor_done)
        {
            console_deb_writeline("send_cbs: Motor off. Done.");
            break;
        }
        console_deb_writeline("send_cbs: Motor off. Waiting (2)..");        
        motor_done = true;
        
        // TODO: NO pause possible!? Necessary?

        while(!gpio_read(MT_TAPE_GPIO_PIN_NR_MOTOR) && s_stop == 0)
        {
            ;
        }
        if(s_stop != 0)
        {
            break;
        }

        console_deb_writeline("send_cbs: Motor on. Resuming..");

        // TODO: NO resume possible!? Necessary?
    }
    if(s_stop != 0)
    {
        console_deb_writeline("\nsend_cbs: Stopping (2)..");
        return true;
    }
    console_deb_writeline("send_cbs: Sending done.");
    return true;  
}

static struct dma_cb * fill_cbs(
    struct dma_cb * const cbs,
    uint32_t const gpio_pin_nr,
    uint8_t * const symbols,
    uint32_t const symbol_count,
    int * const out_cbs_count)
{
    return NULL; // TODO: Implement!
}

/** Send bytes given via compatibility mode.
 */
static bool send_bytes(
    uint8_t * const bytes,
    uint32_t const byte_count,
    char const * const name,
    bool const infinitely)
{
    int symbol_count = 0,
        header_cbs_count = 0,
        content_cbs_count = 0;
    uint8_t* symbols = NULL;
    struct dma_cb * cbs = NULL, 
        * header_cbs = NULL,
        * content_cbs = NULL;

    symbols = create_symbols_from_bytes(bytes, byte_count, name, &symbol_count);
    if(symbols == NULL)
    {
        return false;
    }

    assert(symbol_count > MT_HEADERDATABLOCK_LEN);

    cbs = dma_init(88 * 1000, 1000, 32 * 1024 * 1024); // TODO: Hard-coded!
    if(cbs == NULL)
    {
        alloc_free(symbols);
        return false;
    }

    // TODO: Add check, that there is enough memory available for CBs!

    header_cbs = fill_cbs(
        cbs,
        MT_TAPE_GPIO_PIN_NR_READ,
        symbols,
        MT_HEADERDATABLOCK_LEN,
        &header_cbs_count);
    if(header_cbs == NULL)
    {
        alloc_free(symbols);
        dma_deinit();
        return false;
    }

    // assert(header_pulse_count == 4 * MT_HEADERDATABLOCK_LEN);

    content_cbs = fill_cbs(
        cbs + header_cbs_count,
        MT_TAPE_GPIO_PIN_NR_READ,
        symbols + MT_HEADERDATABLOCK_LEN,
        symbol_count - MT_HEADERDATABLOCK_LEN,
        &content_cbs_count);
    if(content_cbs == NULL)
    {
        alloc_free(symbols);
        dma_deinit();
        return false;
    }

    // assert(content_pulse_count == 4 * (symbol_count - MT_HEADERDATABLOCK_LEN));

    alloc_free(symbols);
    symbols = NULL;
    symbol_count = 0;

    console_writeline(
        "send_bytes: Power-on Commodore, start LOAD command and press ENTER key.");
    getchar();

    console_deb_writeline(
        "send_bytes: Setting sense output line to LOW at CBM..");
    gpio_write(MT_TAPE_GPIO_PIN_NR_SENSE, !false);
    //
    // (inverted, because circuit inverts signal to CBM)

    s_stop = 0;
    if(signal(SIGINT, signal_handler) == SIG_ERR)
    {
        dma_deinit();
        return false;
    }

    if(infinitely)
    {
        console_writeline(
            "send_bytes: Starting infinite sending (press CTRL+C to exit/stop)..");
        
        do
        {
            if(!send_cbs(header_cbs, header_cbs_count, content_cbs))
            {
                dma_deinit();
                return false;
            }
        }while(s_stop == 0);

        console_writeline("send_bytes: Stopping send loop and exiting..");
    }
    else
    {
        console_writeline(
            "send_bytes: Starting sending (press CTRL+C to exit/stop)..");

        if(!send_cbs(header_cbs, header_cbs_count, content_cbs))
        {
            dma_deinit();
            return false;
        }
    }

    dma_deinit();
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
