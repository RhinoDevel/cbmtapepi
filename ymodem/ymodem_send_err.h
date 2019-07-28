
// Marcel Timm, RhinoDevel, 2019jul20

#ifndef MT_YMODEM_SEND_ERR
#define MT_YMODEM_SEND_ERR

enum ymodem_send_err
{
    ymodem_send_err_none = 0,
    ymodem_send_err_unknown = 1,
    ymodem_send_err_nak = 2,
    ymodem_send_err_end_ack = 3,
    ymodem_send_err_end_nak = 4,
    ymodem_send_err_checksum_meta_ack = 5,
    ymodem_send_err_checksum_meta_nak = 6,
    ymodem_send_err_checksum_content_ack = 7
};

#endif //MT_YMODEM_SEND_ERR
