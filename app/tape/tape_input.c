
// Marcel Timm, RhinoDevel, 2019sep14

#include "tape_input.h"
#include "../../lib/console/console.h"
#include "../../lib/alloc/alloc.h"

void tape_input_console_write(struct tape_input const * const d)
{
    uint16_t i;

    console_write("name = \"");
    for(i = 0;i < MT_TAPE_INPUT_NAME_LEN;++i)
    {
        console_write_key((char)d->name[i]);
    }
    console_writeline("\"");

    console_write("type = ");
    console_write_dword_dec((uint32_t)d->type);
    console_writeline("");

    console_write("addr = ");
    console_write_word_dec(d->addr);
    console_writeline("");

    console_write("bytes = ");
    for(i = 0;i < d->len;++i)
    {
        console_write_byte(d->bytes[i]);
    }
    console_writeline("");

    console_write("len = ");
    console_write_word_dec(d->len);
    console_writeline("");

    console_write("add_bytes = ");
    for(i = 0;i < MT_TAPE_INPUT_ADD_BYTES_LEN;++i)
    {
        console_write_byte(d->add_bytes[i]);
    }
    console_writeline("");
}

char* tape_input_create_str_from_name(struct tape_input const * const d)
{
    char * const ret_val = alloc_alloc(
        (MT_TAPE_INPUT_NAME_LEN + 1) * sizeof *ret_val);

    for(int i = 0;i < MT_TAPE_INPUT_NAME_LEN;++i)
    {
        ret_val[i] = (char)d->name[i];// TODO: PETSCII to ASCII conversion?
    }
    ret_val[MT_TAPE_INPUT_NAME_LEN] = '\0';

    return ret_val;
}
