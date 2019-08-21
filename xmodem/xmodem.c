
// Marcel Timm, RhinoDevel, 2018dec16

#include <stdint.h>
#include <stdbool.h>

//#include "../lib/console/console.h"
#include "xmodem_receive_params.h"
#include "xmodem.h"

// Control characters
// (see: https://en.wikipedia.org/wiki/C0_and_C1_control_codes):
//
static uint8_t const SOH = 0x01; // Start of heading.
static uint8_t const ACK = 0x06; // Acknowledge
static uint8_t const NAK = 0x15; // Negative acknowledge.
static uint8_t const EOT = 0x04; // End of transmission.
static uint8_t const CAN = 0x18; // Cancel

// static uint8_t const SUB = 0x1A; // Substitute

static uint8_t const LF = 0x0A; // Line feed.
static uint8_t const CR = 0x0D; // Carriage return.

// Hard-coded for timer being set to 1 million ticks per second (see below):
//
static uint32_t const NAKSOH_TIMEOUT = 4000000;
static uint32_t const WAIT_AFTER_EOT_TIMEOUT = 2000000;
static uint32_t const WAIT_AFTER_CAN_TIMEOUT = 2000000;

bool xmodem_receive(
    struct xmodem_receive_params const * const p, uint32_t * const count)
{
    // - Block numbers start with 1.
    //
    // - Each packet contains:
    //
    //     1. SOH (1 byte).
    //     2. Block number (1 byte).
    //     3. Inverted block number (255 - block number) (1 byte).
    //     4. Data (128 bytes).
    //     5. Checksum for whole packet (1 byte).
    //     6. EOT (1 byte).
    //
    // - Send an ACK after EOT, too.

    uint8_t rb, block = 1, state = 192, csum = 0;
    uint32_t rx, package_offset = 0;

    //console_writeline("xmodem_receive: Starting timer with 1 MHz..");

    p->timer_start_one_mhz();

    //console_writeline("xmodem_receive: Starting initial NAK writes..");

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
    }while(!p->is_ready_to_read());

    //console_writeline("xmodem_receive: Starting receive loop..");

    do // (initial state must be 192)
    {
        rb = p->read_byte();

        //console_write("xmodem_receive: State is ");
        //console_write_byte(state);
        //console_write(", read byte is ");
        //console_write_byte(rb);
        //console_writeline(".");

        switch(state)
        {
            case 192: // Wait for SOH or EOT.
            {
                switch(rb)
                {
                    case SOH:
                    {
                        //console_writeline("xmodem_receive: Read SOH.");
                        csum = rb;
                        ++state;
                        break;
                    }
                    case EOT: // Successfully retrieved data.
                    {
                        //console_writeline("xmodem_receive: Read EOT.");

                        p->write_byte(ACK);

                        rx = p->timer_get_tick();
                        while(p->timer_get_tick() - rx < WAIT_AFTER_EOT_TIMEOUT)
                        {
                            // (do nothing)
                        }

                        p->write_byte(CR);
                        p->write_byte(LF);
                        p->write_byte(LF);

                        // TODO: This is NOT safe:
                        //
                        // {
                        //     uint32_t real_count = *count,
                        //         i = *count - 1;
                        //
                        //     while(p->buf[i] == SUB)
                        //     {
                        //         --real_count;
                        //         --i;
                        //     }
                        //     *count = real_count;
                        // }

                        //console_writeline(
                        //    "xmodem_receive: Success. Returning..");
                        return true; // *** RETURN ***
                    }

                    default: // Error!
                    {
                        //console_writeline("xmodem_receive: Error (192)!");
                        state = 255;
                        break;
                    }
                }
                break;
            }

            case 193: // Uninverted block number.
            {
                if(rb == block)
                {
                    //console_writeline(
                    //    "xmodem_receive: Retrieved uninverted block nr.");
                    csum += rb;
                    ++state;
                    break;
                }
                //console_writeline("xmodem_receive: Error (193)!");
                state = 255; // Error!
                break;
            }

            case 194: // Inverted block number.
            {
                if(rb == 0xFF - block)
                {
                    //console_writeline(
                    //    "xmodem_receive: Retrieved inverted block nr.");
                    csum += rb;
                    state = 0;
                    break;
                }
                //console_writeline("xmodem_receive: Error (194)!");
                state = 255; // Error!
                break;
            }

            case 128: // Checksum.
            {
                if(rb == csum)
                {
                    //console_writeline("xmodem_receive: Retrieved checksum.");
                    p->write_byte(ACK);
                    ++block;
                    package_offset += 128;
                    if(package_offset >= p->len)
                    {
                        //console_writeline(
                        //    "xmodem_receive: Error (128; buffer is too small)!");
                        state = 255; // Error!
                        break;
                    }
                    state = 192;
                    break;
                }
                //console_writeline("xmodem_receive: Error (128)!");
                state = 255; // Error!
                break;
            }

            default:
            {
                uint32_t const full_offset = package_offset + (uint32_t)state;

                //console_writeline("xmodem_receive: Retrieved data (default).");

                if(full_offset >= p->len)
                {
                    //console_writeline(
                    //    "xmodem_receive: Error (default; buffer is too small)!");
                    state = 255; // Error!
                    break;
                }

                csum += rb;
                p->buf[full_offset] = rb;
                *count = full_offset + 1;
                ++state;
                break;
            }
        }
    }while(state != 255);

    // Something went wrong!

    //console_writeline("xmodem_receive: Error, broke out of receive loop!");

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

    //console_writeline("xmodem_receive: Failure! Returning..");
    return false;
}

// Original source code:
//
// https://raw.githubusercontent.com/dwelch67/raspberrypi/master/bootloader06/bootloader06.c

// Original source code copyright and permission notice:
//
// Copyright (c) 2012 David Welch dwelch@dwelch.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
