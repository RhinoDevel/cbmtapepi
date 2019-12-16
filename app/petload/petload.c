
#include "petload.h"
#include "../cbm/cbm_send.h"
#include "../tape/tape_input.h"
#include "../../lib/alloc/alloc.h"
#include "../../lib/basic/basic_addr.h"
#include "../../lib/basic/basic.h"
#include "../../lib/assert.h"

#include <stdint.h>

static char const * const s_name = "petload";
static uint16_t const s_addr_tape_buf_one = 634/*0x27A*/; // v2 ROM.
static uint16_t const s_addr_offset = 5 + MT_TAPE_INPUT_NAME_LEN; // Magic.
    //
    // TODO: Correct, or 5 + actual name's length?

struct tape_input * petload_create()
{
    struct tape_input * const ret_val = alloc_alloc(sizeof *ret_val);
    uint32_t len_buf = 0;

    cbm_send_fill_name(ret_val->name, s_name);
    ret_val->type = tape_filetype_relocatable; // Correct for PET.
    ret_val->addr = MT_BASIC_ADDR_PET;
    ret_val->bytes = basic_get_sys(
        ret_val->addr,
        s_addr_tape_buf_one + s_addr_offset,
        true,
        &len_buf);

    assert(len_buf > 0 && len_buf <= 0xFFFF);

    ret_val->len = (uint16_t)len_buf;

    // TODO: Fill ret_val->add_bytes with actual loader (behind name)!

    return ret_val;
}
