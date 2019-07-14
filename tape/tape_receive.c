
// Marcel Timm, RhinoDevel, 2019jul11

#include <stdbool.h>
#include <stdint.h>

#include "../baregpio/baregpio.h"
#include "../console/console.h"

#include "tape_receive.h"
#include "tape_receive_buf.h"
#include "tape_extract_buf.h"

bool tape_receive(
    struct tape_receive_params const * const p, uint32_t * const mem)
{
    // Use memory at given position as buffer during receival:

    uint8_t * const buf = (uint8_t *)mem;

    // Receive data via GPIO pin with given nr:

    console_writeline("tape_receive: Setting sense line to LOW..");
    baregpio_set_output(p->gpio_pin_nr_sense, false);

    console_writeline("tape_receive: Receiving data..");
    if(tape_receive_buf(p->gpio_pin_nr_motor, p->gpio_pin_nr_write, buf))
    {
        console_writeline(
            "tape_receive: Success. Setting sense line to HIGH..");
        baregpio_set_output(p->gpio_pin_nr_sense, true);

        return tape_extract_buf(buf, p->data);
    }
    console_writeline(
        "tape_receive: Failure! Setting sense line to HIGH..");
    baregpio_set_output(p->gpio_pin_nr_sense, true);
    return false;
}
