
// Marcel Timm, RhinoDevel, 2018nov16

#ifndef MT_TAPE_SEND_PARAMS
#define MT_TAPE_SEND_PARAMS

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct tape_send_params
{
    // Optional function to be able to stop waiting for Commodore and to be able
    // to stop transmission to Commodore:
    //
    bool (*is_stop_requested)();

    uint32_t gpio_pin_nr_read;
    uint32_t gpio_pin_nr_sense;
    uint32_t gpio_pin_nr_motor;

    struct tape_input* data;
};

#ifdef __cplusplus
}
#endif

#endif //MT_TAPE_SEND_PARAMS
