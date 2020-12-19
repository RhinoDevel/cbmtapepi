
// Marcel Timm, RhinoDevel, 2019dec03

#include "filesys.h"
#include "../alloc/alloc.h"
#include "../ff14/source/ff.h"
#include "../assert.h"
#include "../dir/dir.h"
#include "../ff14/source/ff.h"

#ifndef NDEBUG
    #include "../../lib/console/console.h"
#endif //NDEBUG

#include <stdbool.h>
#include <stdint.h>

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

uint8_t* filesys_load(
    char const * const dir_path,
    char const * const filename,
    uint32_t * const out_byte_count)
{
    assert(out_byte_count != 0);

    FIL fil;
    UINT read_len = 0;

    filesys_remount();
    dir_reinit(dir_path);

    char * const full_path = dir_create_full_path(dir_path, filename);

    if(f_open(&fil, full_path, FA_READ) != FR_OK)
    {
#ifndef NDEBUG
        console_write("filesys_load : Error: Opening file \"");
        console_write(full_path);
        console_writeline("\" failed!");
#endif //NDEBUG
        alloc_free(full_path);
        dir_deinit();
        filesys_unmount();
        *out_byte_count = 0;
        return 0;
    }

    uint32_t const count = (uint32_t)f_size(&fil);

    uint8_t * const bytes = alloc_alloc(count * sizeof *bytes + 1);
    //
    // (+1 to support empty files)

    f_read(&fil, bytes, (UINT)count, &read_len);

#ifndef NDEBUG
    assert(count == read_len);

    console_write("filesys_load: Read ");
    console_write_dword_dec(read_len);
    console_writeline(" bytes.");
#endif

    f_close(&fil);
    alloc_free(full_path);
    dir_deinit();
    filesys_unmount();

    *out_byte_count = read_len;
    return bytes;
}

bool filesys_save(
    char const * const dir_path,
    char const * const filename,
    uint8_t const * const bytes,
    uint32_t const byte_count,
    bool const overwrite)
{
    bool ret_val = false,
        file_is_open = false;
    FIL fil;

    filesys_remount();
    dir_reinit(dir_path);

    char * const full_path = dir_create_full_path(dir_path, filename);

    do
    {
        UINT write_count;
        BYTE mode = FA_WRITE;

        if(overwrite)
        {
            mode |= FA_CREATE_ALWAYS;
        }
        else
        {
            mode |= FA_CREATE_NEW;
        }

        if(f_open(&fil, full_path, mode) != FR_OK)
        {
            break;
        }
        file_is_open = true;

        if(f_write(&fil, bytes, byte_count, &write_count) != FR_OK)
        {
            break;
        }
        if(write_count != byte_count)
        {
            break;
        }

        ret_val = true;
    }while(false);

    if(file_is_open)
    {
        f_close(&fil);
        file_is_open = false;

        if(!ret_val)
        {
            f_unlink(filename);
        }
    }
    alloc_free(full_path);
    dir_deinit();
    filesys_unmount();
    return ret_val;
}
