
// Marcel Timm, RhinoDevel, 2019jul11

#include <stdbool.h>
#include <stdint.h>

#include "../hardware/baregpio/baregpio.h"
#include "../console/console.h"
#include "../alloc/alloc.h"

#include "tape_receive.h"
#include "tape_receive_buf.h"
#include "tape_extract_buf.h"

struct tape_input * tape_receive(struct tape_receive_params const * const p)
{
    uint8_t * const buf = alloc_alloc(4 * 1024 * 1024); // Hard-coded

    if(buf == 0)
    {
        console_deb_writeline(
            "tape_receive: Error: Failed to allocate buffer memory!");
        return 0;
    }

    // Receive data via GPIO pin with given nr:

    console_deb_writeline("tape_receive: Setting sense line to LOW..");
    baregpio_set_output(p->gpio_pin_nr_sense, false);

    console_deb_writeline("tape_receive: Receiving data..");
    if(tape_receive_buf(
        p->gpio_pin_nr_motor, p->gpio_pin_nr_write, buf, p->is_stop_requested))
    {
        struct tape_input * input;

        console_deb_writeline(
            "tape_receive: Success. Setting sense line to HIGH..");
        baregpio_set_output(p->gpio_pin_nr_sense, true);

        input = tape_extract_buf(buf);
        alloc_free(buf);
        return input;
    }
    console_deb_writeline(
        "tape_receive: Failure! Setting sense line to HIGH..");
    baregpio_set_output(p->gpio_pin_nr_sense, true);
    alloc_free(buf);
    return 0;
}
