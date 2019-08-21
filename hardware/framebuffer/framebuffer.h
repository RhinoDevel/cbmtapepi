
// Marcel Timm, RhinoDevel, 2019aug21

#ifndef MT_FRAMEBUFFER
#define MT_FRAMEBUFFER

#include <stdint.h>

static uint32_t const s_framebuffer_physical_width = 640;
static uint32_t const s_framebuffer_physical_height = 480;
static uint32_t const s_framebuffer_width = s_framebuffer_physical_width;
static uint32_t const s_framebuffer_height = s_framebuffer_physical_height;
static uint32_t const s_framebuffer_bit_depth = 32;

uint32_t* framebuffer_get();

#endif //MT_FRAMEBUFFER
