
// Marcel Timm, RhinoDevel, 2018oct25

#include <stdint.h>

#include "../calc/calc.h"
#include "console.h"

static uint8_t (*s_read_byte)() = 0;
static void (*s_write_byte)(uint8_t const) = 0;

static void write_newline()
{
    s_write_byte((uint8_t)'\r');
    s_write_byte((uint8_t)'\n');
}

void console_init(struct console_params const * const p)
{
    s_read_byte = p->read_byte;
    s_write_byte = p->write_byte;
}

void console_write_key(char const key)
{
    s_write_byte((uint8_t)key);
}

void console_write_hex(uint8_t const byte)
{
    char h = 0, l = 0;

    calc_byte_to_hex(byte, &h, &l);

    console_write_key(h);
    console_write_key(l);
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
