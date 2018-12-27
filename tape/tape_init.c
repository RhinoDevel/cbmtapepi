
// Marcel Timm, RhinoDevel, 2018dec26

#include <stdbool.h>

#include "tape_init.h"

#include "../baregpio/baregpio.h"
#include "../console/console.h"
#include "../config.h"

void tape_init()
{
    console_writeline("tape_init: Setting sense line to HIGH..");
    baregpio_set_output(MT_TAPE_GPIO_PIN_NR_SENSE, true);

    console_writeline("tape_init: Setting motor line to input with pull-down..");
    baregpio_set_input_pull_down(MT_TAPE_GPIO_PIN_NR_MOTOR);

    console_writeline("tape_init: Setting tape read line to HIGH..");
    baregpio_set_output(MT_TAPE_GPIO_PIN_NR_READ, true);
}