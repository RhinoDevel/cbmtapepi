
// Marcel Timm, RhinoDevel, 2018nov14

#include <stdbool.h>

#include "ui.h"
#include "ui_send_sample.h"
#include "ui_receive_test.h"
#include "ui_send_test.h"
#include "ui_terminal_to_commodore.h"
#include "ui_commodore_to_terminal.h"
#include "../../lib/console/console.h"
#include "../../hardware/watchdog/watchdog.h"

static void reset()
{
    watchdog_reset_system();
}

void ui_enter()
{
    while(true)
    {
        char c[2];

        console_writeline("f - Transfer from terminal to Commodore.");
        console_writeline("t - Transfer from Commodore to terminal.");
        console_writeline("s - Send a sample (to Commodore).");
        console_writeline("e - Receive test (from terminal, via XMODEM).");
        console_writeline("n - Send test (to terminal, via YMODEM).");
        console_writeline("r - Reset!");
        console_write("? ");
        console_read(c, 2);
        console_writeline("");

        switch(c[0])
        {
            case 'f':
                ui_terminal_to_commodore(true);
                break;
            case 't':
                ui_commodore_to_terminal(true);
                break;
            case 's':
                ui_send_sample();
                break;
            case 'e':
                ui_receive_test();
                break;
            case 'n':
                ui_send_test();
                break;
            case 'r':
                reset();
                break;

            default:
                console_writeline("Please select a menu entry.");
                break;
        }
    }
}
