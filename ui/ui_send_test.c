
// Marcel Timm, RhinoDevel, 2019jul20

// Hard-coded for MiniUART and YMODEM usage.
//
// Warning: console_... is probably linked to MiniUART, too!

#include "ui_send_test.h"

#include "../console/console.h"
#include "../alloc/alloc.h"
#include "../ymodem/ymodem.h"
#include "../ymodem/ymodem_send_params.h"
#include "../ymodem/ymodem_send_err.h"
#include "../miniuart/miniuart.h"

void ui_send_test()
{
    static uint32_t const len = 26;
    uint32_t i;

    struct ymodem_send_params p = {
        .write_byte = miniuart_write_byte,
        .read_byte = miniuart_read_byte,
        .is_ready_to_read = miniuart_is_ready_to_read,
        .buf = alloc_alloc(len * sizeof (uint8_t)),
        .file_len = len
        //.name // See below.
    };
    enum ymodem_send_err err;

    // Name == "0123456789\0\0\0\0\0\0" (one more '\0' implied).
    //
    for(i = 0;i < 10;++i)
    {
        p.name[i] = '0' + i;
    }
    while(i < sizeof p.name)
    {
        p.name[i] = '\0';
        ++i;
    }

    // Buffer content == 'abcdefghijklmnopqrstuvwxyz' (no \0 implied).
    //
    for(i = 0;i < p.file_len;++i)
    {
        p.buf[i] = 'a' + i;
    }

    console_writeline("Please trigger receival, now (via YMODEM).");
    err = ymodem_send(&p);
    console_write("ymodem_send() error code was ");
    console_write_dword_dec((uint32_t)err);
    console_writeline(" .");

    alloc_free(p.buf);
}
