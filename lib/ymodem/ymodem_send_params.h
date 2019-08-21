
// Marcel Timm, RhinoDevel, 2019jul20

#ifndef MT_YMODEM_SEND_PARAMS
#define MT_YMODEM_SEND_PARAMS

#include <stdint.h>
#include <stdbool.h>

struct ymodem_send_params
{
    // Functions for communication:
    //
    void (*write_byte)(uint8_t const byte);
    uint8_t (*read_byte)();
    bool (*is_ready_to_read)();

    // Optional function to be able to stop waiting for receiver:
    //
    bool (*is_stop_requested)();

    uint8_t * buf; // Buffer to fill with file's content.
    uint32_t file_len; // File's content byte count AND length of buffer.

    char name[16 + 1];
    //
    // To be filled with file's name's first characters,
    // followed by null termination character.
};

#endif //MT_YMODEM_SEND_PARAMS
