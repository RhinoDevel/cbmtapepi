
// Marcel Timm, RhinoDevel, 2018nov14

#include "../console/console.h"

static void send_sample_pet()
{
    console_writeline("PET (ROM v2 and v3) sample send.");
}

static void send_sample_c64()
{
    console_writeline("C64 sample send.");
}

void ui_send_sample()
{
    char c[2];

    console_writeline("p - Send PET (ROM v2 and v3) sample.");
    console_writeline("c - Send C64 sample.");
    console_writeline("b - Back.");
    console_write("? ");
    console_read(c, 2);
    console_writeline("");

    switch(c[0])
    {
        case 'p':
            send_sample_pet();
            break;
        case 'c':
            send_sample_c64();
            break;
        default:
            console_writeline("Please select a menu entry.");
            break;
    }
}
