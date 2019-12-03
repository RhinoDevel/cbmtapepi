
// Marcel Timm, RhinoDevel, 2019dec04

#include "filesys.h"
#include "../alloc/alloc.h"
#include "../ff14/source/ff.h"

#include <stdbool.h>

static FATFS * s_fs = 0;

/**
 * - Also returns false, if s_fs is not 0.
 */
static bool mount()
{
    if(s_fs != 0)
    {
        return false;
    }

    s_fs = alloc_alloc(sizeof *s_fs);
    if(s_fs == 0)
    {
        return false;
    }

    if(f_mount(s_fs, "", 1) != FR_OK) // (forces mount)
    {
        alloc_free(s_fs);
        s_fs = 0;
        return false;
    }

    return true;
}

bool filesys_unmount()
{
    if(s_fs == 0)
    {
        return true;
    }

    if(f_mount(0, "", 0) != FR_OK)
    {
        return false;
    }

    alloc_free(s_fs);
    s_fs = 0;
    return true;
}

bool filesys_remount()
{
    if(s_fs != 0)
    {
        if(!filesys_unmount())
        {
            return false;
        }
    }
    return mount();
}
