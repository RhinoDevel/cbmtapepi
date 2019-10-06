
// Marcel Timm, RhinoDevel, 2019oct06

#include "tape_guess_filetype.h"

#include "../../lib/basic/basic_addr.h"

#ifndef NDEBUG
    #include "../../lib/console/console.h"
#endif //NDEBUG

enum tape_filetype tape_guess_filetype(
    uint8_t const * const bytes, uint16_t const len)
{
    if(bytes == 0 || len == 0)
    {
        return tape_filetype_unknown;
    }

    uint16_t const addr = (uint16_t)bytes[1] << 8 | (uint16_t)bytes[0];

#ifndef NDEBUG
    console_write("tape_guess_filetype : Address = 0x");
    console_write_word(addr);
    console_write(" / ");
    console_write_word_dec(addr);
    console_writeline(".");
#endif //NDEBUG

    switch(addr)
    {
        case MT_BASIC_ADDR_PET: // (falls through)
        case MT_BASIC_ADDR_VIC: // (falls through)
        case MT_BASIC_ADDR_C64:
        {
            return tape_filetype_relocatable;
        }

        default:
        {
            console_deb_writeline("tape_guess_filetype : No idea, returning non-relocatable/assembler file type..");
            return tape_filetype_non_relocatable; // TODO: Test, if this works for PET!
        }
    }
}
