
// Marcel Timm, RhinoDevel, 2018dec24

// Hard-coded for MiniUART and XMODEM usage.
//
// Warning: console_... is probably linked to MiniUART, too!

#include "ui_terminal_to_commodore.h"

#include "../config.h"
#include "../console/console.h"
#include "../alloc/alloc.h"
#include "../xmodem/xmodem.h"
#include "../xmodem/xmodem_receive_params.h"
#include "../miniuart/miniuart.h"
#include "../tape/tape_send.h"
#include "../tape/tape_input.h"
#include "../tape/tape_send_params.h"

static void fill_name(uint8_t * const name)
{
    static uint8_t const sample_name[] = {
        'R', 'H', 'I', 'N', 'O', 'D', 'E', 'V', 'E', 'L',
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20
    };

    for(int i = 0;i<MT_TAPE_INPUT_NAME_LEN;++i)
    {
        name[i] = sample_name[i];
    }
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

static bool send_to_commodore(uint8_t const * const bytes, uint32_t const count)
{
    bool ret_val = false;
    struct tape_send_params p;
    uint32_t * const mem_addr = alloc_alloc(4 * 1024 * 1024); // Hard-coded

    p.gpio_pin_nr_read = MT_TAPE_GPIO_PIN_NR_READ;
    p.gpio_pin_nr_sense = MT_TAPE_GPIO_PIN_NR_SENSE;
    p.gpio_pin_nr_motor = MT_TAPE_GPIO_PIN_NR_MOTOR;
    p.data = alloc_alloc(sizeof *(p.data));

    fill_name(p.data->name);

    p.data->type = tape_filetype_relocatable; // (necessary for PET PRG file)
    //
    // Hard-coded - maybe not always correct!

    p.data->addr = *((uint16_t const *)bytes);
    p.data->bytes = bytes + 2;
    p.data->len = count - 2;
    fill_add_bytes(p.data->add_bytes);

    console_write("Start address is 0x");
    console_write_word(p.data->addr);
    console_writeline(".");

    hint();
    ret_val = tape_send(&p, mem_addr);

    alloc_free(mem_addr);
    alloc_free(p.data);
    return ret_val;
}

void ui_terminal_to_commodore()
{
    static uint32_t const len = 64 * 1024; // 64 kB.

    struct xmodem_receive_params p;
    uint32_t count = 0;

    p.write_byte = miniuart_write_byte;
    p.read_byte = miniuart_read_byte;
    p.is_ready_to_read = miniuart_is_ready_to_read;
    p.buf = alloc_alloc(len * sizeof (uint8_t));
    p.len = len;

    console_writeline("Please send your file, now (via XMODEM).");
    while(!xmodem_receive(&p, &count))
    {
        console_writeline("Failed to receive file! Retrying..");
    }
    console_write("Received 0x");
    console_write_dword(count);
    console_writeline(" bytes.");

    send_to_commodore(p.buf, count);

    alloc_free(p.buf);
    p.buf = 0;
}
