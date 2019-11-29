
// Marcel Timm, RhinoDevel, 2019nov29

#include "director.h"
#include "../alloc/alloc.h"
#include "../str/str.h"
#include "../ff14/source/ff.h"

#include <stdbool.h>
#include <stdint.h>

static DIR * s_dir = 0;
static char * s_dir_path = 0;

char const * director_get_dir_path()
{
    return s_dir_path;
}

char* director_create_name_of_next_entry(bool * const is_dir)
{
    FILINFO info;

    if(s_dir == 0)
    {
        return 0;
    }
    if(s_dir_path == 0)
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
 * - Also returns false, if s_dir or s_dir_path is not 0.
 */
static bool init(char const * const dir_path)
{
    if(s_dir != 0)
    {
        return false;
    }
    if(s_dir_path != 0)
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

    s_dir_path = str_create_copy(dir_path);

    return true;
}

bool director_deinit()
{
    if(s_dir == 0)
    {
        if(s_dir_path != 0)
        {
            return false;
        }
        return true;
    }

    if(s_dir_path == 0)
    {
        return false;
    }

    if(f_closedir(s_dir) != FR_OK)
    {
        return false;
    }

    alloc_free(s_dir);
    s_dir = 0;

    alloc_free(s_dir_path);
    s_dir_path = 0;
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
