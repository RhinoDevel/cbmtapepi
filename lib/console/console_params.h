
// Marcel Timm, RhinoDevel, 2018oct25

#ifndef MT_CONSOLE_PARAMS
#define MT_CONSOLE_PARAMS

#include <stdint.h>
#include <stdbool.h>

struct console_params
{
    uint8_t (*read_byte)();
    void (*write_byte)(uint8_t const byte);
    bool write_newline_with_cr; // false => '\n', true => '\r\n'
};

#endif //MT_CONSOLE_PARAMS
