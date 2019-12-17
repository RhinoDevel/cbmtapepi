
// Marcel Timm, RhinoDevel, 2019sep14

#include "tape_input.h"
#include "../config.h"
#include "../../lib/assert.h"
#include "../../lib/console/console.h"
#include "../../lib/alloc/alloc.h"
#include "../../lib/str/str.h"
#include "../../lib/petasc/petasc.h"

void tape_input_free(struct tape_input * const d)
{
    if(d == 0)
    {
        assert(false);
        return;
    }
    alloc_free(d->bytes);
    alloc_free(d);
}

void tape_input_console_write(
    struct tape_input const * const d, bool const write_bytes)
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
    if(write_bytes)
    {
        for(i = 0;i < d->len;++i)
        {
            console_write_byte(d->bytes[i]);
        }
        console_writeline("");
    }
    else
    {
        console_writeline("...");
    }

    console_write("len = ");
    console_write_word_dec(d->len);
    console_writeline("");

    console_write("add_bytes = ");
    if(write_bytes)
    {
        for(i = 0;i < MT_TAPE_INPUT_ADD_BYTES_LEN;++i)
        {
            console_write_byte(d->add_bytes[i]);
        }
        console_writeline("");
    }
    else
    {
        console_writeline("...");
    }
}

char* tape_input_create_str_from_name(struct tape_input const * const d)
{
    char * const ret_val = alloc_alloc(
        (MT_TAPE_INPUT_NAME_LEN + 1) * sizeof *ret_val);

    for(int i = 0;i < MT_TAPE_INPUT_NAME_LEN;++i)
    {
        ret_val[i] = petasc_get_ascii((char)d->name[i], MT_ASCII_REPLACER);
    }
    ret_val[MT_TAPE_INPUT_NAME_LEN] = '\0';

    // (this wastes some memory, but not really a problem)
    //
    int const trim_index = str_get_index_of_trailing(ret_val, ' ');
    //
    if(trim_index >= 0)
    {
        ret_val[trim_index] = '\0';
    }

    return ret_val;
}
