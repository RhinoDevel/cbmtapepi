
// Marcel Timm, RhinoDevel, 2018dec31

#include <stdint.h>
#include <stdbool.h>

#include "../armtimer/armtimer.h"
#include "../console/console.h"
#include "ymodem_receive_params.h"
#include "ymodem_receive_state.h"
#include "ymodem.h"

#include "../baregpio/baregpio.h"

// Control characters
// (see: https://en.wikipedia.org/wiki/C0_and_C1_control_codes):
//
static uint8_t const SOH = 0x01; // Start of heading for 128 byte data block.
static uint8_t const STX = 0x02; // Start of heading for 1024 byte data block.
static uint8_t const ACK = 0x06; // Acknowledge
static uint8_t const NAK = 0x15; // Negative acknowledge.
static uint8_t const EOT = 0x04; // End of transmission.
static uint8_t const CAN = 0x18; // Cancel

static uint8_t const SUB = 0x1A; // Substitute

// static uint8_t const LF = 0x0A; // Line feed.
// static uint8_t const CR = 0x0D; // Carriage return.

// Hard-coded for timer being set to 1 million ticks per second (see below):
//
static uint32_t const NAKSOH_TIMEOUT = 4000000;
//static uint32_t const WAIT_AFTER_EOT_TIMEOUT = 2000000;
static uint32_t const WAIT_AFTER_CAN_TIMEOUT = 2000000;

bool ymodem_receive(
    struct ymodem_receive_params const * const p, uint32_t * const count)
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

    uint8_t rb, csum, block_nr = 0;
    uint16_t block_size, data_byte_nr = 0;
    enum ymodem_receive_state state = ymodem_receive_state_start_or_end;
    uint32_t rx, package_offset = 0;
    bool null_file_received = false;

    baregpio_set_output(16, false); // Internal LED (green one).

    //console_writeline("ymodem_receive: Starting timer with 1 MHz..");

    armtimer_start_one_mhz();

    //console_writeline("ymodem_receive: Starting initial NAK writes..");

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

    //console_writeline("ymodem_receive: Starting receive loop..");

    while(true)
    {
        rb = p->read_byte();

        //console_write("ymodem_receive: State is ");
        //console_write_dword((uint32_t)state);
        //console_write(", read byte is ");
        //console_write_byte(rb);
        //console_writeline(".");

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
                    //console_writeline("ymodem_receive: Read SOH.");
                    block_size = 128;
                    csum = rb;
                    state = ymodem_receive_state_block_nr;
                    break;
                }
                if(rb == STX)
                {
                    //console_writeline("ymodem_receive: Read STX.");
                    block_size = 1024;
                    csum = rb;
                    state = ymodem_receive_state_block_nr;
                    break;
                }

                // End:
                //
                if(rb == EOT)
                {
                    //console_writeline("ymodem_receive: Read EOT.");

                    block_nr = 0;
                    data_byte_nr = 0;
                    package_offset = 0;

                    p->write_byte(ACK);
                    p->write_byte(NAK);

                    // Necessary?
                    //
                    // rx = armtimer_get_tick();
                    // while(armtimer_get_tick() - rx < WAIT_AFTER_EOT_TIMEOUT)
                    // {
                    //     // (do nothing)
                    // }

                    break;
                }

                //console_writeline("ymodem_receive: Error (start or end)!");
                //baregpio_write(16, true);

                state = ymodem_receive_state_error;
                break;
            }

            case ymodem_receive_state_block_nr:
            {
                if(rb != block_nr)
                {
                    //console_writeline("ymodem_receive: Error (block nr.)!");
                    //baregpio_write(16, true);

                    state = ymodem_receive_state_error; // Error!
                    break;
                }
                //console_writeline("ymodem_receive: Retrieved block nr.");
                csum += rb;
                state = ymodem_receive_state_inverted_block_nr;
                break;
            }

            case ymodem_receive_state_inverted_block_nr:
            {
                if(rb != 0xFF - block_nr)
                {
                    //console_writeline(
                    //    "ymodem_receive: Error (inv. block nr.)!");
                    //baregpio_write(16, true);

                    state = ymodem_receive_state_error;
                    break;
                }
                //console_writeline("ymodem_receive: Retrieved inv. block nr.");
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
                // TODO: Use meta data received (file name, size, etc.).

                csum += rb;

                // static int debugI = 0;
                // //
                // if(debugI == 1 && data_byte_nr == 0)
                // {
                //     baregpio_write(16, true);
                // }
                // debugI = 1;

                if(data_byte_nr == 0 && rb == '\0')
                {
                    null_file_received = true;
                }
                else
                {
                    *count = 0;
                }

                ++data_byte_nr;
                if(data_byte_nr == block_size)
                {
                    state = ymodem_receive_state_checksum_meta;
                    data_byte_nr = 0;
                }
                break;
            }

            case ymodem_receive_state_checksum_meta:
            {
                if(rb != csum)
                {
                    //console_writeline("ymodem_receive: Error (meta checksum)!");
                    //baregpio_write(16, true);

                    state = ymodem_receive_state_error; // Error!
                    break;
                }

                //console_writeline("ymodem_receive: Retrieved meta checksum.");

                block_nr = 1;
                state = ymodem_receive_state_start_or_end;
                p->write_byte(ACK);
                p->write_byte(NAK);

                if(null_file_received)
                {
                    // p->write_byte(CR);
                    // p->write_byte(LF);
                    // p->write_byte(LF);

                    // TODO: This is NOT safe:
                    //
                    {
                        uint32_t real_count = *count,
                            i = *count - 1;

                        while(p->buf[i] == SUB)
                        {
                            --real_count;
                            --i;
                        }
                        *count = real_count;
                    }
                    //console_writeline("ymodem_receive: Success. Returning..");
                    return true; // *** RETURN ***
                }
                break;
            }

            case ymodem_receive_state_content:
            {
                uint32_t const full_offset =
                                    package_offset + (uint32_t)data_byte_nr;

                //console_writeline("ymodem_receive: Retrieved content data.");

                if(full_offset >= p->len)
                {
                    //console_writeline(
                    //    "ymodem_receive: Error (content data; buffer is too small)!");
                    //baregpio_write(16, true);

                    state = ymodem_receive_state_error;
                    break;
                }
                csum += rb;
                p->buf[full_offset] = rb;
                *count = full_offset + 1;
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
                    //console_writeline("ymodem_receive: Error (content checksum)!");
                    //baregpio_write(16, true);

                    state = ymodem_receive_state_error; // Error!
                    break;
                }

                //console_writeline("ymodem_receive: Retrieved content checksum.");

                ++block_nr;
                package_offset += (uint32_t)block_size;
                if(package_offset >= p->len)
                {
                    //console_writeline(
                    //    "ymodem_receive: Error (content checksum; buffer is too small)!");
                    //baregpio_write(16, true);

                    state = ymodem_receive_state_error; // Error!
                    break;
                }
                state = ymodem_receive_state_start_or_end;
                p->write_byte(ACK);
                break;
            }

            default: // Should not happen, but would fall through..
            case ymodem_receive_state_error:
            {
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

                // p->write_byte(CR);
                // p->write_byte(LF);
                // p->write_byte(LF);

                //p->flush(); // Necessary?

                // Try to flush some out so we hit this timeout fewer times.

                while(p->is_ready_to_read())
                {
                    p->read_byte();
                }

                return false; // *** RETURN ***
            }
        }
    }
}
