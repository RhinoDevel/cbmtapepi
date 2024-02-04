
// Marcel Timm, RhinoDevel, 2019apr23

// TODO: Free ALL (including DMA-reserved Video Core RAM) memory on (e.g.) CTRL+C!

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
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
#include "../app/tape/tape_symbol.h"
#include "../app/petload/petload_prg_name.h"
#include "../app/petload/petload_c64tom.h"
#include "../app/petload/petload_pet4.h"
#include "../app/petload/petload_pet4tom.h"

#include "ProgressBar/ProgressBar.h"
#include "file/file.h"
#include "dma/pwm/pwm.h"
#include "dma/dma.h"
#include "dma/dma_cb.h"
#include "dma/inf/inf.h"
#include "dma/dma_gpio/dma_gpio.h"

static uint8_t * s_mem = NULL; // [see init() and deinit()]

static int const s_max_file_size = 64 * 1024; // 64 KB.
static int const s_mem_buf_size = 4 * 1024 * 1024; // 4 MB.

static int const s_progress_bar_len = 50; // Characters.

static struct dma_cb * s_data_cb = NULL; // Set and reset by send_bytes().

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

    file_init(s_max_file_size);

    // Initialize memory (heap) manager for dynamic allocation/deallocation
    // [this is used by the parts of the code that originate in the bare metal
    // version of CBM Tape Pi, where no malloc() exists]:
    //
    assert(s_mem_buf_size < MT_HEAP_SIZE); // (probably just by-definition..)
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
    uint8_t * const bytes = file_load(file_name, &size);

    if(bytes == NULL)
    {
        return false;
    }

    int const last_path_sep_pos = str_get_last_index(
            file_name, MT_FILE_PATH_SEP);

    uint8_t * const symbols = create_symbols_from_bytes(
        bytes,
        (uint32_t)size,
        last_path_sep_pos == -1
            ? file_name
            : (file_name + last_path_sep_pos + 1),
        out_symbol_count);

    alloc_free(bytes);
    return symbols;
}

/**
 * - Header and content sending both must stop when done.
 */
static bool send_cbs(
    struct dma_cb * const header_cbs,
    int const header_cbs_count,
    struct dma_cb * const content_cbs,
    int const content_cbs_count)
{
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
    dma_start(1); // Sends header once. // TODO: Hard-coded offset!
    console_deb_writeline("send_cbs: Waiting for sending header to finish..");
    
    {
        uint32_t const first_addr = dma_get_bus_addr_from_vc_ptr(header_cbs);
        uint32_t const last_addr = first_addr + 32 * (header_cbs_count - 1);

        while(dma_is_busy() && s_stop == 0)
        {
            uint32_t const next_cb_addr = dma_get_next_control_block_addr();

            if(first_addr <= next_cb_addr && next_cb_addr <= last_addr)
            {
                ProgressBar_print(
                    1,
                    1 + (next_cb_addr - first_addr) / 32,
                    header_cbs_count,
                    s_progress_bar_len,
                    true);
            }

            usleep(100);
        }
    }
    if(s_stop != 0)
    {
        console_deb_writeline("\nsend_cbs: Stopping (1)..");
        return true;
    }
    ProgressBar_print( // Cheating..
        1, header_cbs_count, header_cbs_count, s_progress_bar_len, true);
    printf("\n");

    dma_start(1 + header_cbs_count); // Sends content once. // TODO: Hard-coded offset!
    console_deb_writeline("send_cbs: Waiting for sending content to finish..");

    {
        uint32_t const first_addr = dma_get_bus_addr_from_vc_ptr(content_cbs);
        uint32_t const last_addr = first_addr + 32 * (content_cbs_count - 1);

        while(dma_is_busy() && s_stop == 0)
        {
            if(gpio_read(MT_TAPE_GPIO_PIN_NR_MOTOR))
            {
                uint32_t const next_cb_addr = dma_get_next_control_block_addr();

                if(first_addr <= next_cb_addr && next_cb_addr <= last_addr)
                {
                    ProgressBar_print(
                        1,
                        1 + (next_cb_addr - first_addr) / 32,
                        content_cbs_count,
                        s_progress_bar_len,
                        true);
                }

                usleep(100);
                continue; // Motor is on. Keep "endless tape" running.
            }

            // Motor is off.

            if(motor_done)
            {
                //console_deb_writeline("send_cbs: Motor off. Done.");
                break;
            }
            //console_deb_writeline("send_cbs: Motor off. Waiting (2)..");        
            motor_done = true;
            
            dma_pause();

            // TODO: Sometimes the loading is finished, but we are stuck, here
            //       (maybe because the first motor-off was not detected,
            //       because of OS scheduling?)!
            //
            while(!gpio_read(MT_TAPE_GPIO_PIN_NR_MOTOR) && s_stop == 0)
            {
                usleep(100);
            }
            if(s_stop != 0)
            {
                break;
            }

            //console_deb_writeline("send_cbs: Motor on. Resuming..");

            dma_resume();
        }
    }
    if(s_stop != 0)
    {
        console_deb_writeline("\nsend_cbs: Stopping (2)..");
        return true;
    }
    ProgressBar_print( // Cheating..
        1, content_cbs_count, content_cbs_count, s_progress_bar_len, true);
    printf("\n");
    console_deb_writeline("send_cbs: Sending done.");
    return true;  
}

static struct dma_cb * add_pulse_atomic_spacer(struct dma_cb * const cbs)
{
    struct dma_cb * ret_val = cbs;

    // Update PWM FIFO (clearing DMA request):

    ret_val->src_addr = dma_get_bus_addr_from_vc_ptr(
        (void*)(&(s_data_cb->reserved1))); // PWM data.
    ret_val->dest_addr = DMA_PWM_OFFSET_TO_BUS_ADDR(PWM_OFFSET_FIF1);
    
    // Enable paced transfer via PWM:
    //
    ret_val->transfer_info =
        (5 << 16) // PERMAP (bits 16-20, page 51). 5 <=> PWM.
            | (1 << 6); // DEST_DREQ (bit 6, page 52).
    
    ret_val->transfer_len = 4;
    ret_val->next_cb_addr = dma_get_bus_addr_from_vc_ptr((void*)(ret_val + 1));
    ret_val->stride = 0;
    ret_val->reserved0 = 0;
    ret_val->reserved1 = 0;

    ret_val = ret_val + 1;
    assert(ret_val - cbs == 1);
    return ret_val;
}

static struct dma_cb * add_set_to_low(struct dma_cb * const cbs)
{
    struct dma_cb * ret_val = cbs;

    ret_val->src_addr = dma_get_bus_addr_from_vc_ptr(
        (void*)(&(s_data_cb->reserved0))); // GPIO pin data.
    ret_val->dest_addr =
        DMA_GPIO_OFFSET_TO_BUS_ADDR(GPIO_OFFSET_SET0); // Inverted
    
    // Enable paced transfer via PWM:
    //
    ret_val->transfer_info =
        (5 << 16) // PERMAP (bits 16-20, page 51). 5 <=> PWM.
            | (1 << 6); // DEST_DREQ (bit 6, page 52).
    
    ret_val->transfer_len = 4;
    ret_val->next_cb_addr = dma_get_bus_addr_from_vc_ptr((void*)(ret_val + 1));
    ret_val->stride = 0;
    ret_val->reserved0 = 0;
    ret_val->reserved1 = 0;

    ret_val = ret_val + 1;
    assert(ret_val - cbs == 1);
    return ret_val;
}

static struct dma_cb * add_set_to_high(struct dma_cb * const cbs)
{
    struct dma_cb * ret_val = cbs;

    ret_val->src_addr = dma_get_bus_addr_from_vc_ptr(
        (void*)(&(s_data_cb->reserved0))); // GPIO pin data.
    ret_val->dest_addr =
        DMA_GPIO_OFFSET_TO_BUS_ADDR(GPIO_OFFSET_CLR0); // Inverted
    
    // Enable paced transfer via PWM:
    //
    ret_val->transfer_info =
        (5 << 16) // PERMAP (bits 16-20, page 51). 5 <=> PWM.
            | (1 << 6); // DEST_DREQ (bit 6, page 52).
    
    ret_val->transfer_len = 4;
    ret_val->next_cb_addr = dma_get_bus_addr_from_vc_ptr((void*)(ret_val + 1));
    ret_val->stride = 0;
    ret_val->reserved0 = 0;
    ret_val->reserved1 = 0;

    ret_val = ret_val + 1;
    assert(ret_val - cbs == 1);
    return ret_val;
}

static struct dma_cb * add_pulse_short_to_cbs(struct dma_cb * const cbs)
{
    struct dma_cb * ret_val = cbs;

    // LOAD: Low to high.
    //
    // Short: 2840 Hz <-> 352 us = 4 x 88 us = 2 x 88 us LOW + 2 x 88us HIGH.

    ret_val = add_set_to_low(ret_val);
    ret_val = add_pulse_atomic_spacer(ret_val);
    ret_val = add_pulse_atomic_spacer(ret_val);
    ret_val = add_set_to_high(ret_val);
    ret_val = add_pulse_atomic_spacer(ret_val);
    ret_val = add_pulse_atomic_spacer(ret_val);

    assert(ret_val - cbs == 6);
    return ret_val;
}

static struct dma_cb * add_pulse_medium_to_cbs(struct dma_cb * const cbs)
{
    struct dma_cb * ret_val = cbs;

    // LOAD: Low to high.
    //
    // Short:  2840 Hz <-> 352 us = 4 x 88 us = 2 x 88 us LOW + 2 x 88 us HIGH.
    // Medium: 1953 Hz <-> 512 us ~ 6 x 88 us = 3 x 88 us LOW + 3 x 88 us HIGH.

    ret_val = add_set_to_low(ret_val);
    ret_val = add_pulse_atomic_spacer(ret_val);
    ret_val = add_pulse_atomic_spacer(ret_val);
    ret_val = add_pulse_atomic_spacer(ret_val);
    ret_val = add_set_to_high(ret_val);
    ret_val = add_pulse_atomic_spacer(ret_val);
    ret_val = add_pulse_atomic_spacer(ret_val);
    ret_val = add_pulse_atomic_spacer(ret_val);

    assert(ret_val - cbs == 8);
    return ret_val;
}

static struct dma_cb * add_pulse_long_to_cbs(struct dma_cb * const cbs)
{
    struct dma_cb * ret_val = cbs;

    // LOAD: Low to high.
    //
    // Short:  2840 Hz <-> 352 us = 4 x 88 us = 2 x 88 us LOW + 2 x 88 us HIGH.
    // Medium: 1953 Hz <-> 512 us ~ 6 x 88 us = 3 x 88 us LOW + 3 x 88 us HIGH.
    // Long:   1488 Hz <-> 672 us ~ 8 x 88 us = 4 x 88 us LOW + 4 x 88 us HIGH.

    ret_val = add_set_to_low(ret_val);
    ret_val = add_pulse_atomic_spacer(ret_val);
    ret_val = add_pulse_atomic_spacer(ret_val);
    ret_val = add_pulse_atomic_spacer(ret_val);
    ret_val = add_pulse_atomic_spacer(ret_val);
    ret_val = add_set_to_high(ret_val);
    ret_val = add_pulse_atomic_spacer(ret_val);
    ret_val = add_pulse_atomic_spacer(ret_val);
    ret_val = add_pulse_atomic_spacer(ret_val);
    ret_val = add_pulse_atomic_spacer(ret_val);

    assert(ret_val - cbs == 10);
    return ret_val;
}

/**
 * - Returns NULL on error (that must never happen).
 */
static struct dma_cb * add_symbol_to_cbs(
    uint8_t const symbol, struct dma_cb * const cbs)
{
    struct dma_cb * ret_val = cbs;

    switch((enum tape_symbol)symbol)
    {
        case tape_symbol_zero:
        {
            ret_val = add_pulse_short_to_cbs(ret_val);
            ret_val = add_pulse_medium_to_cbs(ret_val);
            break;
        }
        case tape_symbol_one:
        {
            ret_val = add_pulse_medium_to_cbs(ret_val);
            ret_val = add_pulse_short_to_cbs(ret_val);
            break;
        }
        case tape_symbol_sync:
        {
            ret_val = add_pulse_short_to_cbs(ret_val);
            ret_val = add_pulse_short_to_cbs(ret_val);
            break;
        }
        case tape_symbol_new:
        {
            ret_val = add_pulse_long_to_cbs(ret_val);
            ret_val = add_pulse_medium_to_cbs(ret_val);
            break;
        }
        case tape_symbol_end: // Used for transmit block gap start, only.
        {
            ret_val = add_pulse_long_to_cbs(ret_val);
            ret_val = add_pulse_short_to_cbs(ret_val);
            break;
        }

        case tape_symbol_err: // (falls through)
        default: // Must not happen.
        {
            assert(false);
            console_deb_writeline("add_symbol_to_cbs: Error: Unknown symbol!");
            ret_val = NULL;
            break;
        }
    }
    return ret_val;
}

/**
 * - The real control block count necessary for the symbol count given is always
 *   lower. 
 */
static uint32_t get_max_cbs_count(uint32_t const symbol_count)
{
    // tape_symbol_new <=> long & medium pulse. <=> 10 CBs + 8 CBs.

    return (10 + 8) * symbol_count; // Hard-coded CB counts per symbol types.
}

/**
 * - The real byte count necessary for the symbol count given is always lower. 
 */
static uint32_t get_max_cbs_byte_count(uint32_t const symbol_count)
{
    uint32_t const max_cbs_count = get_max_cbs_count(symbol_count);

    return 32 * (1 + max_cbs_count); // Hard-coded bytes per control block & +1.
}

static struct dma_cb * fill_cbs(
    struct dma_cb * const cbs,
    uint8_t * const symbols,
    uint32_t const symbol_count,
    int * const out_cbs_count)
{
    struct dma_cb * ret_val = cbs;

    for(int i = 0;i < symbol_count; ++i)
    {
        ret_val = add_symbol_to_cbs(symbols[i], ret_val);
    }
    (ret_val - 1)->next_cb_addr = 0;
    *out_cbs_count = ret_val - cbs;
    return ret_val;
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

    console_write("send_bytes: Symbol count: ");
    console_write_dword_dec(symbol_count);
    console_writeline("");

    assert(symbol_count > MT_HEADERDATABLOCK_LEN);

    uint32_t const max_byte_count = get_max_cbs_byte_count(symbol_count);

    console_write("send_bytes: Max. bytes necessary in Video Core RAM: ");
    console_write_dword_dec(max_byte_count);
    console_writeline("");

    cbs = dma_init(
        113640, // 2 x 88 us = 176 us <-> 5682 Hz => 2 x 5682 Hz = 11364 Hz
        20, // 11364 Hz / 2 = 5682 Hz <-> 176 us = 2 x 88 us
        max_byte_count);
    if(cbs == NULL)
    {
        alloc_free(symbols);
        return false;
    }

    console_writeline("send_bytes: DMA is initialized.");

    // TODO: Add check, that there is enough memory available for CBs!

    // Misuse to not destroy 32 byte alignment of following real code blocks:
    //
    s_data_cb = cbs;
    s_data_cb->reserved0 = 1 << MT_TAPE_GPIO_PIN_NR_READ; // Pin data.
    s_data_cb->reserved1 = 1000 / 2; // TODO: Hard-coded "PWM range", but this is just for the FIFO (value should not matter)..

    header_cbs = cbs + 1;

    content_cbs = fill_cbs(
        header_cbs,
        symbols,
        MT_HEADERDATABLOCK_LEN,
        &header_cbs_count);
    if(content_cbs == NULL)
    {
        s_data_cb = NULL;
        alloc_free(symbols);
        dma_deinit();
        return false;
    }

    console_write("send_bytes: Header CBS count: ");
    console_write_dword_dec(header_cbs_count);
    console_writeline("");

    // assert(header_pulse_count == 4 * MT_HEADERDATABLOCK_LEN);

    if(fill_cbs(
        content_cbs,
        symbols + MT_HEADERDATABLOCK_LEN,
        symbol_count - MT_HEADERDATABLOCK_LEN,
        &content_cbs_count) == NULL)
    {
        s_data_cb = NULL;
        alloc_free(symbols);
        dma_deinit();
        return false;
    }

    console_write("send_bytes: Content CBS count: ");
    console_write_dword_dec(content_cbs_count);
    console_writeline("");

    // assert(content_pulse_count == 4 * (symbol_count - MT_HEADERDATABLOCK_LEN));

    s_data_cb = NULL;
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
        s_data_cb = NULL;
        dma_deinit();
        return false;
    }

    if(infinitely)
    {
        console_writeline(
            "send_bytes: Starting infinite sending (press CTRL+C to exit/stop)..");

        do
        {
            if(!send_cbs(
                    header_cbs,
                    header_cbs_count,
                    content_cbs,
                    content_cbs_count))
            {
                s_data_cb = NULL;
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

        if(!send_cbs(
                header_cbs,
                header_cbs_count,
                content_cbs,
                content_cbs_count))
        {
            s_data_cb = NULL;
            dma_deinit();
            return false;
        }
    }

    s_data_cb = NULL;
    dma_deinit();
    return true;
}

static bool send_petload_c64tom(bool const infinitely)
{
    return send_bytes(
        (uint8_t*)s_petload_c64tom, // *** CONST CAST ***
        sizeof s_petload_c64tom  / sizeof *s_petload_c64tom,
        MT_PETLOAD_PRG_NAME_RUN,
        infinitely);
}

static bool send_petload_pet4(bool const infinitely)
{
    return send_bytes(
        (uint8_t*)s_petload_pet4, // *** CONST CAST ***
        sizeof s_petload_pet4  / sizeof *s_petload_pet4,
        MT_PETLOAD_PRG_NAME_TAPE_BUF,
        infinitely);
}

static bool send_petload_pet4tom(bool const infinitely)
{
    return send_bytes(
        (uint8_t*)s_petload_pet4tom, // *** CONST CAST ***
        sizeof s_petload_pet4tom  / sizeof *s_petload_pet4tom,
        MT_PETLOAD_PRG_NAME_RUN,
        infinitely);
}

/** Send file with given name/path via compatibility mode.
 */
static bool send_file(char * const file_name, bool const infinitely)
{
    // Assuming uint8_t being equal to unsigned char.

    off_t size = 0;
    uint8_t * const bytes = file_load(file_name, &size);

    if(bytes == NULL)
    {
        return false;
    }

    int const last_path_sep_pos = str_get_last_index(
            file_name, MT_FILE_PATH_SEP);

    bool const success = send_bytes(
        bytes, 
        (uint32_t)size,
        last_path_sep_pos == -1
            ? file_name
            : (file_name + last_path_sep_pos + 1),
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
            case 'v':
            {
                if(argc != 2)
                {
                    break;
                }
                return send_petload_pet4(false);
            }
            case 'w':
            {
                if(argc != 2)
                {
                    break;
                }
                return send_petload_pet4(true);
            }
            case 'q':
            {
                if(argc != 2)
                {
                    break;
                }
                return send_petload_pet4tom(false);
            }
            case 'r':
            {
                if(argc != 2)
                {
                    break;
                }
                return send_petload_pet4tom(true);
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
        "v = Send PET BASIC v4 wedge via compatibility mode once."
        "\n"
        "w = Send PET BASIC v4 wedge via compatibility mode in a loop."
        "\n"
        "q = Send PET BASIC v4 TOM wedge via compatibility mode once."
        "\n"
        "r = Send PET BASIC v4 TOM wedge via compatibility mode in a loop."
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
