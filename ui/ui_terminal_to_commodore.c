
// Marcel Timm, RhinoDevel, 2018dec24

// Hard-coded for MiniUART and XMODEM usage.
//
// Warning: console_... is probably linked to MiniUART, too!

#include "ui_terminal_to_commodore.h"

#include "../config.h"
#include "../console/console.h"
#include "../alloc/alloc.h"
#include "../ymodem/ymodem.h"
#include "../ymodem/ymodem_receive_params.h"
#include "../ymodem/ymodem_receive_err.h"
#include "../miniuart/miniuart.h"
#include "../tape/tape_send.h"
#include "../tape/tape_input.h"
#include "../tape/tape_send_params.h"
#include "../str/str.h"

static void fill_name(uint8_t * const name_out, char const * const name_in)
{
    // static uint8_t const sample_name[] = {
    //     'R', 'H', 'I', 'N', 'O', 'D', 'E', 'V', 'E', 'L',
    //     0x20, 0x20, 0x20, 0x20, 0x20, 0x20
    // };

    int i = 0;
    char * const buf = alloc_alloc(str_get_len(name_in) + 1);

    str_to_upper(buf, name_in);

    while(i < MT_TAPE_INPUT_NAME_LEN && buf[i] != '\0')
    {
        name_out[i] = (uint8_t)buf[i]; // TODO: Implement real conversion to PETSCII.
        ++i;
    }
    while(i < MT_TAPE_INPUT_NAME_LEN)
    {
        name_out[i] = 0x20;
        ++i;
    }

    alloc_free(buf);
}

static void fill_add_bytes(uint8_t * const add_bytes)
{
    // Additional bytes (to be stored in header):
    //
    for(int i = 0;i<MT_TAPE_INPUT_ADD_BYTES_LEN;++i)
    {
        add_bytes[i] = 0x20;
    }
}

static void hint()
{
    char c[2];

    console_write(
        "Prepare Commodore, now (e.g. enter LOAD and press Return key). Then press anykey (here)..");
    console_read(c, 2);
    console_writeline("");
}

static bool send_to_commodore(
    uint8_t const * const bytes, char const * const name, uint32_t const count)
{
    bool ret_val = false;
    struct tape_send_params p;
    uint32_t * const mem_addr = alloc_alloc(4 * 1024 * 1024); // Hard-coded

    p.gpio_pin_nr_read = MT_TAPE_GPIO_PIN_NR_READ;
    p.gpio_pin_nr_sense = MT_TAPE_GPIO_PIN_NR_SENSE;
    p.gpio_pin_nr_motor = MT_TAPE_GPIO_PIN_NR_MOTOR;
    p.data = alloc_alloc(sizeof *(p.data));

    fill_name(p.data->name, name);

    p.data->type = tape_filetype_relocatable; // (necessary for PET PRG file)
    //
    // Hard-coded - maybe not always correct, but works for C64 and PET,
    // both with machine language and BASIC PRG files.

    // First two bytes hold the start address:
    //
    p.data->addr = *((uint16_t const *)bytes);
    p.data->bytes = bytes + 2;
    p.data->len = count - 2;

    fill_add_bytes(p.data->add_bytes);

    console_write("Start address is 0x");
    console_write_word(p.data->addr);
    console_write(" (");
    console_write_word_dec(p.data->addr);
    console_writeline(").");

    hint();
    ret_val = tape_send(&p, mem_addr);

    alloc_free(mem_addr);
    alloc_free(p.data);
    return ret_val;
}

void ui_terminal_to_commodore()
{
    static uint32_t const len = 64 * 1024; // 64 kB.

    struct ymodem_receive_params p;
    enum ymodem_receive_err e;

    p.write_byte = miniuart_write_byte;
    p.read_byte = miniuart_read_byte;
    p.is_ready_to_read = miniuart_is_ready_to_read;
    p.buf = alloc_alloc(len * sizeof (uint8_t));
    p.buf_len = len;
    p.file_len = 0;
    p.name[0] = '\0';

    console_writeline("Please send your file, now (via YMODEM).");

    e = ymodem_receive(&p);
    if(e != ymodem_receive_err_none)
    {
        console_write("Failed to receive file (error ");
        console_write_dword_dec((uint32_t)e);
        console_writeline(")!");

        alloc_free(p.buf);
        p.buf = 0;
        return;
    }

    console_write("Received file \"");
    console_write(p.name);
    console_write("\" with a length of ");
    console_write_dword_dec(p.file_len);
    console_writeline(" bytes.");

    send_to_commodore(p.buf, p.name, p.file_len);

    alloc_free(p.buf);
    p.buf = 0;
}
