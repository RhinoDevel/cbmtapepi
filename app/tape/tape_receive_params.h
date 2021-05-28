
// Marcel Timm, RhinoDevel, 2019jul11

#ifndef MT_TAPE_RECEIVE_PARAMS
#define MT_TAPE_RECEIVE_PARAMS

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct tape_receive_params
{
    // Optional function to be able to stop waiting for Commodore and to be able
    // to stop transmission from Commodore:
    //
    bool (*is_stop_requested)();

    uint32_t gpio_pin_nr_write;
    uint32_t gpio_pin_nr_sense;
    uint32_t gpio_pin_nr_motor;
};

#ifdef __cplusplus
}
#endif

#endif //MT_TAPE_RECEIVE_PARAMS
