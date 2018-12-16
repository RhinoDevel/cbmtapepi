
// Marcel Timm, RhinoDevel, 2018nov14

#include "../console/console.h"
#include "../tape/tape_sample_type.h"
#include "../tape/tape_sample.h"

static void send_sample_pet()
{
    if(tape_sample_send(tape_sample_type_pet))
    {
        console_writeline("PET (ROM v2 and v3) sample send.");
        return;
    }
    console_writeline("Failed to send PET (ROM v2 and v3) sample!");
}

static void send_sample_c64()
{
    if(tape_sample_send(tape_sample_type_c64))
    {
        console_writeline("C64 sample send.");
        return;
    }
    console_writeline("Failed to send C64 sample!");
}

void ui_send_sample()
{
    while(true)
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
            case 'b':
                return; // *** RETURN ***

            default:
                console_writeline("Please select a menu entry.");
                break;
        }
    }
}
