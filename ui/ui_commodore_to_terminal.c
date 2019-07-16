
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

    // TODO: Implement forwarding via YMODEM (1/2):
    //
    // struct ymodem_send_params p;
    // enum ymodem_send_err e;

    // TODO: Implement:
    //
    //receive_from_commodore(..., interactive);

    // TODO: Implement forwarding via YMODEM (2/2):
    //
    // // TODO: Prepare YMODEM send parameters.
    //
    // console_writeline("Please prepare for file retrieval, now (via YMODEM).");
    //
    // e = ymodem_send(&p);
    // if(e != ymodem_send_err_none)
    // {
    //     console_write("Failed to send file (error ");
    //     console_write_dword_dec((uint32_t)e);
    //     console_writeline(")!");
    //
    //     // TODO: Deallocate stuff!
    //     return;
    // }
    //
    // console_write("Send file \"");
    // console_write(p.name);
    // console_write("\" with a length of ");
    // console_write_dword_dec(p.file_len);
    // console_writeline(" bytes.");
    //
    // // TODO: Deallocate stuff!

    alloc_free(input->bytes);
    alloc_free(input);
}
