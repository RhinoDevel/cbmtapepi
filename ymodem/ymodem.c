
// Marcel Timm, RhinoDevel, 2018dec31

#include <stdint.h>
#include <stdbool.h>

#include "../armtimer/armtimer.h"
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

static uint8_t const LF = 0x0A; // Line feed.
static uint8_t const CR = 0x0D; // Carriage return.

// Hard-coded for timer being set to 1 million ticks per second (see below):
//
static uint32_t const NAKSOH_TIMEOUT = 4000000;
static uint32_t const WAIT_AFTER_EOT_TIMEOUT = 2000000;
static uint32_t const WAIT_AFTER_CAN_TIMEOUT = 2000000;

enum ymodem_send_err ymodem_send(struct ymodem_send_params * const p)
{
    uint16_t const block_size = 1024;
    enum ymodem_send_err err = ymodem_send_err_unknown;
    enum ymodem_send_state state = ymodem_send_state_start;
    uint8_t sb, csum, block_nr = 0;
    uint32_t offset = 0;
    bool send_null_file = false;

    // Waiting for NAK:
    //
    if(p->read_byte() != NAK) // (p->read_byte() waits for being ready to read)
    {
        state = ymodem_send_state_error;
        err = ymodem_send_err_nak;
    }
    while(true)
    {
        switch(state)
        {
            case ymodem_send_state_start:
            {
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
                csum = sb;
                state = ymodem_send_state_block_nr;
                break;
            }
            case ymodem_send_state_end:
            {
                //assert(send_null_file);

                sb = EOT;
                p->write_byte(sb);

                // Waiting for NAK:
                //
                if(p->read_byte() != ACK)
                {
                    state = ymodem_send_state_error;
                    err = ymodem_send_err_end;
                    break;
                }

                // Done:

                err = ymodem_send_err_none;
                return err;
            }

            case ymodem_send_state_block_nr:
            {
                sb = block_nr;
                p->write_byte(sb);
                csum += sb;
                state = ymodem_send_state_inverted_block_nr;
                break;
            }

            case ymodem_send_state_inverted_block_nr:
            {
                sb = 0xFF - block_nr;
                p->write_byte(sb);
                csum += sb;
                if(block_nr == 0)
                {
                    state = ymodem_send_state_meta;
                }
                else
                {
                    state = ymodem_send_state_content;
                }
                break;
            }

            case ymodem_send_state_meta:
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

                        ++i;
                    }
                    state = ymodem_receive_state_checksum_meta;
                    break;
                }

                // Send meta data of file to transfer:

                // Send terminated file name:
                //
                while(i < sizeof p->name - 1)
                {
                    sb = p->name[i];
                    p->write_byte(sb);
                    csum += sb;

                    ++i;
                }
                //assert(p->name[i] == '\0');
                sb = '\0';
                p->write_byte(sb);
                csum += sb;
                ++i;

                // Send file length in decimal as ASCII string:
                //
                { // (limit scopes)
                    char dec[10];
                    uint32_t j = 0; // Position in array dec.

                    calc_dword_to_dec(p->file_len, dec);

                    while(dec[j] == 0)
                    {
                        ++j;

                        //assert(j < sizeof dec);
                    }

                    while(j < sizeof dec)
                    {
                        sb = dec[j];
                        p->write_byte(sb);
                        csum += sb;

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

                    ++i;
                }

                state = ymodem_send_state_checksum_meta;
                break;
            }

            case ymodem_send_state_checksum_meta:
            {
                //assert(block_nr == 0);

                sb = csum;
                p->write_byte(sb);

                if(p->read_byte() != ACK) // Receival acknowledged?
                {
                    state = ymodem_send_state_error;
                    err = ymodem_send_err_checksum_meta_ack;
                    break;
                }
                if(p->read_byte() != NAK) // 8-bit checksum requested?
                {
                    // ('C' will be send for 16-bit checksum request,
                    // not supported, here)

                    state = ymodem_send_state_error;
                    err = ymodem_send_err_checksum_meta_nak;
                    break;
                }

                block_nr = 1;
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
                        sb = 0; // Padding
                    }
                    ++offset;

                    p->write_byte(sb);
                    csum += sb;

                    ++i;
                }while(i < block_size);

                state = ymodem_send_state_checksum_content;
                break;
            }

            case ymodem_send_state_checksum_content:
            {
                //assert(block_nr > 0);
                //assert(!send_null_file);

                sb = csum;
                p->write_byte(sb);

                if(p->read_byte() != ACK) // Receival acknowledged?
                {
                    state = ymodem_send_state_error;
                    err = ymodem_send_err_checksum_content_ack;
                    break;
                }

                ++block_nr;
                state = ymodem_send_state_start;
                break;
            }

            default: // Should not happen, but would fall through..
                //assert(false);
            case ymodem_send_state_error:
            {
                //p->flush(); // Necessary?

                while(p->is_ready_to_read())
                {
                    p->read_byte();
                }

                return err;
            }
        }
    }

    //assert(false);
}

enum ymodem_receive_err ymodem_receive(struct ymodem_receive_params * const p)
{
    // - Data block nr. 0 holds meta data (file name, size).
    //
    // - (Non-meta) data block numbers start with 1.
    //
    // - Each packet contains:
    //
    //     1. SOH (for data block size of 128)
    //        or STX (for data block size of 1024 byte).
    //     2. Data block number (1 byte).
    //     3. Inverted data block number (255 - data block number) (1 byte).
    //     4. Data block (block_size bytes).
    //     5. Checksum for whole packet (1 byte).
    //
    // - Send NAK after first packet's checksum (packet with data block nr. 0).
    //
    // - The last packet is followed by an EOT.
    //
    // - Send an ACK after receiving EOT, too.

    enum ymodem_receive_err err = ymodem_receive_err_unknown;
    uint8_t rb, csum, block_nr = 0;
    uint16_t block_size, data_byte_nr = 0;
    enum ymodem_receive_state state = ymodem_receive_state_start_or_end;
    uint32_t rx, package_offset = 0;
    bool null_file_received = false;
    uint8_t* meta_buf = 0;

    armtimer_start_one_mhz();

    // (not going to deal with timeouts other than here)
    //
    p->write_byte(NAK);
    rx = armtimer_get_tick();
    do
    {
        if(armtimer_get_tick() - rx >= NAKSOH_TIMEOUT)
        {
            p->write_byte(NAK);
            rx += NAKSOH_TIMEOUT;
        }
    }while(!p->is_ready_to_read());

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
                    rx = armtimer_get_tick();
                    while(armtimer_get_tick() - rx < WAIT_AFTER_EOT_TIMEOUT)
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

                rx = armtimer_get_tick();
                while(armtimer_get_tick() - rx < WAIT_AFTER_CAN_TIMEOUT)
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
