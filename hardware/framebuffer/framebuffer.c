
// Marcel Timm, RhinoDevel, 2019aug21

#include "framebuffer.h"
#include "../mailbox/mailbox.h"
//#include "../../lib/assert.h"

#include <stdint.h>

static uint32_t * s_fb = 0; // Framebuffer address. Filled by init().

// TODO: May not work reliably on anything newer than Raspberry Pi 1!
//
// TODO: Memory barriers or data cache invalidation/flushing may be required!
//
static void init()
{
    static uint32_t const channel_nr = 1; // Of mailbox 0.

    volatile uint32_t msg_buf[10] __attribute__((aligned (16)));
    //
    // "__attribute__", etc. seems to be GCC-specific..

    // DEPRECATED mailbox?
    //
    // See: https://github.com/raspberrypi/firmware/wiki/Mailbox-framebuffer-interface

    msg_buf[0] = s_framebuffer_physical_width; // Physical width.
    msg_buf[1] = s_framebuffer_physical_height; // Physical height.
    msg_buf[2] = s_framebuffer_width; // Virtual width.
    msg_buf[3] = s_framebuffer_height; // Virtual height.
    msg_buf[4] = 0; // Pitch.
    msg_buf[5] = s_framebuffer_bit_depth; // Bit depth.
    msg_buf[6] = 0; // X offset of virtual framebuffer.
    msg_buf[7] = 0; // Y offset of virtual framebuffer.
    msg_buf[8] = 0; // Framebuffer address.
    msg_buf[9] = 0; // Framebuffer size.

    mailbox_write(
        channel_nr,
        (0x40000000 + (uint32_t)msg_buf) // TODO: Replace hard-coded conversion to physical memory address. Assuming L2 cache to be enabled, otherwise 0xC0000000 would be correct!
            >> 4);
    mailbox_read(channel_nr); // (return value ignored)

    //assert(msg_buf[4] == 0); // No support for pitch implemented..

    s_fb = (uint32_t*)msg_buf[8];
}

uint32_t* framebuffer_get()
{
    if(s_fb == 0)
    {
        init();
    }
    return s_fb;
}
