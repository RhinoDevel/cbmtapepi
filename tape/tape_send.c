
// Marcel Timm, RhinoDevel, 2018nov16

#include <stdbool.h>
#include <stdint.h>

#include "../baregpio/baregpio.h"
#include "../console/console.h"

#include "tape_fill_buf.h"
#include "tape_transfer_buf.h"

#include "tape_send.h"

bool tape_send(struct tape_send_params const * const p, uint32_t * const mem)
{
    // Use memory at given position for sample data to send:

    uint8_t * const buf = (uint8_t *)mem;

    console_writeline("tape_send: Setting sense line to HIGH..");
    baregpio_set_output(p->gpio_pin_nr_sense, true);

    console_writeline("tape_send: Setting motor line to input without pull..");
    baregpio_set_input_pull_off(p->gpio_pin_nr_motor);

    console_writeline("tape_send: Setting tape read line to HIGH..");
    baregpio_set_output(p->gpio_pin_nr_read, true);

    // Get sample data to send:

    console_writeline("tape_send: Filling send buffer from input structure..");
    tape_fill_buf(p->data, buf);

    // Send sample data via GPIO pin with given nr:

    console_writeline("tape_send: Setting sense line to LOW..");
    baregpio_set_output(p->gpio_pin_nr_sense, false);

    console_writeline("tape_send: Sending buffer content..");
    if(tape_transfer_buf(buf, p->gpio_pin_nr_motor, p->gpio_pin_nr_read))
    {
        console_writeline(
            "tape_send: Success. Setting tape read line and sense line to HIGH..");
        baregpio_set_output(p->gpio_pin_nr_read, true);
        baregpio_set_output(p->gpio_pin_nr_sense, true);
        return true;
    }
    console_writeline(
        "tape_send: Failure! Setting tape read line and sense line to HIGH..");
    baregpio_set_output(p->gpio_pin_nr_read, true);
    baregpio_set_output(p->gpio_pin_nr_sense, true);
    return false;
}
