
// Marcel Timm, RhinoDevel, 2019aug04

#ifndef MT_STATETOGGLE
#define MT_STATETOGGLE

#include <stdint.h>
#include <stdbool.h>

void statetoggle_init(
    uint32_t const gpio_pin_nr_button,
    uint32_t const gpio_pin_nr_led,
    bool const initial_state);

bool statetoggle_is_requested();

void statetoggle_toggle();

bool statetoggle_get_state();

#endif //MT_STATETOGGLE
