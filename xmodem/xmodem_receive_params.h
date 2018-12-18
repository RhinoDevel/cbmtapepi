
// Marcel Timm, RhinoDevel, 2018dec18

#ifndef MT_XMODEM_RECEIVE_PARAMS
#define MT_XMODEM_RECEIVE_PARAMS

#include <stdint.h>
#include <stdbool.h>

struct xmodem_receive_params
{
    void (*write_byte)(uint8_t const byte);
    uint8_t (*read_byte)();
    bool (*is_ready_to_read)();
    uint8_t * const buf;
    uint32_t const len;
};

#endif //MT_XMODEM_RECEIVE_PARAMS
