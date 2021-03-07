
// Marcel Timm, RhinoDevel, 2020dec20

#include "mode.h"
#include "mode_type.h"
#include "../../lib/cfg/cfg.h"
#include "../../lib/assert.h"
#include "../config.h"

#include <stdint.h>

static char const * const s_dir_path = MT_FILESYS_ROOT;
static char const * const s_filename = "cbmtape.pi";

enum mode_type mode_load()
{
    uint8_t const val = cfg_load(s_dir_path, s_filename);

    if(val == MT_CFG_ERR)
    {
        return mode_type_err;
    }

    return (enum mode_type)val;
}

bool mode_save(enum mode_type const type)
{
    switch(type)
    {
        case mode_type_save: // (falls through)
        case mode_type_pet1: // (falls through)
        case mode_type_pet1tom: // (falls through)
        case mode_type_pet2: // (falls through)
        case mode_type_pet2tom: // (falls through)
        case mode_type_pet4: // (falls through)
        case mode_type_pet4tom:
        {
            return cfg_save(s_dir_path, s_filename, (uint8_t)type);
        }

        case mode_type_err: // (falls through)
        default:
        {
            assert(false);
            return false;
        }
    }
}
