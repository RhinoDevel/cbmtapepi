
// Marcel Timm, RhinoDevel, 2019aug07

#include "video.h"
#include "../ascii/ascii.h"
#include "../mem/mem.h"
#include "../mailbox/mailbox.h"
//#include "../assert.h"

#include <stdint.h>

// Framebuffer parameters:
//
static uint32_t const s_physical_width = 320;
static uint32_t const s_physical_height = 200;
static uint32_t const s_fb_width = s_physical_width;
static uint32_t const s_fb_height = s_physical_height;
static uint32_t const s_bit_depth = 32;

// TODO: May not work reliably on anything newer than Raspberry Pi 1!
//
// TODO: Memory barriers or data cache invalidation/flushing may be required!
//
void video_init()
{
    static uint32_t const channel_nr = 1; // Of mailbox 0.

    volatile uint32_t msg_buf[10] __attribute__((aligned (16)));
    //
    // "__attribute__", etc. seems to be GCC-specific..

    uint32_t rb, rx, ry;

    // DEPRECATED mailbox?
    //
    // See: https://github.com/raspberrypi/firmware/wiki/Mailbox-framebuffer-interface

    msg_buf[0] = s_physical_width; // Physical width.
    msg_buf[1] = s_physical_height; // Physical height.
    msg_buf[2] = s_fb_width; // Virtual width.
    msg_buf[3] = s_fb_height; // Virtual height.
    msg_buf[4] = 0; // Pitch.
    msg_buf[5] = s_bit_depth; // Bit depth.
    msg_buf[6] = 0; // X offset of virtual framebuffer.
    msg_buf[7] = 0; // Y offset of virtual framebuffer.
    msg_buf[8] = 0; // Framebuffer address.
    msg_buf[9] = 0; // Framebuffer size.

    mailbox_write(
        channel_nr,
        (0x40000000 + (uint32_t)msg_buf) // TODO: Replace hard-coded conversion to physical memory address.
            >> 4);
    mailbox_read(channel_nr);

    //assert(msg_buf[4] == 0); // No support for pitch implemented..

    rb = msg_buf[8];
    for(ry=0;ry<s_fb_height;ry++)
    {
        for(rx=0;rx<s_fb_width;rx++)
        {
            mem_write(rb, 0xFF352879);
            rb += 4;
        }
    }

    for(int i = 0;i < 25;++i)
    {
        for(ry = 0;ry < 8; ++ry)
        {
            rb = msg_buf[8] + i * 4 * 8 * s_fb_width + 4 * ry * s_fb_width;

            for(rx = 0;rx < 8;++rx)
            {
                if(((ascii[0x41 + i][ry] >> rx) & 1) == 1)
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
