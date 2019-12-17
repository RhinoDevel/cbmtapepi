
// Marcel Timm, RhinoDevel, 2019sep15

#include "basic.h"
#include "basic_token.h"
#include "../alloc/alloc.h"
#include "../petasc/petasc.h"
#include "../str/str.h"
#include "../calc/calc.h"
#include "../assert.h"

#include <stdint.h>
#include <stdbool.h>

static uint8_t const s_end_of_line = 0x00;
static uint16_t const s_end_of_prg = 0x0000;
static uint16_t const s_line_first = 1000;
static uint16_t const s_line_step = 10;

uint8_t* basic_get_prints(
    uint16_t const addr,
    char const * const * const str_arr,
    uint32_t const str_count,
    char const petscii_not_found_replacer,
    uint32_t * const len)
{
    assert(addr > 0);
    assert(str_arr != 0);
    assert(str_count > 0);
    assert(len != 0);

    uint32_t i = 0;
    uint16_t line_nr = s_line_first;

    if(str_get_len_max(str_arr, str_count) > MT_BASIC_MAX_CHAR_PER_LOGICAL_LINE)
    {
        assert(false);
        return 0;
    }

    uint32_t const str_len_all = str_get_len_sum(str_arr, str_count);

    *len = 2 // Address
        + ( // One line.
            2 // Link to next line.
            + 2 // Line number.
            + 1 // BASIC token PRINT.
            + 1 // "
            // (actual string gets here)
            + 1 // "
            + 1 // End of line.
        ) * str_count

        + 2 // End of program.

        + str_len_all; // Space for actual strings.

    uint8_t * const ret_val = alloc_alloc(*len);

    // *** Address: ***

    ret_val[i++] = (uint8_t)(addr & 0x00FF);
    ret_val[i++] = (uint8_t)(addr >> 8);

    for(uint32_t j = 0;j < str_count;++j)
    {
        uint32_t const len_str = str_get_len(str_arr[j]);
        uint16_t const next_line_addr = addr
            + (uint16_t)i // Alr. written byte count.
            + 2 // Link to next line.
            + 2 // Line number.
            + 1 // BASIC token PRINT.
            + 1 // "
            + len_str // Actual string.
            + 1 // "
            + 1; // End of line.

        uint32_t k = 0;

        // *** Link to next line: ***

        ret_val[i++] = (uint8_t)(next_line_addr & 0x00FF);
        ret_val[i++] = (uint8_t)(next_line_addr >> 8);

        // *** Line number: ***

        ret_val[i++] = (uint8_t)(line_nr & 0x00FF);
        ret_val[i++] = (uint8_t)(line_nr >> 8);
        line_nr += s_line_step;

        // *** Line content: ***

        ret_val[i++] = (uint8_t)basic_token_print; // PRINT

        ret_val[i++] = '"';
        while(str_arr[j][k] != '\0')
        {
            ret_val[i++] = petasc_get_petscii(
                str_arr[j][k], petscii_not_found_replacer);
            ++k;
        }
        ret_val[i++] = '"';

        // *** End of line: ***

        ret_val[i++] = s_end_of_line;
    }

    // *** End of program: ***

    ret_val[i++] = (uint8_t)(s_end_of_prg & 0x00FF);
    ret_val[i++] = (uint8_t)(s_end_of_prg >> 8);

    assert(i == *len);

    return ret_val;
}

uint8_t* basic_get_sample(uint16_t const addr, uint32_t * const len)
{
    assert(addr > 0);
    assert(len != 0);

    static char const * const print_val = "GREETINGS FROM CBMTAPEPI!";
    uint32_t i = 0, j = 0;
    uint16_t line_nr = s_line_first;

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

    ret_val[i++] = (uint8_t)(line_nr & 0x00FF);
    ret_val[i++] = (uint8_t)(line_nr >> 8);
    line_nr += s_line_step;

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

uint8_t* basic_get_sys(
    uint16_t const addr,
    uint16_t const sys_addr,
    bool const skip_addr,
    uint32_t * const len)
{
    assert(addr > 0);
    assert(sys_addr > 0);
    assert(len != 0);

    static uint32_t const sys_addr_len = 5;
    uint32_t i = 0, j = 0;
    uint16_t line_nr = s_line_first;
    char sys_addr_dec[sys_addr_len];

    calc_word_to_dec(sys_addr, sys_addr_dec);
    while(j < sys_addr_len && sys_addr_dec[j] == '0')
    {
        ++j;
    }

    *len = (skip_addr ? 0 : 2) // Address
        + 2 // Link to next line.
        + 2 // Line number.
        + 1 // BASIC token SYS.
        + (sys_addr_len - j) // Real length of SYS address.
        + 1 // End of line.
        + 2; // End of program.

    uint8_t * const ret_val = alloc_alloc(*len);

    if(!skip_addr)
    {
        // *** Address: ***

        ret_val[i++] = (uint8_t)(addr & 0x00FF);
        ret_val[i++] = (uint8_t)(addr >> 8);
    }

    // *** Link to next line: ***

    uint16_t const next_line_addr = addr + ((uint16_t)(*len)) - 2;

    ret_val[i++] = (uint8_t)(next_line_addr & 0x00FF);
    ret_val[i++] = (uint8_t)(next_line_addr >> 8);

    // *** Line number: ***

    ret_val[i++] = (uint8_t)(s_line_first & 0x00FF);
    ret_val[i++] = (uint8_t)(s_line_first >> 8);
    line_nr += s_line_step;

    // *** Line content: ***

    ret_val[i++] = (uint8_t)basic_token_sys; // SYS

    while(j < sys_addr_len)
    {
        ret_val[i++] = sys_addr_dec[j];
        ++j;
    }

    // *** End of line: ***

    ret_val[i++] = s_end_of_line;

    // *** End of program: ***

    ret_val[i++] = (uint8_t)(s_end_of_prg & 0x00FF);
    ret_val[i++] = (uint8_t)(s_end_of_prg >> 8);

    assert(i == *len);

    return ret_val;
}
