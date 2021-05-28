
// Marcel Timm, RhinoDevel, 2018dec26

#include <stdbool.h>

#include "tape_init.h"
#include "tape_receive_buf.h"
#include "tape_send_buf.h"

#include "../../hardware/gpio/gpio.h"
#include "../../lib/console/console.h"
#include "../config.h"

void tape_init(
    void (*timer_start_one_mhz)(),
    uint32_t (*timer_get_tick)(),
    void (*timer_busywait_microseconds)(uint32_t const microseconds))
{
    tape_receive_buf_init(timer_start_one_mhz, timer_get_tick);

    tape_send_buf_init(timer_busywait_microseconds, gpio_write, gpio_read);

    console_deb_writeline("tape_init: Setting sense output line to HIGH at CBM..");
    gpio_set_output(MT_TAPE_GPIO_PIN_NR_SENSE, !true);
    //
    // (inverted, because circuit inverts signal to CBM)

    console_deb_writeline("tape_init: Setting motor line to input with pull-down..");
    gpio_set_input_pull_down(MT_TAPE_GPIO_PIN_NR_MOTOR);

    console_deb_writeline("tape_init: Setting tape read output line to HIGH at CBM..");
    gpio_set_output(MT_TAPE_GPIO_PIN_NR_READ, !true);
    //
    // (inverted, because circuit inverts signal to CBM)

    console_deb_writeline(
        "tape_init: Setting tape write line to input with pull-down..");
    gpio_set_input_pull_down(MT_TAPE_GPIO_PIN_NR_WRITE);
}
