
// Marcel Timm, RhinoDevel, 2020dec19

#include "cfg.h"
#include "../alloc/alloc.h"
#include "../filesys/filesys.h"
#include "../assert.h"

#include <stdint.h>

static uint8_t const s_err = CFG_ERR;

uint8_t cfg_load(char const * const dir_path, char const * const filename)
{
    uint32_t byte_count = 0;
    uint8_t * const bytes = filesys_load(dir_path, filename, &byte_count);

    if(bytes == 0)
    {
        return s_err;
    }

    uint8_t const mode = bytes[0];

    assert(mode != s_err);

    alloc_free(bytes);

    return mode;
}

bool cfg_save(
    char const * const dir_path,
    char const * const filename,
    uint8_t const mode)
{
    if(mode == s_err)
    {
        assert(false);
        return false;
    }
    return filesys_save(dir_path, filename, &mode, 1, true);
}
