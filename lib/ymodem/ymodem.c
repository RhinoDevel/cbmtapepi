
// Marcel Timm, RhinoDevel, 2018dec31

#include <stdint.h>
#include <stdbool.h>

#include "../alloc/alloc.h"
#include "../calc/calc.h"
//#include "../assert.h"
#include "ymodem_send_params.h"
#include "ymodem_send_state.h"
#include "ymodem_send_err.h"
#include "ymodem_receive_params.h"
#include "ymodem_receive_state.h"
#include "ymodem_receive_err.h"
#include "ymodem.h"

// #ifndef NDEBUG
//     #include "../console/console.h"
// #endif //NDEBUG

// YMODEM documentation at: http://pauillac.inria.fr/~doligez/zmodem/ymodem.txt

// Control characters
// (see: https://en.wikipedia.org/wiki/C0_and_C1_control_codes):
//
static uint8_t const SOH = 0x01; // Start of heading for 128 byte data block.
static uint8_t const STX = 0x02; // Start of heading for 1024 byte data block.
static uint8_t const ACK = 0x06; // Acknowledge
static uint8_t const NAK = 0x15; // Negative acknowledge.
static uint8_t const EOT = 0x04; // End of transmission.
static uint8_t const CAN = 0x18; // Cancel
static uint8_t const CRC = 0x43; // 'C'

static uint8_t const LF = 0x0A; // Line feed.
static uint8_t const CR = 0x0D; // Carriage return.
static uint8_t const SUB = 0x1A; // CPMEOF

// Hard-coded for timer being set to 1 million ticks per second (see below):
//
static uint32_t const NAKSOH_TIMEOUT = 4000000;
static uint32_t const WAIT_AFTER_EOT_TIMEOUT = 2000000;
static uint32_t const WAIT_AFTER_CAN_TIMEOUT = 2000000;

static uint16_t get_crc(uint8_t const byte, uint16_t const crc)
{
    uint16_t ret_val = crc ^ (uint16_t)byte << 8;

    for (int i = 0;i < 8;++i)
    {
        if(ret_val & 0x8000)
        {
            ret_val = ret_val << 1 ^ 0x1021;
        }
        else
        {
            ret_val = ret_val << 1;
        }
    }
    return ret_val;
}

enum ymodem_send_err ymodem_send(struct ymodem_send_params * const p)
{
    uint16_t const block_size = 128;
    enum ymodem_send_err err = ymodem_send_err_unknown;
    enum ymodem_send_state state = ymodem_send_state_error;
    uint8_t sb, csum, block_nr = 0;
    uint16_t crc;
    uint32_t offset = 0;
    bool send_null_file = false,
        use_crc = false;

    // Waiting for stop request, NAK or CRC:
    //
    if(p->is_stop_requested != 0)
    {
        while(!p->is_ready_to_read())
        {
            if(p->is_stop_requested())
            {
                err = ymodem_send_err_stop_requested;
                state = ymodem_send_state_error;
                break;
            }
        }
    }
    if(err != ymodem_send_err_stop_requested)
    {
        uint8_t rb = p->read_byte();
        //
        // (p->read_byte() waits for being ready to read)

// #ifndef NDEBUG
//         console_write("ymodem_send : Read byte ");
//         console_write_byte(rb);
//         console_writeline(" (1).");
// #endif //NDEBUG

        if(rb == NAK)
        {
            state = ymodem_send_state_start;
            use_crc = false;

            while(p->is_ready_to_read())
            {
                rb = p->read_byte();

// #ifndef NDEBUG
//                 console_write("ymodem_send : Read byte ");
//                 console_write_byte(rb);
//                 console_writeline(" (8).");
// #endif //NDEBUG

                if(rb != NAK)
                {
                    state = ymodem_send_state_error;
                    err = ymodem_send_err_nak_flush;
                    break;
                }
            }
        }
        else
        {
            if(rb == CRC)
            {
                state = ymodem_send_state_start;
                use_crc = true;

                while(p->is_ready_to_read())
                {
                    rb = p->read_byte();

// #ifndef NDEBUG
//                     console_write("ymodem_send : Read byte ");
//                     console_write_byte(rb);
//                     console_writeline(" (9).");
// #endif //NDEBUG

                    if(rb != CRC)
                    {
                        state = ymodem_send_state_error;
                        err = ymodem_send_err_crc_flush;
                        break;
                    }
                }
            }
            else
            {
                state = ymodem_send_state_error;
                err = ymodem_send_err_nak;
            }
        }
    }

// #ifndef NDEBUG
//     console_write("ymodem_send : State is ");
//     console_write_byte((uint8_t)state);
//     console_write(", CRC is ");
//     console_write(use_crc ? "enabled" : "disabled");
//     console_writeline(", entering send loop..");
// #endif //NDEBUG

    while(true)
    {
// #ifndef NDEBUG
//             console_write("ymodem_send : State at beginning of send loop is ");
//             console_write_byte((uint8_t)state);
//             console_writeline(".");
// #endif //NDEBUG

        switch(state)
        {
            case ymodem_send_state_start: // Start another block/sector:
            {
                // Send block size as header of current block:
                //
                if(block_size == 128)
                {
                    sb = SOH;
                }
                else
                {
                    //assert(block_size == 1024);

                    sb = STX;
                }
                p->write_byte(sb);
                state = ymodem_send_state_block_nr;

                // Reset checksum/CRC, do not add SOH/STX to checksum/CRC:
                //
                csum = 0;
                crc = 0;

                break;
            }
            case ymodem_send_state_end:
            {
                uint8_t rb;

                //assert(send_null_file);
                sb = EOT;
                p->write_byte(sb);

                // Waiting for ACK:

                rb = p->read_byte();

// #ifndef NDEBUG
//                 console_write("ymodem_send : Read byte ");
//                 console_write_byte(rb);
//                 console_writeline(" (2).");
// #endif //NDEBUG

                if(rb != ACK)
                {
                    state = ymodem_send_state_error;
                    err = ymodem_send_err_end_ack;
                    break;
                }

                // Not sure, if YMODEM supports CRC on/off toggling during
                // transfer, but should do no harm..
                //
                rb = p->read_byte();

// #ifndef NDEBUG
//                 console_write("ymodem_send : Read byte ");
//                 console_write_byte(rb);
//                 console_writeline(" (3).");
// #endif //NDEBUG

                if(rb == NAK)
                {
                    use_crc = false;
                }
                else
                {
                    if(rb == CRC)
                    {
                        use_crc = true;
                    }
                    else
                    {
                        state = ymodem_send_state_error;
                        err = ymodem_send_err_end_nak;
                        break;
                    }
                }

                // Done (triggers sending of empty file):

                block_nr = 0;
                state = ymodem_send_state_start;
                break;
            }

            case ymodem_send_state_block_nr: // Send block number:
            {
                sb = block_nr;
                p->write_byte(sb);
                state = ymodem_send_state_inverted_block_nr;

                // (do not add block nr. to checksum/CRC)

                break;
            }

            case ymodem_send_state_inverted_block_nr: // Send inverted block nr:
            {
                sb = 0xFF - block_nr;
                p->write_byte(sb);

                if(block_nr == 0)
                {
                    state = ymodem_send_state_meta;
                }
                else
                {
                    state = ymodem_send_state_content;
                }

                // (do not add inverted block nr. to checksum/CRC)

                break;
            }

            case ymodem_send_state_meta: // Send meta data:
            {
                //assert(block_nr == 0);

                uint32_t i = 0; // Position in current block.

                if(send_null_file)
                {
                    // End of (complete) transfer, signalize to receiver by
                    // sending an empty/null file name:

                    while(i < block_size)
                    {
                        sb = '\0';
                        p->write_byte(sb);
                        csum += sb;
                        crc = get_crc(sb, crc);

                        ++i;
                    }
                    state = ymodem_send_state_checksum_meta;
                    break;
                }

                // Send meta data of file to transfer:

                // Send terminated file name:
                //
                while((i < sizeof p->name - 1) && (p->name[i] != '\0'))
                {
                    sb = p->name[i];
                    p->write_byte(sb);
                    csum += sb;
                    crc = get_crc(sb, crc);

                    ++i;
                }
                //assert(p->name[i] == '\0');
                sb = '\0';
                p->write_byte(sb);
                csum += sb;
                crc = get_crc(sb, crc);
                ++i;

                // Send file length in decimal as ASCII string:
                //
                { // (limit scopes)
                    char dec[10];
                    uint32_t j = 0; // Position in array dec.

                    calc_dword_to_dec(p->file_len, dec);

                    while(dec[j] == '0')
                    {
                        ++j;

                        //assert(j < sizeof dec);
                    }

                    while(j < sizeof dec)
                    {
                        sb = dec[j];
                        p->write_byte(sb);
                        csum += sb;
                        crc = get_crc(sb, crc);
                        ++i;

                        ++j;
                    }
                }

                // Terminate file length and fill unused part of block:
                //
                while(i < block_size)
                {
                    sb = '\0';
                    p->write_byte(sb);
                    csum += sb;
                    crc = get_crc(sb, crc);

                    ++i;
                }

                state = ymodem_send_state_checksum_meta;
                break;
            }

            case ymodem_send_state_checksum_meta: // Send meta data CRC/checks.:
            {
                //assert(block_nr == 0);

                int rb;

                if(use_crc)
                {
                    sb = (uint8_t)(crc >> 8);
                    p->write_byte(sb);
                    sb = (uint8_t)(crc & 0x00FF);
                    p->write_byte(sb);
                }
                else
                {
                    sb = csum;
                    p->write_byte(sb);
                }

                rb = p->read_byte();

// #ifndef NDEBUG
//                 console_write("ymodem_send : Read byte ");
//                 console_write_byte(rb);
//                 console_writeline(" (4).");
// #endif //NDEBUG

                if(rb != ACK) // Receival acknowledged?
                {
                    state = ymodem_send_state_error;
                    err = ymodem_send_err_checksum_meta_ack;

// #ifndef NDEBUG
//                     console_write("ymodem_send : Error: Retrieved byte ");
//                     console_write_byte((uint8_t)rb);
//                     console_writeline("!");
// #endif //NDEBUG
                    break;
                }

                if(send_null_file) // Done:
                {
                    err = ymodem_send_err_none;
                    return err;
                }

                rb = p->read_byte();

// #ifndef NDEBUG
//                 console_write("ymodem_send : Read byte ");
//                 console_write_byte(rb);
//                 console_writeline(" (5).");
// #endif //NDEBUG

                // Not sure, if YMODEM supports CRC on/off toggling during
                // transfer, but should do no harm..
                //
                if(rb == NAK)
                {
                    use_crc = false;
                }
                else
                {
                    if(rb == CRC)
                    {
                        use_crc = true;
                    }
                    else
                    {
                        state = ymodem_send_state_error;
                        err = ymodem_send_err_checksum_meta_nak;
                        break;
                    }
                }

                // Send next block/sector (first content block) of this file:

                block_nr = 1;
                state = ymodem_send_state_start;
                break;
            }

            case ymodem_send_state_content:
            {
                //assert(block_nr > 0);
                //assert(
                //    (block_nr == 1 && offset == 0)
                //    || (block_nr > 1 && offset > 0);
                //assert(offset % block_size == 0);

                uint32_t i = 0;

                do
                {
                    if(offset < p->file_len)
                    {
                        sb = p->buf[offset]; // Content
                    }
                    else
                    {
                        send_null_file = true;
                        //
                        // Single file was completely transmitted.

                        sb = SUB; // Padding
                    }
                    ++offset;

                    p->write_byte(sb);
                    csum += sb;
                    crc = get_crc(sb, crc);

                    ++i;
                }while(i < block_size);

                state = ymodem_send_state_checksum_content;
                break;
            }

            case ymodem_send_state_checksum_content:
            {
                //assert(block_nr > 0);
                //assert(!send_null_file);

                uint8_t rb;

                if(use_crc)
                {
                    sb = (uint8_t)(crc >> 8);
                    p->write_byte(sb);
                    sb = (uint8_t)(crc & 0x00FF);
                    p->write_byte(sb);
                }
                else
                {
                    sb = csum;
                    p->write_byte(sb);
                }

                rb = p->read_byte();

// #ifndef NDEBUG
//                 console_write("ymodem_send : Read byte ");
//                 console_write_byte(rb);
//                 console_writeline(" (6).");
// #endif //NDEBUG

                if(rb != ACK) // Receival acknowledged?
                {
                    state = ymodem_send_state_error;
                    err = ymodem_send_err_checksum_content_ack;
                    break;
                }

                ++block_nr;

                if(send_null_file)
                {
                    state = ymodem_send_state_end;
                }
                else
                {
                    state = ymodem_send_state_start;
                }
                break;
            }

            default: // Should not happen, but would fall through..
                //assert(false);
            case ymodem_send_state_error:
            {
                while(p->is_ready_to_read())
                {
// #ifndef NDEBUG
//                     uint8_t rb = p->read_byte();
//
//                     console_write("ymodem_send : Read byte ");
//                     console_write_byte(rb);
//                     console_writeline(" (7).");
// #elseif //NDEBUG
                    p->read_byte();
// #endif //NDEBUG
                }

                return err;
            }
        }
    }

    //assert(false);
}

enum ymodem_receive_err ymodem_receive(struct ymodem_receive_params * const p)
{
    enum ymodem_receive_err err = ymodem_receive_err_unknown;
    uint8_t rb, csum, block_nr = 0;
    uint16_t block_size, data_byte_nr = 0;
    enum ymodem_receive_state state = ymodem_receive_state_start_or_end;
    uint32_t rx, package_offset = 0;
    bool null_file_received = false;
    uint8_t* meta_buf = 0;

    p->timer_start_one_mhz();

    // (not going to deal with timeouts other than here)
    //
    p->write_byte(NAK);
    rx = p->timer_get_tick();
    do
    {
        if(p->timer_get_tick() - rx >= NAKSOH_TIMEOUT)
        {
            p->write_byte(NAK);
            rx += NAKSOH_TIMEOUT;
        }

        if(p->is_stop_requested != 0 && p->is_stop_requested())
        {
            err = ymodem_receive_err_stop_requested;
            return err; // (a flush should not be necessary)
        }
    }while(!p->is_ready_to_read());
    //
    // Requests 8-bit checksum
    // (send 'C' for 16-bit checksum - not supported, here).

    while(true)
    {
        rb = p->read_byte();

        switch(state)
        {
            // Waiting for SOH, STX or EOT:
            //
            case ymodem_receive_state_start_or_end:
            {
                // Start:
                //
                if(rb == SOH)
                {
                    block_size = 128;
                    csum = rb;
                    state = ymodem_receive_state_block_nr;
                    break;
                }
                if(rb == STX)
                {
                    block_size = 1024;
                    csum = rb;
                    state = ymodem_receive_state_block_nr;
                    break;
                }

                // End (of transmission of current file, there may be more):
                //
                if(rb == EOT)
                {
                    block_nr = 0;
                    data_byte_nr = 0;
                    package_offset = 0;

                    p->write_byte(ACK);

                    p->write_byte(NAK); // <=> "Next file, please."
                    break;
                }

                state = ymodem_receive_state_error;
                err = ymodem_receive_err_start_or_end;
                break;
            }

            case ymodem_receive_state_block_nr:
            {
                if(rb != block_nr)
                {
                    state = ymodem_receive_state_error;
                    err = ymodem_receive_err_block_nr;
                    break;
                }
                csum += rb;
                state = ymodem_receive_state_inverted_block_nr;
                break;
            }

            case ymodem_receive_state_inverted_block_nr:
            {
                if(rb != 0xFF - block_nr)
                {
                    state = ymodem_receive_state_error;
                    err = ymodem_receive_err_inverted_block_nr;
                    break;
                }
                csum += rb;
                if(block_nr == 0)
                {
                    state = ymodem_receive_state_meta;
                }
                else
                {
                    state = ymodem_receive_state_content;
                }
                break;
            }

            case ymodem_receive_state_meta:
            {
                csum += rb;

                if(data_byte_nr == 0)
                {
                    if(rb == '\0')
                    {
                        null_file_received = true;
                    }
                    else
                    {
                        p->file_len = 0;
                        p->name[0] = '\0';

                        meta_buf = alloc_alloc(block_size * sizeof *meta_buf);
                    }
                }

                if(!null_file_received)
                {
                    meta_buf[data_byte_nr] = rb;
                }

                ++data_byte_nr;
                if(data_byte_nr == block_size)
                {
                    if(!null_file_received)
                    {
                        uint32_t i = 0, j = 0;
                        char dec[10];

                        // Get first characters of file name:
                        //
                        for(;i < sizeof p->name;++i)
                        {
                            p->name[i] = (char)meta_buf[i];
                            if(p->name[i] == '\0')
                            {
                                break;
                            }
                        }
                        p->name[sizeof p->name - 1] = '\0';
                        while(meta_buf[i] != '\0' && i < block_size)
                        {
                            ++i;
                        }

                        ++i;

                        if(i >= block_size)
                        {
                            state = ymodem_receive_state_error;
                            err = ymodem_receive_err_meta_data;
                            break;
                        }

                        // Get file's byte count:
                        //
                        while(j < sizeof dec - 1) // Is this correct?
                        {
                            dec[j] = meta_buf[i];

                            ++j;
                            ++i;
                            if(meta_buf[i] == ' ')
                            {
                                break;
                            }
                            if(i == block_size)
                            {
                                break;
                            }
                        }
                        dec[j] = '\0';
                        p->file_len = calc_str_to_dword(dec);

                        if(p->file_len > p->buf_len)
                        {
                            state = ymodem_receive_state_error;
                            err = ymodem_receive_err_meta_len;
                            break;
                        }
                        alloc_free(meta_buf);
                        meta_buf = 0;
                    }

                    state = ymodem_receive_state_checksum_meta;
                    data_byte_nr = 0;
                }
                break;
            }

            case ymodem_receive_state_checksum_meta:
            {
                if(rb != csum)
                {
                    state = ymodem_receive_state_error; // Error!
                    err = ymodem_receive_err_checksum_meta;
                    break;
                }

                block_nr = 1;
                state = ymodem_receive_state_start_or_end;
                p->write_byte(ACK); // Acknowledge receival of block 0.

                p->write_byte(NAK);
                //
                // Requests 8-bit checksum
                // (send 'C' for 16-bit checksum - not supported, here).

                if(null_file_received)
                {
                    rx = p->timer_get_tick();
                    while(p->timer_get_tick() - rx < WAIT_AFTER_EOT_TIMEOUT)
                    {
                        // (do nothing)
                    }

                    p->write_byte(CR);
                    p->write_byte(LF);
                    p->write_byte(LF);

                    return ymodem_receive_err_none;
                }
                break;
            }

            case ymodem_receive_state_content:
            {
                uint32_t const full_offset =
                                    package_offset + (uint32_t)data_byte_nr;

                csum += rb;

                if(full_offset < p->file_len)
                {
                    p->buf[full_offset] = rb;
                }

                ++data_byte_nr;
                if(data_byte_nr == block_size)
                {
                    state = ymodem_receive_state_checksum_content;
                    data_byte_nr = 0;
                }
                break;
            }

            case ymodem_receive_state_checksum_content:
            {
                if(rb != csum)
                {
                    state = ymodem_receive_state_error;
                    err = ymodem_receive_err_checksum_content;
                    break;
                }

                ++block_nr;
                package_offset += (uint32_t)block_size;
                state = ymodem_receive_state_start_or_end;
                p->write_byte(ACK);
                break;
            }

            default: // Should not happen, but would fall through..
            case ymodem_receive_state_error:
            {
                alloc_free(meta_buf);
                meta_buf = 0;

                p->write_byte(CAN);
                p->write_byte(CAN);
                p->write_byte(CAN);
                p->write_byte(CAN);
                p->write_byte(CAN);

                rx = p->timer_get_tick();
                while(p->timer_get_tick() - rx < WAIT_AFTER_CAN_TIMEOUT)
                {
                    // (do nothing)
                }

                p->write_byte(CR);
                p->write_byte(LF);
                p->write_byte(LF);

                //p->flush(); // Necessary?

                // Try to flush some out so we hit this timeout fewer times.

                while(p->is_ready_to_read())
                {
                    p->read_byte();
                }

                return err;
            }
        }
    }
}
