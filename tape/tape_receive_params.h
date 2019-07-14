
// Marcel Timm, RhinoDevel, 2019jul11

#ifndef MT_TAPE_RECEIVE_PARAMS
#define MT_TAPE_RECEIVE_PARAMS

#include <stdint.h>

struct tape_receive_params
{
    uint32_t gpio_pin_nr_write;
    uint32_t gpio_pin_nr_sense;
    uint32_t gpio_pin_nr_motor;
};

#endif //MT_TAPE_RECEIVE_PARAMS
