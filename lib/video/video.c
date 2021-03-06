
// Marcel Timm, RhinoDevel, 2019aug07

#include "video.h"
#include "../../hardware/framebuffer/framebuffer.h"
#include "../../data/rhino.h"
#include "../../data/ascii.h"
#include "../mem/mem.h"
//#include "../../assert.h"

#include <stdint.h>

// Hard-coded for 8x8 character size font (see ascii.h file):
//
static uint32_t const s_char_width = 8;
static uint32_t const s_char_height = 8;

static uint32_t const s_color_foreground = 0xFF00FF00;//0xFF6C5EB5;
static uint32_t const s_color_background = 0xFF000000;//0xFF352879;

static uint32_t const s_char_row_count = s_framebuffer_height / s_char_height;
static uint32_t const s_char_col_count = s_framebuffer_width / s_char_width;

static uint32_t s_char_cursor_row = 0; // In characters.
static uint32_t s_char_cursor_col = 0; // In characters.

static uint32_t * s_fb = 0; // Framebuffer address.

static void draw_fill(
    uint32_t const val, uint32_t * const fb_ptr, uint32_t const pixel_count)
{
    for(uint32_t i = 0;i < pixel_count;++i)
    {
        fb_ptr[i] = val;
    }
}

static void draw_fill_screen(uint32_t const val)
{
    draw_fill(val, s_fb, s_framebuffer_width * s_framebuffer_height);
}

static void draw_background()
{
    draw_fill_screen(s_color_background);
}

static void scroll_up(uint32_t const fb_row_count)
{
    uint32_t const offset = fb_row_count * s_framebuffer_width;
    uint32_t const * const fb_lim =
        s_fb + s_framebuffer_height * s_framebuffer_width - offset;
    uint32_t* fb;

    for(fb = s_fb; fb < fb_lim; ++fb)
    {
        *fb = fb[offset];
    }
    draw_fill(s_color_background, fb, offset);
}

static void scroll_char_up(uint32_t const char_row_count)
{
    scroll_up(char_row_count * s_char_height);
}

static void forward_cursor()
{
    ++s_char_cursor_col;
    if(s_char_cursor_col == s_char_col_count)
    {
        s_char_cursor_col = 0;
        ++s_char_cursor_row;
        if(s_char_cursor_row == s_char_row_count)
        {
            scroll_char_up(1);
            --s_char_cursor_row;
        }
    }
}

static void newline_cursor()
{
    s_char_cursor_col = s_char_col_count - 1;
    forward_cursor();
}

/**
 * - Also forwards cursor.
 */
static void draw_visible_char_at_cursor(
    uint8_t const ascii_index, bool const swap_colors)
{
    //assert(ascii_index >= ' ' && ascii_index <= '~');

    uint32_t const fb_row_start = s_char_height * s_char_cursor_row,
        fb_row_lim = fb_row_start + s_char_height,
        fb_col_start = s_char_width * s_char_cursor_col,
        fb_col_lim = fb_col_start + s_char_width,
        fg_color = swap_colors ? s_color_background : s_color_foreground,
        bg_color = swap_colors ? s_color_foreground : s_color_background;
    uint32_t char_row = 0;

    for(uint32_t fb_row = fb_row_start; fb_row < fb_row_lim; ++fb_row)
    {
        uint32_t * const fb_row_ptr = s_fb + fb_row * s_framebuffer_width;
        uint8_t char_row_buf = s_ascii[ascii_index][char_row];

        for(uint32_t fb_col = fb_col_start;fb_col < fb_col_lim;++fb_col)
        {
            uint32_t * const fb_col_ptr = fb_row_ptr + fb_col;

            if((char_row_buf & 1) == 1)
            {
                *fb_col_ptr = fg_color;
            }
            else
            {
                *fb_col_ptr = bg_color;
            }

            char_row_buf = char_row_buf >> 1;
        }

        ++char_row;
    }

    forward_cursor();
}

/**
 * - Also alters cursor accordingly.
 */
static void draw_char_at_cursor(uint8_t const ascii_index)
{
    if(ascii_index >= 0x20 && ascii_index <= 0x7E)
    {
        draw_visible_char_at_cursor(ascii_index, false);
        return;
    }
    switch(ascii_index)
    {
        case '\n': // Line feed.
        {
            newline_cursor();
            break;
        }

        default: // Unsupported character.
        {
            draw_visible_char_at_cursor('?', true);
            break;
        }
    }
    //assert(false);
}

static void draw_rhino()
{
    static uint32_t const pos_x = s_framebuffer_width - s_rhino_width;
    static uint32_t const pos_y = 0/*s_framebuffer_height - s_rhino_height*/;
    static uint32_t const row_lim = pos_y + s_rhino_height;
    static uint32_t const col_lim = pos_x + s_rhino_width;

    uint32_t rhino_index = 0;

    for(uint32_t row = pos_y;row < row_lim;++row)
    {
        uint32_t * const fb_row = s_fb + row * s_framebuffer_width;

        for(uint32_t col = pos_x;col < col_lim;++col)
        {
            //assert(s_rhino_bytes_per_pixel == 3);

            uint32_t const r = (uint32_t)s_rhino[rhino_index],
                g = (uint32_t)s_rhino[rhino_index + 1],
                b = (uint32_t)s_rhino[rhino_index + 2];

            fb_row[col] = 0xFF000000 | r << 16 | g << 8 | b;

            rhino_index += s_rhino_bytes_per_pixel;
        }
    }
}

void video_write_byte(uint8_t const byte)
{
    draw_char_at_cursor(byte);
}

void video_init()
{
    s_fb = framebuffer_get();

    draw_background();
    draw_rhino();
}
