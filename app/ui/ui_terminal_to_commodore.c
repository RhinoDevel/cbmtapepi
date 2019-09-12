
// Marcel Timm, RhinoDevel, 2018dec24

// Hard-coded for MiniUART and YMODEM usage.

#include "ui_terminal_to_commodore.h"

#include "../../lib/console/console.h"
#include "../../lib/alloc/alloc.h"
#include "../../lib/ymodem/ymodem.h"
#include "../../lib/ymodem/ymodem_receive_params.h"
#include "../../lib/ymodem/ymodem_receive_err.h"
#include "../../hardware/armtimer/armtimer.h"
#include "../../hardware/miniuart/miniuart.h"
#include "../statetoggle/statetoggle.h"
#include "../cbm/cbm_send.h"

static void hint()
{
    char c[2];

    console_write(
        "Prepare Commodore, now (e.g. enter LOAD and press Return key). Then press anykey (here)..");
    console_read(c, 2);
    console_writeline("");
}

bool ui_terminal_to_commodore(bool const interactive)
{
    static uint32_t const len = 64 * 1024; // 64 kB.

    struct ymodem_receive_params p;
    enum ymodem_receive_err e;

    p.timer_start_one_mhz = armtimer_start_one_mhz;
    p.timer_get_tick = armtimer_get_tick;
    p.write_byte = miniuart_write_byte;
    p.read_byte = miniuart_read_byte;
    p.is_ready_to_read = miniuart_is_ready_to_read;
    p.is_stop_requested = interactive
        ? 0
        : statetoggle_is_requested;
    p.buf = alloc_alloc(len * sizeof (uint8_t));
    p.buf_len = len;
    p.file_len = 0;
    p.name[0] = '\0';

    if(interactive)
    {
        console_writeline("Please send your file, now (via YMODEM).");
    }

    e = ymodem_receive(&p);
    if(e != ymodem_receive_err_none)
    {
#ifndef NDEBUG
        console_write("ui_terminal_to_commodore : Failed to receive file (error ");
        console_write_dword_dec((uint32_t)e);
        console_writeline(")!");
#endif //NDEBUG

        alloc_free(p.buf);
        p.buf = 0;
        return false;
    }

#ifndef NDEBUG
    console_write("ui_terminal_to_commodore : Received file \"");
    console_write(p.name);
    console_write("\" with a length of ");
    console_write_dword_dec(p.file_len);
    console_writeline(" bytes.");
#endif //NDEBUG

    if(interactive)
    {
        hint();
    }

    bool const send_succeeded = cbm_send(
        p.buf, p.name, p.file_len, p.is_stop_requested);

    alloc_free(p.buf);
    p.buf = 0;
    return send_succeeded;
}
