
// Marcel Timm, RhinoDevel, 2019aug04

#include "statetoggle.h"

#include "../../lib/console/console.h"
#include "../../hardware/gpio/gpio.h"
#include "../../hardware/armtimer/armtimer.h"

#include <stdbool.h>
#include <stdint.h>

static bool s_toggle_requested = false;
static bool s_state = false;
static uint32_t s_gpio_pin_nr_button = 0;
static uint32_t s_gpio_pin_nr_led = 0;

static void update_led()
{
    gpio_set_output(s_gpio_pin_nr_led, s_state);
}

void statetoggle_init(
    uint32_t const gpio_pin_nr_button,
    uint32_t const gpio_pin_nr_led,
    bool const initial_state)
{
    s_toggle_requested = false;
    s_state = initial_state;
    s_gpio_pin_nr_button = gpio_pin_nr_button;
    s_gpio_pin_nr_led = gpio_pin_nr_led;

    console_deb_writeline(
        "statetoggle_init: Setting button line to input with pull-down..");
    gpio_set_input_pull_down(s_gpio_pin_nr_button);

    console_deb_writeline(
        "statetoggle_init: Setting LED output line to reflect state..");
    update_led();

    armtimer_start_one_mhz();
}

bool statetoggle_is_requested()
{
    s_toggle_requested = s_toggle_requested
                            || gpio_read(s_gpio_pin_nr_button);

    return s_toggle_requested;
}

void statetoggle_toggle()
{
    static uint32_t const low_microseconds = 200;
    uint32_t i = 0;

    s_toggle_requested = false;
    s_state = !s_state;

    do
    {
        armtimer_busywait_microseconds(1);

        if(gpio_read(s_gpio_pin_nr_button))
        {
            i = 0;
        }
        ++i;
    }while(i < low_microseconds);

    update_led();
}

bool statetoggle_get_state()
{
    return s_state;
}
