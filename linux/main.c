
// Marcel Timm, RhinoDevel, 2019apr23

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <time.h>

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
#include "send_cbs_result.h"

static uint8_t * s_mem = NULL; // [see init() and deinit()]

static int const s_max_file_size = 64 * 1024; // 64 KB.
static int const s_mem_buf_size = 4 * 1024 * 1024; // 4 MB.

static int const s_progress_bar_len = 50; // Characters.

static struct dma_cb * s_data_cb = NULL; // Set and reset by send_bytes().

static volatile sig_atomic_t s_stop = 0;

static uint64_t s_signal_last = 0;

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

static uint64_t get_tick_us()
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);

    return (uint64_t)(ts.tv_nsec / 1000) + ((uint64_t)ts.tv_sec * 1000000ull);
}
static bool is_signal_detected()
{
    if(!gpio_get_eds(MT_TAPE_GPIO_PIN_NR_WRITE))
    {
        return false; // No edge detected since last calling this function.
    }

    // There was an edge detected since last call of this function!

    gpio_clear_eds(MT_TAPE_GPIO_PIN_NR_WRITE); // Resets the flag for next time.

    uint64_t const cur = get_tick_us(); // Gets current timestamp.

    if(s_signal_last == 0 || // Should not be necessary..
        cur - s_signal_last > 5000) // Hard-coded 5000 (see kernel_main.c).
    {
        // The last detected edge is too long ago for the signal to-be-detected.

        s_signal_last = cur; // Remembers timestamp for next check.
        return false;
    }
    return true; // Signal detected (because of short enough "pulse" found)!
}
static void init_signal_detect()
{
    gpio_enable_fen(MT_TAPE_GPIO_PIN_NR_WRITE); // Falling edge detection.
    gpio_enable_ren(MT_TAPE_GPIO_PIN_NR_WRITE); // Rising edge detection.

    gpio_clear_eds(MT_TAPE_GPIO_PIN_NR_WRITE);

    s_signal_last = 0;
}
static void deinit_signal_detect()
{
    gpio_disable_fen(MT_TAPE_GPIO_PIN_NR_WRITE); // Falling edge detection.
    gpio_disable_ren(MT_TAPE_GPIO_PIN_NR_WRITE); // Rising edge detection.

    s_signal_last = 0;
}

/**
 * - Returns false, if stop signal was received (NOT an error). 
 */
static enum send_cbs_result send_cbs(
    struct dma_cb * const header_cbs,
    int const header_cbs_count,
    struct dma_cb * const content_cbs,
    int const content_cbs_count,
    bool const detect_signal)
{
    enum send_cbs_result ret_val = send_cbs_result_invalid;
    bool motor_done = false;

    if(!gpio_read(MT_TAPE_GPIO_PIN_NR_MOTOR))
    {
        console_deb_writeline("send_cbs: Motor off. Waiting (1)..");

        while(!gpio_read(MT_TAPE_GPIO_PIN_NR_MOTOR) && s_stop == 0)
        {
            if(detect_signal && is_signal_detected())
            {
                console_deb_writeline("\nsend_cbs: Signal detected (1)!");
                ret_val = send_cbs_result_fast_mode_detected;
                goto send_cbs_done;   
            }
        }
    }
    if(s_stop != 0)
    {
        console_deb_writeline("\nsend_cbs: Stopping (1)..");
        ret_val = send_cbs_result_stopped;
        goto send_cbs_done;
    }

    // Start header data tranfer to CBM via MT_TAPE_GPIO_PIN_READ:
    //
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

            if(detect_signal && is_signal_detected())
            {
                console_deb_writeline("\nsend_cbs: Signal detected (2)!");
                ret_val = send_cbs_result_fast_mode_detected;
                goto send_cbs_done;   
            }
        }
    }
    if(s_stop != 0)
    {
        console_deb_writeline("\nsend_cbs: Stopping (2)..");
        ret_val = send_cbs_result_stopped;
        goto send_cbs_done;
    }
    ProgressBar_print( // Cheating..
        1, header_cbs_count, header_cbs_count, s_progress_bar_len, true);
    printf("\n");

    // [dma_stop() must not be called, here (maybe improve this, later..)]

    // Start content data tranfer to CBM via MT_TAPE_GPIO_PIN_READ:
    //
    dma_start(1 + header_cbs_count); // Sends content once. // TODO: Hard-coded offset!
    console_deb_writeline("send_cbs: Waiting for sending content to finish..");

    {
        uint32_t const first_addr = dma_get_bus_addr_from_vc_ptr(content_cbs);
        uint32_t const last_addr = first_addr + 32 * (content_cbs_count - 1);
        uint32_t left_cbs = UINT32_MAX; // Value must be high enough, only.

        while(dma_is_busy() && s_stop == 0)
        {
            if(gpio_read(MT_TAPE_GPIO_PIN_NR_MOTOR))
            {
                uint32_t const next_cb_addr = dma_get_next_control_block_addr();

                if(first_addr <= next_cb_addr && next_cb_addr <= last_addr)
                {
                    left_cbs = (last_addr - next_cb_addr) / 32 + 1;

                    ProgressBar_print(
                        1,
                        1 + (next_cb_addr - first_addr) / 32,
                        content_cbs_count,
                        s_progress_bar_len,
                        true);
                }

                if(detect_signal && is_signal_detected())
                {
                    console_deb_writeline("\nsend_cbs: Signal detected (3)!");
                    ret_val = send_cbs_result_fast_mode_detected;
                    goto send_cbs_done;   
                }
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

            // Sometimes the loading is finished, but we are stuck, here
            // (maybe because the first motor-off was not detected, because of
            // OS scheduling?). That is why we stop waiting for the motor and
            // resume sending, if the data was already completely transferred
            // and there are only pulses for the last transmit block gap left.
            // Because of motor_done == true, the loop will be left successfully
            // after that.
            //
            while(
                !gpio_read(MT_TAPE_GPIO_PIN_NR_MOTOR) 
                && s_stop == 0
                && left_cbs > 2 * 6 * 60 / 2) // Hard-coded, see tape_fill_buf.c and transmit_block_gap_pulse_count!
            {
                if(detect_signal && is_signal_detected())
                {
                    console_deb_writeline("\nsend_cbs: Signal detected (4)!");
                    ret_val = send_cbs_result_fast_mode_detected;
                    goto send_cbs_done;   
                }
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
        console_deb_writeline("\nsend_cbs: Stopping (3)..");
        ret_val = send_cbs_result_stopped;
        goto send_cbs_done;
    }
    ProgressBar_print( // Cheating..
        1, content_cbs_count, content_cbs_count, s_progress_bar_len, true);
    printf("\n");

    console_deb_writeline("send_cbs: Sending done.");
    assert(ret_val == send_cbs_result_invalid);
    ret_val = send_cbs_result_finished;

send_cbs_done:
    assert(ret_val != send_cbs_result_invalid);

    dma_stop();

    console_deb_writeline(
        "send_cbs: Setting tape read line to HIGH at CBM..");
    gpio_set_output(MT_TAPE_GPIO_PIN_NR_READ, !true);
    //
    // (inverted, because circuit inverts signal to CBM)
    
    return ret_val;  
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
        if(ret_val == NULL)
        {
            assert(false); // Must not happen.
            *out_cbs_count = -1;
            return ret_val;
        }
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
    bool const infinitely,
    bool * const out_fast_mode_detected)
{
    bool ret_val = true; // TRUE by default (meaning: No error).
    int symbol_count = 0,
        header_cbs_count = 0,
        content_cbs_count = 0;
    uint8_t* symbols = NULL;
    struct dma_cb * cbs = NULL, 
        * header_cbs = NULL,
        * content_cbs = NULL;
    enum send_cbs_result send_done_reason = send_cbs_result_invalid;

    symbols = create_symbols_from_bytes(bytes, byte_count, name, &symbol_count);
    if(symbols == NULL)
    {
        assert(false); // Should not fail.
        ret_val = false;
        goto send_bytes_done;
    }

    console_write("send_bytes: Symbol count: ");
    console_write_dword_dec(symbol_count);
    console_writeline("");

    assert(symbol_count > MT_HEADERDATABLOCK_LEN); // Not a real check..

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
        // Can happen, if there is not enough (Video Core) memory.

        ret_val = false;
        goto send_bytes_done;
    }

    console_writeline("send_bytes: DMA is initialized.");

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
        assert(false); // Must not happen.
        ret_val = false;
        goto send_bytes_done;
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
        assert(false); // Must not happen.
        ret_val = false;
        goto send_bytes_done;
    }

    console_write("send_bytes: Content CBS count: ");
    console_write_dword_dec(content_cbs_count);
    console_writeline("");

    // Free stuff that already can be freed:
    //
    s_data_cb = NULL;
    alloc_free(symbols);
    symbols = NULL;
    symbol_count = 0;

    if(!infinitely)
    {
        console_writeline(
            "send_bytes: Power-on Commodore, start LOAD command and press ENTER key.");
        getchar();
    }

    // Simulate the PLAY button to be pressed by the user on the tape drive:
    //
    console_deb_writeline(
        "send_bytes: Setting sense output line to LOW at CBM..");
    gpio_write(MT_TAPE_GPIO_PIN_NR_SENSE, !false);
    //
    // (inverted, because circuit inverts signal to CBM)

    s_stop = 0;
    if(signal(SIGINT, signal_handler) == SIG_ERR)
    {
        assert(false); // Must not happen.
        ret_val = false;
        goto send_bytes_done;
    }

    if(infinitely)
    {
        console_writeline(
            "send_bytes: Starting infinite sending (press CTRL+C to exit/stop)..");

        init_signal_detect();

        do
        {
            send_done_reason = send_cbs(
                    header_cbs,
                    header_cbs_count,
                    content_cbs,
                    content_cbs_count,
                    true);

            if(send_done_reason != send_cbs_result_finished)
            {
                assert(send_done_reason != send_cbs_result_invalid);

                // Either fast mode or stop signal received.
                
                goto send_bytes_done;
            }

            usleep(10 * 20 * 1000); // (see kernel_main.c)
        }while(s_stop == 0);

        console_writeline("send_bytes: Stopping send loop and exiting..");
    }
    else
    {
        console_writeline(
            "send_bytes: Starting sending (press CTRL+C to exit/stop)..");

        send_done_reason = send_cbs(
            header_cbs,
            header_cbs_count,
            content_cbs,
            content_cbs_count,
            false);
    }

send_bytes_done:
    deinit_signal_detect(); // (does not matter, if initialized or not)

    alloc_free(symbols); // (works with NULL)
    //symbols = NULL;
    //symbol_count = 0;

    console_deb_writeline(
        "send_bytes: Setting sense output line to HIGH at CBM..");
    gpio_write(MT_TAPE_GPIO_PIN_NR_SENSE, !true);
    //
    // (inverted, because circuit inverts signal to CBM)

    s_data_cb = NULL;
    dma_deinit();
    if(out_fast_mode_detected != NULL
        && send_done_reason == send_cbs_result_fast_mode_detected)
    {
        *out_fast_mode_detected = true;
    }
    return ret_val;
}

static bool send_petload_c64tom(bool * const out_fast_mode_detected)
{
    return send_bytes(
        (uint8_t*)s_petload_c64tom, // *** CONST CAST ***
        sizeof s_petload_c64tom / sizeof *s_petload_c64tom,
        MT_PETLOAD_PRG_NAME_RUN,
        true, // Always enters infinite send loop.
        out_fast_mode_detected);
}

static bool send_petload_pet4(bool * const out_fast_mode_detected)
{
    return send_bytes(
        (uint8_t*)s_petload_pet4, // *** CONST CAST ***
        sizeof s_petload_pet4 / sizeof *s_petload_pet4,
        MT_PETLOAD_PRG_NAME_TAPE_BUF,
        true, // Always enters infinite send loop.
        out_fast_mode_detected);
}

static bool send_petload_pet4tom(bool * const out_fast_mode_detected)
{
    return send_bytes(
        (uint8_t*)s_petload_pet4tom, // *** CONST CAST ***
        sizeof s_petload_pet4tom / sizeof *s_petload_pet4tom,
        MT_PETLOAD_PRG_NAME_RUN,
        true, // Always enters infinite send loop.
        out_fast_mode_detected);
}

/** Send file with given name/path via compatibility mode.
 */
static bool send_file(
    char * const file_name)
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
        false, // Always send a file one time, only.
        NULL); // No fast mode detection (does not make sense).

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
        bool fast_mode_detected = false;

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
                return send_file(argv[2]);
            }

            case 'u':
            {
                if(argc != 2)
                {
                    break;
                }
                return send_petload_c64tom(&fast_mode_detected); // TODO: Enter fast mode, if detected at CBM!
            }
            case 'w':
            {
                if(argc != 2)
                {
                    break;
                }
                return send_petload_pet4(&fast_mode_detected); // TODO: Enter fast mode, if detected at CBM!
            }
            case 'r':
            {
                if(argc != 2)
                {
                    break;
                }
                return send_petload_pet4tom(&fast_mode_detected); // TODO: Enter fast mode, if detected at CBM!
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
        "u = Send C64 TOM wedge via compatibility mode in a loop."
        "\n"
        "w = Send PET BASIC v4 wedge via compatibility mode in a loop."
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

//     // TODO: Debugging:
//     //
// #ifndef NDEBUG
//     {
//         uint64_t const deb_test = get_tick_us();
    
//         printf("%llu / 0x%X%X\n", deb_test, (uint32_t)(deb_test >> 32), (uint32_t)(deb_test & 0xFFFFFFFF));

//         //usleep(100);

//         uint64_t const deb_test2 = get_tick_us();

//         printf("%llu / 0x%X%X\n", deb_test2, (uint32_t)(deb_test2 >> 32), (uint32_t)(deb_test2 & 0xFFFFFFFF));

//         printf("%llu\n", deb_test2 - deb_test);
//     }
// #endif //NDEBUG

    deinit();

    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
