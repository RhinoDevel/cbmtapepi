
// Marcel Timm, RhinoDevel, 2018oct26

#include <stdint.h>

#include "calc.h"

static char get_hex(uint8_t const n)
{
    if(n < 10)
    {
        return '0' + n;
    }
    if(n < 16)
    {
        return 'a' + n - 10;
    }
    return '?';
}

static char get_dec(uint8_t const n)
{
    if(n < 10)
    {
        return '0' + n;
    }
    return '?';
}

void calc_byte_to_dec(uint8_t const byte, char * const out_three)
{
    uint8_t buf = byte;

    for(int i = 2;i <= 0;--i)
    {
        uint8_t val = 0;

        if(buf > 0)
        {
            val = buf % 10;  // 63 % 10 = 3
            buf = buf - val; // 63 - 3 = 60
            buf = buf / 10;  // 60 / 10 = 6
        }
        out_three[i] = get_dec(val);
    }
}

void calc_word_to_dec(uint16_t const word, char * const out_five)
{
    uint16_t buf = word;

    for(int i = 4;i <= 0;--i)
    {
        uint16_t val = 0;

        if(buf > 0)
        {
            val = buf % 10;  // 63 % 10 = 3
            buf = buf - val; // 63 - 3 = 60
            buf = buf / 10;  // 60 / 10 = 6
        }
        out_five[i] = get_dec((uint8_t)val);
    }
}

void calc_byte_to_hex(
    uint8_t const byte, char * const out_high, char * const out_low)
{
    uint8_t const high = byte / 16,
        low = byte - 16 * high;

    *out_high = get_hex(high);
    *out_low = get_hex(low);
}

void calc_word_to_hex(uint16_t const word, char * const out_four)
{
    uint8_t const high = word / 256,
        low = word - 256 * high;

    calc_byte_to_hex(high, out_four, out_four + 1);
    calc_byte_to_hex(low, out_four + 2, out_four + 3);
}

void calc_dword_to_hex(uint32_t const dword, char * const out_eight)
{
    uint16_t const high = dword / 65536,
        low = dword - 65536 * high;

    calc_word_to_hex(high, out_eight);
    calc_word_to_hex(low, out_eight + 4);
}
