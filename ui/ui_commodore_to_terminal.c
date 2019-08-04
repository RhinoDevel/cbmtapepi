
// Marcel Timm, RhinoDevel, 2019jul16

// Hard-coded for MiniUART and YMODEM usage.
//
// Warning: console_... is probably linked to MiniUART, too!

#include "ui_commodore_to_terminal.h"

#include "../alloc/alloc.h"
#include "../config.h"
#include "../console/console.h"
#include "../tape/tape_receive.h"
#include "../tape/tape_input.h"
#include "../tape/tape_receive_params.h"
#include "../ymodem/ymodem_send_params.h"
#include "../ymodem/ymodem_send_err.h"
#include "../ymodem/ymodem.h"
#include "../miniuart/miniuart.h"
#include "../statetoggle/statetoggle.h"

static void hint()
{
    char c[2];

    console_write(
        "Prepare Commodore, now (e.g. enter SAVE and press Return key). Then press anykey (here)..");
    console_read(c, 2);
    console_writeline("");
}

/**
 * - Caller takes ownership of return value.
 */
static struct tape_input * receive_from_commodore(bool const interactive)
{
    struct tape_input * ret_val;
    struct tape_receive_params const p = {
        .is_stop_requested = interactive
            ? 0
            : statetoggle_is_requested,
        .gpio_pin_nr_write = MT_TAPE_GPIO_PIN_NR_WRITE,
        .gpio_pin_nr_sense = MT_TAPE_GPIO_PIN_NR_SENSE,
        .gpio_pin_nr_motor = MT_TAPE_GPIO_PIN_NR_MOTOR
    };

    if(interactive)
    {
        hint();
    }

    ret_val = tape_receive(&p);

#ifndef NDEBUG
    if(ret_val != 0)
    {
        console_write("receive_from_commodore : Name: \"");
        for(int i = 0;i < 16;++i) // Hard-coded
        {
            console_write_key((char)ret_val->name[i]);
        }
        console_writeline("\".");

        console_write("receive_from_commodore : Type: ");
        console_write_byte_dec((uint8_t)ret_val->type);
        console_writeline(".");

        console_write("receive_from_commodore : Address: ");
        console_write_word_dec(ret_val->addr);
        console_writeline(".");

        console_write("receive_from_commodore : Length: ");
        console_write_word_dec(ret_val->len);
        console_writeline(".");
    }
#endif //NDEBUG

    return ret_val;
}

void ui_commodore_to_terminal(bool const interactive)
{
    struct tape_input * input = receive_from_commodore(interactive);
    struct ymodem_send_params p = {
        .write_byte = miniuart_write_byte,
        .read_byte = miniuart_read_byte,
        .is_ready_to_read = miniuart_is_ready_to_read,
        .is_stop_requested = interactive
            ? 0
            : statetoggle_is_requested,
        //.buf // See below.
        .file_len = input->len + 2
        //.name // See below.
    };
    uint32_t i = 0;
    enum ymodem_send_err err;

    // Address:
    //
    p.buf = alloc_alloc(p.file_len);
    p.buf[i] = (uint8_t)(input->addr & 0x00FF); // Low byte.
    ++i;
    p.buf[i] = (uint8_t)(input->addr >> 8); // High byte.
    ++i;

    // Actual binary data:
    //
    while(i < p.file_len)
    {
        p.buf[i] = input->bytes[i - 2];
        ++i;
    }

    // Name:
    //
    for(i = 0;i < sizeof p.name - 1 && i < sizeof input->name;++i)
    {
        p.name[i] = input->name[i]; // TODO: PETSCII to ASCII conversion!
    }
    while(i < sizeof p.name)
    {
        p.name[i] = '\0';
        ++i;
    }

    console_writeline("Please trigger receival, now (via YMODEM).");
    err = ymodem_send(&p);

    console_write("YMODEM send error code was ");
    console_write_dword_dec((uint32_t)err);
    console_writeline(".");

    alloc_free(p.buf);
    alloc_free(input->bytes);
    alloc_free(input);
}
