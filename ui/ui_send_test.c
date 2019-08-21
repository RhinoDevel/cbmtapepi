
// Marcel Timm, RhinoDevel, 2019jul20

// Hard-coded for MiniUART and YMODEM usage.

#include "ui_send_test.h"

#include "../lib/console/console.h"
#include "../lib/alloc/alloc.h"
#include "../ymodem/ymodem.h"
#include "../ymodem/ymodem_send_params.h"
#include "../ymodem/ymodem_send_err.h"
#include "../hardware/miniuart/miniuart.h"
#include "../statetoggle/statetoggle.h"

void ui_send_test()
{
    static uint32_t const len = 26 + 1;
    uint32_t i;
    struct ymodem_send_params p = {
        .write_byte = miniuart_write_byte,
        .read_byte = miniuart_read_byte,
        .is_ready_to_read = miniuart_is_ready_to_read,

        .is_stop_requested = 0,
        //
        // If this is set to 0 (or just ignored), memset() must be available to
        // the linker (see lib/mem/mem.h)!

        .buf = alloc_alloc(len * sizeof (uint8_t)),
        .file_len = len
        //.name // See below.
    };
    enum ymodem_send_err err;

    // Name == "123456789\0\0\0\0\0\0\0" (one more '\0' implied).
    //
    for(i = 0;i < 9;++i)
    {
        p.name[i] = '0' + i + 1;
    }
    while(i < sizeof p.name)
    {
        p.name[i] = '\0';
        ++i;
    }

    // Buffer content == 'abcdefghijklmnopqrstuvwxyz\n' (no \0 implied).
    //
    for(i = 0;i < p.file_len - 1;++i)
    {
        p.buf[i] = 'A' + i;
    }
    p.buf[i] = 0x0A; // LF

    console_writeline("Please trigger receival, now (via YMODEM).");
    err = ymodem_send(&p);

    console_write("ymodem_send() error code was ");
    console_write_dword_dec((uint32_t)err);
    console_writeline(".");

    alloc_free(p.buf);
}
