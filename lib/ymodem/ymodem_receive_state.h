
// Marcel Timm, RhinoDevel, 2019jan01

#ifndef MT_YMODEM_RECEIVE_STATE
#define MT_YMODEM_RECEIVE_STATE

enum ymodem_receive_state
{
    ymodem_receive_state_error = 0,

    ymodem_receive_state_start_or_end = 1, // 1 byte.
    ymodem_receive_state_block_nr = 2, // 1 byte.
    ymodem_receive_state_inverted_block_nr = 3, // 1 byte.
    ymodem_receive_state_meta = 4, // 128 or 1024 bytes.
    ymodem_receive_state_checksum_meta = 5, // 1 byte.
    ymodem_receive_state_content = 6, // 128 or 1024 bytes.
    ymodem_receive_state_checksum_content = 7 // 1 byte.
};

#endif //MT_YMODEM_RECEIVE_STATE
