
// Marcel Timm, RhinoDevel, 2019sep15

#include "basic.h"
#include "basic_token.h"
#include "../alloc/alloc.h"
#include "../assert.h"

#include <stdint.h>

static uint8_t const s_end_of_line = 0x00;
static uint16_t const s_end_of_prg = 0x0000;
static uint16_t const s_line_first = 1000;
static uint16_t const s_line_step = 10;

uint8_t* basic_get_sample(uint16_t const addr, uint32_t * const len)
{
    assert(addr > 0);
    assert(len != 0);

    static char const * const print_val = "GREETINGS FROM CBMTAPEPI!";
    uint32_t i = 0, j = 0;
    uint16_t lineNr = s_line_first;

    *len = 12 + 25; // Hard-coded

    uint8_t * const ret_val = alloc_alloc(*len);

    // *** Address: ***

    ret_val[i++] = (uint8_t)(addr & 0x00FF);
    ret_val[i++] = (uint8_t)(addr >> 8);

    // *** Link to next line: ***

    uint16_t const next_line_addr = addr + ((uint16_t)(*len)) - 2;

    ret_val[i++] = (uint8_t)(next_line_addr & 0x00FF);
    ret_val[i++] = (uint8_t)(next_line_addr >> 8);

    // *** Line number: ***

    ret_val[i++] = (uint8_t)(lineNr & 0x00FF);
    ret_val[i++] = (uint8_t)(lineNr >> 8);
    lineNr += s_line_step;

    // *** Line content: ***

    ret_val[i++] = (uint8_t)basic_token_print; // PRINT

    ret_val[i++] = '"';
    while(print_val[j] != '\0')
    {
        ret_val[i++] = print_val[j];
        ++j;
    }
    ret_val[i++] = '"';

    // *** End of line: ***

    ret_val[i++] = s_end_of_line;

    // *** End of program: ***

    ret_val[i++] = (uint8_t)(s_end_of_prg & 0x00FF);
    ret_val[i++] = (uint8_t)(s_end_of_prg >> 8);

    assert(i == *len);

    return ret_val;
}
