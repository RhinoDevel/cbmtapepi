
// Marcel Timm, RhinoDevel, 2019jul20

#ifndef MT_YMODEM_SEND_ERR
#define MT_YMODEM_SEND_ERR

enum ymodem_send_err
{
    ymodem_send_err_none = 0,
    ymodem_send_err_unknown = 1,
    ymodem_send_err_nak = 2,
    ymodem_send_err_end = 3,
    ymodem_send_err_checksum_meta_ack = 4,
    ymodem_send_err_checksum_meta_nak = 5,
    ymodem_send_err_checksum_content_ack = 6
};

#endif //MT_YMODEM_SEND_ERR
