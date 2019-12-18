
#include "petload.h"
#include "../cbm/cbm_send.h"
#include "../tape/tape_input.h"
#include "../../lib/alloc/alloc.h"
#include "../../lib/basic/basic_addr.h"
#include "../../lib/basic/basic.h"
#include "../../lib/assert.h"

#include <stdint.h>

static char const * const s_name = "petload";

// *** BASIC v2 / Rev. 3 ROMs: ***

// static uint16_t const s_addr_key_buf_char_count = 158/*0x009E*/;
//
// Number of characters currently stored in keyboard buffer.

//static uint16_t const s_addr_key_buf = 623/*0x026F*/; // Keyboard buffer.
static uint16_t const s_addr_tape_buf_one = 634/*0x027A*/; // Tape #1 buffer.

// *** ***

static uint16_t const s_addr_offset = 5 + MT_TAPE_INPUT_NAME_LEN; // Magic.

struct tape_input * petload_create()
{
    struct tape_input * const ret_val = alloc_alloc(sizeof *ret_val);
    uint32_t len_buf = 0;
    int i = 0;

    cbm_send_fill_name(ret_val->name, s_name);

    ret_val->type = tape_filetype_relocatable; // Correct for PET.

    ret_val->addr = MT_BASIC_ADDR_PET;
    ret_val->bytes = basic_get_sys(
        ret_val->addr,
        s_addr_tape_buf_one + s_addr_offset,
        true,
        &len_buf);

    assert(len_buf > 0 && len_buf <= 0x0000FFFF);

    ret_val->len = (uint16_t)len_buf;

    ret_val->add_bytes[i++] = 169; // Immediate LDA.
    ret_val->add_bytes[i++] = 83; // Heart symbol (yes, it is romantic).
    ret_val->add_bytes[i++] = 141; // Absolute STA.
    ret_val->add_bytes[i++] = 0; // Lower byte of 32768 (0x8000 - PET video RAM start).
    ret_val->add_bytes[i++] = 128; // Higher byte of 32768.
    ret_val->add_bytes[i++] = 96; // RTS.
    while(i < MT_TAPE_INPUT_ADD_BYTES_LEN)
    {
        ret_val->add_bytes[i++] = 0;
    }

    return ret_val;
}
