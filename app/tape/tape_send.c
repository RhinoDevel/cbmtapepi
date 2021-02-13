
// Marcel Timm, RhinoDevel, 2018nov16

#include <stdbool.h>
#include <stdint.h>

#include "../../hardware/gpio/gpio.h"
#include "../../lib/console/console.h"

#include "tape_fill_buf.h"
#include "tape_send_buf.h"
#include "tape_input.h"
#include "tape_send.h"

bool tape_send(struct tape_send_params const * const p, uint32_t * const mem)
{
    // Use memory at given position for data to send:

    uint8_t * const buf = (uint8_t *)mem;

#ifndef NDEBUG
    console_writeline("tape_send : Given tape_input content:");
    tape_input_console_write(p->data, false);
    console_writeline("");
#endif //NDEBUG

    // Create data to send from given:

    console_deb_writeline(
        "tape_send: Filling send buffer from input structure..");
    tape_fill_buf(p->data, buf);

    // Send data via GPIO pin with given nr:

    console_deb_writeline("tape_send: Setting sense line to LOW at CBM..");
    gpio_set_output(p->gpio_pin_nr_sense, !false);
    //
    // (inverted, because circuit inverts signal to CBM)

    console_deb_writeline("tape_send: Sending buffer content..");
    if(tape_send_buf(
        buf, p->gpio_pin_nr_motor, p->gpio_pin_nr_read, p->is_stop_requested))
    {
        console_deb_writeline(
            "tape_send: Success. Setting tape read line and sense line to HIGH at CBM..");
        gpio_set_output(p->gpio_pin_nr_read, !true);
        gpio_set_output(p->gpio_pin_nr_sense, !true);
        //
        // (inverted, because circuit inverts signal to CBM)

        return true;
    }
    console_deb_writeline(
        "tape_send: Failure! Setting tape read line and sense line to HIGH at CBM..");
    gpio_set_output(p->gpio_pin_nr_read, !true);
    gpio_set_output(p->gpio_pin_nr_sense, !true);
    //
    // (inverted, because circuit inverts signal to CBM)
    
    return false;
}
