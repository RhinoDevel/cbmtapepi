
// Marcel Timm, RhinoDevel, 2018dec21

// Hard-coded for MiniUART and XMODEM usage.

#include "ui_receive_test.h"

#include "../console/console.h"
#include "../alloc/alloc.h"
#include "../xmodem/xmodem.h"
#include "../xmodem/xmodem_receive_params.h"
#include "../hardware/miniuart/miniuart.h"

void ui_receive_test()
{
    static uint32_t const len = 64 * 1024; // 64 kB.

    struct xmodem_receive_params p;
    uint32_t count = 0;

    p.write_byte = miniuart_write_byte;
    p.read_byte = miniuart_read_byte;
    p.is_ready_to_read = miniuart_is_ready_to_read;
    p.buf = alloc_alloc(len * sizeof (uint8_t));
    p.len = len;

    console_writeline("Please send your data, now (via XMODEM).");
    while(!xmodem_receive(&p, &count))
    {
        console_writeline("Failed to receive bytes! Retrying..");
    }
    console_write("Received following ");
    console_write_dword_dec(count);
    console_writeline(" bytes:");
    console_writeline("***");
    for(uint32_t i = 0;i < count; ++i)
    {
        console_write_byte(p.buf[i]);
        console_write(" |");
        if(((i + 1) % 32) == 0)
        {
            console_writeline("");
            continue;
        }
        console_write(" ");
    }
    console_writeline("***");

    alloc_free(p.buf);
    p.buf = 0;
}
