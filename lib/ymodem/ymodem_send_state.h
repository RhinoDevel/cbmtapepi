
// Marcel Timm, RhinoDevel, 2019jul20

#ifndef MT_YMODEM_SEND_STATE
#define MT_YMODEM_SEND_STATE

enum ymodem_send_state
{
    ymodem_send_state_error = 0,
    ymodem_send_state_start = 1,
    ymodem_send_state_block_nr = 2,
    ymodem_send_state_inverted_block_nr = 3,
    ymodem_send_state_meta = 4,
    ymodem_send_state_checksum_meta = 5,
    ymodem_send_state_content = 6,
    ymodem_send_state_checksum_content = 7,
    ymodem_send_state_end = 8
};

#endif //MT_YMODEM_SEND_STATE
