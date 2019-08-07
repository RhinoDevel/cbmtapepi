
// Marcel Timm, RhinoDevel, 2019aug07

#include "video.h"
#include "../mem/mem.h"

#include <stdint.h>
#include <stdbool.h>

static uint32_t const s_mailbox_addr = 0x2000B880;

static uint32_t mailbox_write(
    uint32_t const fbinfo_addr, uint32_t const channel)
{
    while(true)
    {
        if((mem_read(s_mailbox_addr + 0x18) & 0x80000000)==0)
        {
            break;
        }
    }
    mem_write(s_mailbox_addr + 0x20, fbinfo_addr + channel);

    return 0;
}

static uint32_t mailbox_read(uint32_t const channel)
{
    uint32_t ret_val;

    while(true)
    {
        while(true)
        {
            ret_val = mem_read(s_mailbox_addr + 0x18);
            if((ret_val & 0x40000000) == 0)
            {
                break;
            }
        }

        ret_val = mem_read(s_mailbox_addr + 0);
        if((ret_val & 0xF) == channel)
        {
            return ret_val;
        }
    }

    //assert(false);
}

void video_init()
{
    uint32_t ra, rb, rx, ry;

    mem_write(0x40040000, 640); /* #0 Physical Width */
    mem_write(0x40040004, 480); /* #4 Physical Height */
    mem_write(0x40040008, 640); /* #8 Virtual Width */
    mem_write(0x4004000C, 480); /* #12 Virtual Height */
    mem_write(0x40040010, 0); /* #16 GPU - Pitch */
    mem_write(0x40040014, 32); /* #20 Bit Depth */
    mem_write(0x40040018, 0); /* #24 X */
    mem_write(0x4004001C, 0); /* #28 Y */
    mem_write(0x40040020, 0); /* #32 GPU - Pointer */
    mem_write(0x40040024, 0); /* #36 GPU - Size */

    mailbox_write(0x40040000, 1);
    mailbox_read(1);

    rb = 0x40040000;
    for(ra = 0;ra < 10;++ra)
    {
        mem_read(rb);
        rb += 4;
    }

    rb = mem_read(0x40040020);
    for(ra = 0;ra < 10000;++ra)
    {
        mem_write(rb,~((ra&0xFF)<<0));
        rb+=4;
    }
    for(ra=0;ra<10000;ra++)
    {
        mem_write(rb,~((ra&0xFF)<<8));
        rb+=4;
    }
    for(ra=0;ra<10000;ra++)
    {
        mem_write(rb,~((ra&0xFF)<<16));
        rb+=4;
    }
    for(ra=0;ra<10000;ra++)
    {
        mem_write(rb,~((ra&0xFF)<<24));
        rb+=4;
    }
    rb=mem_read(0x40040020);
    ra=0;
    for(ry=0;ry<480;ry++)
    {
        for(rx=0;rx<640;rx++)
        {
            uint32_t r = (255 * ry) / 480;
            uint32_t b = (255 * rx) / 640;
            uint32_t g = 0;

            mem_write(rb, (0xFF | (r << 16) | (g << 8) | b) );
            rb+=4;
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
