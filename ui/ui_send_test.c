
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
    static uint32_t const len = 26 + 1;
    uint32_t i;

    uint8_t * const debug_buf = alloc_alloc(1024 * 1);
    uint32_t debug_buf_len = 0;

    struct ymodem_send_params p = {
        .write_byte = miniuart_write_byte,
        .read_byte = miniuart_read_byte,
        .is_ready_to_read = miniuart_is_ready_to_read,
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
    err = ymodem_send(&p, debug_buf, &debug_buf_len);
    console_write("ymodem_send() error code was ");
    console_write_dword_dec((uint32_t)err);
    console_writeline(" .");

    console_writeline("***");
    for(i = 0;i < debug_buf_len; ++i)
    {
        console_write_byte(debug_buf[i]);
        if(((i + 1) % 8) == 0)
        {
            console_writeline("");
        }
    }
    console_writeline("***");
    console_write_dword_dec(debug_buf_len);
    console_writeline("***");

    alloc_free(debug_buf);

    alloc_free(p.buf);
}
