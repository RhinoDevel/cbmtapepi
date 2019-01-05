
// Marcel Timm, RhinoDevel, 2018dec31

#ifndef MT_YMODEM_RECEIVE_PARAMS
#define MT_YMODEM_RECEIVE_PARAMS

#include <stdint.h>
#include <stdbool.h>

struct ymodem_receive_params
{
    void (*write_byte)(uint8_t const byte);
    uint8_t (*read_byte)();
    bool (*is_ready_to_read)();
    uint8_t * buf;
    uint32_t len;
};

#endif //MT_YMODEM_RECEIVE_PARAMS
