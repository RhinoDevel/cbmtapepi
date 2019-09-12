
// Marcel Timm, RhinoDevel, 2019jul16

// Hard-coded for MiniUART and YMODEM usage.

#include "ui_commodore_to_terminal.h"

#include "../../lib/alloc/alloc.h"
#include "../../lib/console/console.h"
#include "../tape/tape_input.h"
#include "../cbm/cbm_receive.h"
#include "../../lib/ymodem/ymodem_send_params.h"
#include "../../lib/ymodem/ymodem_send_err.h"
#include "../../lib/ymodem/ymodem.h"
#include "../../hardware/miniuart/miniuart.h"
#include "../statetoggle/statetoggle.h"

static void hint()
{
    char c[2];

    console_write(
        "Prepare Commodore, now (e.g. enter SAVE and press Return key). Then press anykey (here)..");
    console_read(c, 2);
    console_writeline("");
}

bool ui_commodore_to_terminal(bool const interactive)
{
    struct ymodem_send_params p;
    uint32_t i;
    enum ymodem_send_err err;

    if(interactive)
    {
        hint();
    }

    struct tape_input * const input = cbm_receive(
        interactive
            ? 0
            : statetoggle_is_requested);

    if(input == 0)
    {
        return false;
    }

    p.write_byte = miniuart_write_byte;
    p.read_byte = miniuart_read_byte;
    p.is_ready_to_read = miniuart_is_ready_to_read;
    p.is_stop_requested = interactive ? 0 : statetoggle_is_requested;
    //.buf // See below.
    p.file_len = input->len + 2;
    //.name // See below.

    i = 0;

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

    if(interactive)
    {
        console_writeline("Please trigger receival, now (via YMODEM).");
    }

    err = ymodem_send(&p);

#ifndef NDEBUG
    console_write("ui_commodore_to_terminal : YMODEM send error code was ");
    console_write_dword_dec((uint32_t)err);
    console_writeline(".");
#endif //NDEBUG

    alloc_free(p.buf);
    alloc_free(input->bytes);
    alloc_free(input);

    return err == ymodem_send_err_none;
}
