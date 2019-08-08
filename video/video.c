
// Marcel Timm, RhinoDevel, 2019aug07

#include "video.h"
#include "../peribase.h"
#include "../mem/mem.h"
#include "../ascii/ascii.h"

#include <stdint.h>
#include <stdbool.h>

static uint32_t const s_mailbox0_base = PERI_BASE + 0xB880;

static uint32_t const s_mailbox0_read = s_mailbox0_base;
//
// Upper 28 bits hold value, lower 4 bits hold the sending channel's number.

static uint32_t const s_mailbox0_status = s_mailbox0_base + 0x18;

static uint32_t const s_mailbox0_write = s_mailbox0_base + 0x20;
//
// Upper 28 bits hold value, lower 4 bits hold the receiving channel's number.

static uint32_t const s_status_empty = 0x40000000; // (bit 30 set, if empty)
static uint32_t const s_status_full = 0x80000000; // (bit 31 set, if full)

/**
 * - Expects 28 bit value.
 */
static void mailbox_write(uint32_t const channel, uint32_t const val)
{
    //assert(channel >= 0 && channel < 16);
    //assert((val <= 0x0FFFFFFF);

    while((mem_read(s_mailbox0_status) & s_status_full) != 0)
    {
        // Mailbox is full. Wait.
    }
    mem_write(s_mailbox0_write, (val << 16) | channel);
}

/**
 * - Returns higher 28 bits (without the channel nr.).
 */
static uint32_t mailbox_read(uint32_t const channel)
{
    //assert(channel >= 0 && channel < 16);

    while(true)
    {
        uint32_t val;

        while((mem_read(s_mailbox0_status) & s_status_empty) != 0)
        {
            // Mailbox is empty. Wait.
        }

        val = mem_read(s_mailbox0_read);
        if((val & 0xF) == channel) // Lower 4 bits.
        {
            return val >> 4; // Higher 28 bits.
        }
    }

    //assert(false);
}

void video_init()
{
    static uint32_t const channel_nr = 1; // Of mailbox 0.
    static uint32_t const msg_buf_addr = 0x40040000; // Must be 16-byte aligned.

    static uint32_t const physical_width = 320;
    static uint32_t const physical_height = 200;
    static uint32_t const fb_width = physical_width;
    static uint32_t const fb_height = physical_height;
    static uint32_t const bit_depth = 32;

    uint32_t rb, rx, ry;

    // DEPRECATED mailbox?
    //
    // See: https://github.com/raspberrypi/firmware/wiki/Mailbox-framebuffer-interface

    mem_write(msg_buf_addr, physical_width); /* #0 Physical width. */
    mem_write(msg_buf_addr + 4, physical_height); /* #4 Physical height. */
    mem_write(msg_buf_addr + 8, fb_width); /* #8 Virtual width. */
    mem_write(msg_buf_addr + 12, fb_height); /* #12 Virtual height. */
    mem_write(msg_buf_addr + 16, 0); /* #16 Pitch. */
    mem_write(msg_buf_addr + 20, bit_depth); /* #20 Bit depth. */
    mem_write(msg_buf_addr + 24, 0); /* #24 X offset of virtual framebuffer. */
    mem_write(msg_buf_addr + 28, 0); /* #28 Y offset of virtual framebuffer. */
    mem_write(msg_buf_addr + 32, 0); /* #32 Framebuffer address. */
    mem_write(msg_buf_addr + 36, 0); /* #36 Framebuffer size. */

    mailbox_write(channel_nr, msg_buf_addr >> 16);
    mailbox_read(channel_nr);

    rb = mem_read(0x40040020);
    for(ry=0;ry<fb_height;ry++)
    {
        for(rx=0;rx<fb_width;rx++)
        {
            mem_write(rb, 0xFF352879);
            rb += 4;
        }
    }

    for(int i = 0;i < 6;++i)
    {
        for(ry = 0;ry < 8; ++ry)
        {
            rb = mem_read(0x40040020) + i * 4 * 8 * fb_width + 4 * ry * fb_width;

            for(rx = 0;rx < 8;++rx)
            {
                if(ascii[i][ry * 8 + rx] == 1)
                {
                    mem_write(rb, 0xFF6C5EB5);
                }
                else
                {
                    mem_write(rb, 0xFF352879);
                }
                rb += 4;
            }
        }
    }
}

// Original source code:
//
// https://github.com/dwelch67/raspberrypi/blob/master/video01/video01.c

// Original source code copyright and permission notice:
//
// Copyright (c) 2012 David Welch dwelch@dwelch.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
