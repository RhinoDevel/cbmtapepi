
// Marcel Timm, RhinoDevel, 2019aug07

#include "video.h"
#include "../ascii/ascii.h"
#include "../mem/mem.h"
#include "../mailbox/mailbox.h"
//#include "../assert.h"

#include <stdint.h>

// Hard-coded for 8x8 character size font (see ascii.h file):
//
static uint32_t const s_char_width = 8;
static uint32_t const s_char_height = 8;

// Framebuffer parameters:
//
static uint32_t const s_physical_width = 640;
static uint32_t const s_physical_height = 480;
static uint32_t const s_fb_width = s_physical_width;
static uint32_t const s_fb_height = s_physical_height;
static uint32_t const s_bit_depth = 32;

static uint32_t const s_color_foreground = 0xFF6C5EB5;
static uint32_t const s_color_background = 0xFF352879;

static uint32_t const s_char_row_count = s_fb_height / s_char_height;
static uint32_t const s_char_col_count = s_fb_width / s_char_width;

static uint32_t s_char_cursor_row = 0; // In characters.
static uint32_t s_char_cursor_col = 0; // In characters.

static void forward_cursor()
{
    ++s_char_cursor_col;
    if(s_char_cursor_col == s_char_col_count)
    {
        s_char_cursor_col = 0;
        ++s_char_cursor_row;
        if(s_char_cursor_row == s_char_row_count)
        {
            s_char_cursor_row = 0; // No scrolling..
        }
    }
}

static void draw_fill(uint32_t * const fb, uint32_t const val)
{
    static uint32_t const pixel_count = s_fb_width * s_fb_height;

    for(uint32_t i = 0;i < pixel_count;++i)
    {
        fb[i] = val;
    }
}

static void draw_background(uint32_t * const fb)
{
    draw_fill(fb, s_color_background);
}

/**
 * - Also forwards cursor.
 */
static void draw_char_at_cursor(
    uint8_t const ascii_index, uint32_t * const fb)
{
    uint32_t const fb_row_start = s_char_height * s_char_cursor_row,
        fb_row_lim = fb_row_start + s_char_height,
        fb_col_start = s_char_width * s_char_cursor_col,
        fb_col_lim = fb_col_start + s_char_width;
    uint32_t char_row = 0;

    for(uint32_t fb_row = fb_row_start; fb_row < fb_row_lim; ++fb_row)
    {
        uint32_t * const fb_row_ptr = fb + fb_row * s_fb_width;
        uint8_t char_row_buf = ascii[ascii_index][char_row];

        for(uint32_t fb_col = fb_col_start;fb_col < fb_col_lim; ++fb_col)
        {
            uint32_t * const fb_col_ptr = fb_row_ptr + fb_col;

            if((char_row_buf & 1) == 1)
            {
                *fb_col_ptr = s_color_foreground;
            }
            else
            {
                *fb_col_ptr = s_color_background;
            }

            char_row_buf = char_row_buf >> 1;
        }

        ++char_row;
    }

    forward_cursor();
}

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

    uint32_t* fb;

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

    fb = (uint32_t*)msg_buf[8];

    draw_background(fb);

    for(uint32_t i = 0;i < s_char_col_count * s_char_row_count;i += 10)
    {
        for(uint8_t ascii_index = 'A';ascii_index <= 'J';++ascii_index)
        {
            draw_char_at_cursor(ascii_index, fb);
        }
    }
    for(uint32_t i = 0;i < s_char_col_count;i += 10)
    {
        for(uint8_t ascii_index = '0';ascii_index <= '9';++ascii_index)
        {
            draw_char_at_cursor(ascii_index, fb);
        }
    }
}
