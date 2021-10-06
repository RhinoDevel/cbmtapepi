
// Marcel Timm, RhinoDevel, 2021oct05

#include <stdbool.h>
#include <pigpio.h>

#include "pigpio.h"
#include "../../lib/console/console.h"

bool pigpio_init()
{
    int const result = gpioInitialise();

    if(result == PI_INIT_FAILED)
    {
        console_writeline("pigpio_init : Error: Initialization failed!");
        return false;
    }
    return true;
}

int pigpio_create_wave(
        uint32_t const gpio_pin_nr,
        uint8_t const * const symbols,
        int const symbol_count)
{
    gpioPulse_t p;

    if(gpioWaveAddNew() != 0)
    {
        return -1;
    }

    // TODO: Implement correctly:
    //
    p.gpioOn = gpio_pin_nr;
    p.gpioOff = 0;
    p.usDelay = 176; // us
    //
    if(gpioWaveAddGeneric(1, &p) == PI_TOO_MANY_PULSES)
    {
        return -1;
    }

    int const wave_id = gpioWaveCreate();

    if(wave_id < 0)
    {
        return -1;
    }
    return wave_id;
}
