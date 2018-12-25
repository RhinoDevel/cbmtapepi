
// Marcel Timm, RhinoDevel, 2018nov14

#include <stdbool.h>

#include "ui.h"
#include "ui_send_sample.h"
#include "ui_receive_test.h"
#include "ui_terminal_to_commodore.h"
#include "../console/console.h"
#include "../watchdog/watchdog.h"

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
        console_writeline("s - Send a sample.");
        console_writeline("e - Receive test.");
        console_writeline("r - Reset!");
        console_write("? ");
        console_read(c, 2);
        console_writeline("");

        switch(c[0])
        {
            case 'f':
                ui_terminal_to_commodore();
                break;
            case 's':
                ui_send_sample();
                break;
            case 'e':
                ui_receive_test();
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
