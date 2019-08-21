
// Marcel Timm, RhinoDevel, 2019jan06

#ifndef MT_YMODEM_RECEIVE_ERR
#define MT_YMODEM_RECEIVE_ERR

enum ymodem_receive_err
{
    ymodem_receive_err_none = 0,
    ymodem_receive_err_unknown = 1,
    ymodem_receive_err_start_or_end = 2,
    ymodem_receive_err_block_nr = 3,
    ymodem_receive_err_inverted_block_nr = 4,
    ymodem_receive_err_meta_data = 5,
    ymodem_receive_err_meta_len = 6,
    ymodem_receive_err_checksum_meta = 7,
    ymodem_receive_err_checksum_content = 8,
    ymodem_receive_err_stop_requested = 9 // Not really an error.
};

#endif //MT_YMODEM_RECEIVE_ERR
