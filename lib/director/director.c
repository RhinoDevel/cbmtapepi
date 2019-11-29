
// Marcel Timm, RhinoDevel, 2019nov29

#include "director.h"
#include "../alloc/alloc.h"
#include "../str/str.h"
#include "../ff14/source/ff.h"

#include <stdbool.h>
#include <stdint.h>

static DIR * s_dir = 0;

char* director_create_name_of_next_entry(bool * const is_dir)
{
    FILINFO info;

    if(s_dir == 0)
    {
        return 0;
    }

    if(is_dir == 0)
    {
        return 0;
    }

    if(f_readdir(s_dir, &info) != FR_OK)
    {
        return 0;
    }

    uint32_t const len = str_get_len(info.fname);
    char * const ret_val = alloc_alloc((len + 1) * sizeof *ret_val);

    if(ret_val == 0)
    {
        return 0;
    }

    str_copy(ret_val, info.fname);

    *is_dir = (info.fattrib & AM_DIR) != 0;

    return ret_val;
}

/**
 * - Also returns false, if s_dir is not 0.
 */
static bool init(char const * const dir_path)
{
    if(s_dir != 0)
    {
        return false;
    }

    s_dir = alloc_alloc(sizeof *s_dir);
    if(s_dir == 0)
    {
        return false;
    }

    if(f_opendir(s_dir, dir_path) != FR_OK)
    {
        return false;
    }
    return true;
}

bool director_deinit()
{
    if(s_dir == 0)
    {
        return true;
    }

    if(f_closedir(s_dir) != FR_OK)
    {
        return false;
    }

    alloc_free(s_dir);
    s_dir = 0;
    return true;
}

bool director_reinit(char const * const dir_path)
{
    if(s_dir != 0)
    {
        if(!director_deinit())
        {
            return false;
        }
    }
    return init(dir_path);
}
