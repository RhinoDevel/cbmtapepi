
// Marcel Timm, RhinoDevel, 2018oct26

#include <stdint.h>

#include "calc.h"
#include "../lib/str/str.h"

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

char calc_get_dec(uint8_t const n)
{
    if(n < 10)
    {
        return '0' + n;
    }
    return '?';
}

/**
 *  - Returns UINT8_MAX on error.
 */
uint8_t calc_get_digit(char const c)
{
    if(c < '0')
    {
        return UINT8_MAX;
    }
    if(c > '9')
    {
        return UINT8_MAX;
    }
    return (uint8_t)(c - '0');
}

void calc_byte_to_dec(uint8_t const byte, char * const out_three)
{
    uint8_t buf = byte;

    for(int i = 2;i >= 0;--i)
    {
        uint8_t val = 0;

        if(buf > 0)
        {
            val = buf % 10;
            buf = buf / 10;
        }
        out_three[i] = calc_get_dec(val);
    }
}

void calc_word_to_dec(uint16_t const word, char * const out_five)
{
    uint16_t buf = word;

    for(int i = 4;i >= 0;--i)
    {
        uint8_t val = 0;

        if(buf > 0)
        {
            val = (uint8_t)(buf % 10);
            buf = buf / 10;
        }
        out_five[i] = calc_get_dec(val);
    }
}

void calc_dword_to_dec(uint32_t const dword, char * const out_ten)
{
    uint32_t buf = dword;

    for(int i = 9;i >= 0;--i)
    {
        uint8_t val = 0;

        if(buf > 0)
        {
            val = (uint8_t)(buf % 10);
            buf = buf / 10;
        }
        out_ten[i] = calc_get_dec(val);
    }
}

uint32_t calc_get_pow_of_ten(uint32_t const val)
{
    uint32_t ret_val = 1;

    for(uint32_t i = 0;i < val;++i)
    {
        ret_val *= 10;
    }
    return ret_val;
}

uint32_t calc_str_to_dword(char const * const s)
{
    uint32_t const len = str_get_len(s);

    uint32_t ret_val = 0;

    for(uint32_t i = 0;i < len;++i)
    {
        uint8_t const digit = calc_get_digit(s[i]);

        if(digit > 9)
        {
            return UINT32_MAX; // Bad: Could be a valid return value, too!
        }

        ret_val += (uint32_t)digit * calc_get_pow_of_ten(len - i - 1);
    }

    return ret_val;
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
