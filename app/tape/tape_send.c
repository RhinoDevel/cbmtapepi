
// Marcel Timm, RhinoDevel, 2018nov16

#include <stdbool.h>
#include <stdint.h>

#include "../../hardware/baregpio/baregpio.h"
#include "../../lib/console/console.h"

#include "tape_fill_buf.h"
#include "tape_send_buf.h"

#include "tape_send.h"

bool tape_send(struct tape_send_params const * const p, uint32_t * const mem)
{
    // Use memory at given position for data to send:

    uint8_t * const buf = (uint8_t *)mem;

    // Create data to send from given:

    console_deb_writeline(
        "tape_send: Filling send buffer from input structure..");
    tape_fill_buf(p->data, buf);

    // Send data via GPIO pin with given nr:

    console_deb_writeline("tape_send: Setting sense line to LOW..");
    baregpio_set_output(p->gpio_pin_nr_sense, false);

    console_deb_writeline("tape_send: Sending buffer content..");
    if(tape_send_buf(
        buf, p->gpio_pin_nr_motor, p->gpio_pin_nr_read, p->is_stop_requested))
    {
        console_deb_writeline(
            "tape_send: Success. Setting tape read line and sense line to HIGH..");
        baregpio_set_output(p->gpio_pin_nr_read, true);
        baregpio_set_output(p->gpio_pin_nr_sense, true);
        return true;
    }
    console_deb_writeline(
        "tape_send: Failure! Setting tape read line and sense line to HIGH..");
    baregpio_set_output(p->gpio_pin_nr_read, true);
    baregpio_set_output(p->gpio_pin_nr_sense, true);
    return false;
}
