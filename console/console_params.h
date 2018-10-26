
// Marcel Timm, RhinoDevel, 2018oct25

#ifndef MT_CONSOLE_PARAMS
#define MT_CONSOLE_PARAMS

#include <stdint.h>

struct console_params
{
    uint8_t (*read_byte)();
    void (*write_byte)(uint8_t const byte);
};

#endif //MT_CONSOLE_PARAMS
