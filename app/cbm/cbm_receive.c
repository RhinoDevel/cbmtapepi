
// Marcel Timm, RhinoDevel, 2019sep12

#include "cbm_receive.h"
#include "../config.h"
#include "../tape/tape_input.h"
#include "../tape/tape_receive_params.h"
#include "../tape/tape_receive.h"

#ifndef NDEBUG
    #include "../../lib/console/console.h"
#endif //NDEBUG

#include <stdbool.h>

struct tape_input * cbm_receive(bool (*is_stop_requested)())
{
    struct tape_input * ret_val;
    struct tape_receive_params const p = {
        .is_stop_requested = is_stop_requested,
        .gpio_pin_nr_write = MT_TAPE_GPIO_PIN_NR_WRITE,
        .gpio_pin_nr_sense = MT_TAPE_GPIO_PIN_NR_SENSE,
        .gpio_pin_nr_motor = MT_TAPE_GPIO_PIN_NR_MOTOR
    };

    ret_val = tape_receive(&p);

#ifndef NDEBUG
    if(ret_val != 0)
    {
        console_write("cbm_receive : Name: \"");
        for(int i = 0;i < 16;++i) // Hard-coded
        {
            console_write_key((char)ret_val->name[i]);
        }
        console_writeline("\".");

        console_write("cbm_receive : Type: ");
        console_write_byte_dec((uint8_t)ret_val->type);
        console_writeline(".");

        console_write("cbm_receive : Address: ");
        console_write_word_dec(ret_val->addr);
        console_writeline(".");

        console_write("cbm_receive : Length: ");
        console_write_word_dec(ret_val->len);
        console_writeline(".");
    }
#endif //NDEBUG

    return ret_val;
}
