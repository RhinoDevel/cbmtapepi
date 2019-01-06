
// Marcel Timm, RhinoDevel, 2018dec31

#ifndef MT_YMODEM_RECEIVE_PARAMS
#define MT_YMODEM_RECEIVE_PARAMS

#include <stdint.h>
#include <stdbool.h>

struct ymodem_receive_params
{
    // Functions for communication:
    //
    void (*write_byte)(uint8_t const byte);
    uint8_t (*read_byte)();
    bool (*is_ready_to_read)();

    uint8_t * buf; // Buffer to fill with received file's content.
    uint32_t buf_len; // Buffer length in byte.

    uint32_t file_len; // To be filled with received file's content byte count.

    char name[16 + 1];
    //
    // To be filled with received file's name's first characters,
    // followed by null termination character.
};

#endif //MT_YMODEM_RECEIVE_PARAMS
