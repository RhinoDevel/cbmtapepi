
// Marcel Timm, RhinoDevel, 2018oct25

#include <stdint.h>
#include <stdbool.h>

#include "../calc/calc.h"
#include "console.h"

static uint8_t (*s_read_byte)() = 0;
static void (*s_write_byte)(uint8_t const) = 0;
static bool s_write_newline_with_cr = true;

static void write_newline()
{
    if(s_write_newline_with_cr)
    {
        s_write_byte((uint8_t)'\r');
    }
    s_write_byte((uint8_t)'\n');
}

void console_init(struct console_params const * const p)
{
    s_read_byte = p->read_byte;
    s_write_byte = p->write_byte;
    s_write_newline_with_cr = p->write_newline_with_cr;
}

void console_write_key(char const key)
{
    s_write_byte((uint8_t)key);
}

void console_write_byte(uint8_t const byte)
{
    char h = 0, l = 0;

    calc_byte_to_hex(byte, &h, &l);

    console_write_key(h);
    console_write_key(l);
}

void console_write_byte_dec(uint8_t const byte)
{
    char const zero = calc_get_dec(0);
    char three[3];
    bool zeroes = true;

    calc_byte_to_dec(byte, three);

    for(int i = 0;i < 3;++i)
    {
        if(zeroes && three[i] == zero)
        {
            if(i == 3 - 1)
            {
                console_write_key(three[i]/*zero*/);
            }
            continue;
        }
        zeroes = false;
        console_write_key(three[i]);
    }
}

void console_write_word_dec(uint16_t const word)
{
    char const zero = calc_get_dec(0);
    char five[5];
    bool zeroes = true;

    calc_word_to_dec(word, five);

    for(int i = 0;i < 5;++i)
    {
        if(zeroes && five[i] == zero)
        {
            if(i == 5 - 1)
            {
                console_write_key(five[i]/*zero*/);
            }
            continue;
        }
        zeroes = false;
        console_write_key(five[i]);
    }
}

void console_write_dword_dec(uint32_t const dword)
{
    char const zero = calc_get_dec(0);
    char ten[10];
    bool zeroes = true;

    calc_dword_to_dec(dword, ten);

    for(int i = 0;i < 10;++i)
    {
        if(zeroes && ten[i] == zero)
        {
            if(i == 10 - 1)
            {
                console_write_key(ten[i]/*zero*/);
            }
            continue;
        }
        zeroes = false;
        console_write_key(ten[i]);
    }
}

void console_write_word(uint16_t const word)
{
    char four[4];

    calc_word_to_hex(word, four);

    for(int i = 0;i < 4;++i)
    {
        console_write_key(four[i]);
    }
}

void console_write_dword(uint32_t const dword)
{
    char eight[8];

    calc_dword_to_hex(dword, eight);

    for(int i = 0;i < 8;++i)
    {
        console_write_key(eight[i]);
    }
}

void console_write(char const * const buf)
{
    int i = 0;

    while(buf[i] != '\0')
    {
        console_write_key(buf[i]);

        ++i;
    }
}

void console_writeline(char const * const buf)
{
    console_write(buf);
    write_newline();
}

char console_read_char()
{
    return (char)s_read_byte();
}

void console_read(char * const buf, int const count)
{
    int const lastIndex = count - 1;
    int i = 0;

    if(count < 1)
    {
        return;
    }
    while(i < lastIndex)
    {
        buf[i] = console_read_char();

        ++i;

        if(buf[i - 1] == '\r')
        {
            write_newline();
            break; // Done reading.
        }
        console_write_key(buf[i - 1]);
    }
    buf[i] = '\0';
}
