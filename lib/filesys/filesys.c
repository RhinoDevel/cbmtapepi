
// Marcel Timm, RhinoDevel, 2019dec03

#include "filesys.h"
#include "../alloc/alloc.h"
#include "../assert.h"
#include "../dir/dir.h"

#ifdef MT_LINUX
    #include <stdio.h>
    #include <unistd.h>
    #include <sys/stat.h>
#else //MT_LINUX    
    #include "../ff14/source/ff.h"
#endif //MT_LINUX

#ifndef NDEBUG
    #include "../../lib/console/console.h"
#endif //NDEBUG

#include <stdbool.h>
#include <stdint.h>

#ifndef MT_LINUX
    static FATFS * s_fs = 0;
#endif //MT_LINUX

#ifndef MT_LINUX
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
#endif //MT_LINUX

bool filesys_unmount()
{
#ifndef MT_LINUX // No need to unmount something in Linux.
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
#endif //MT_LINUX
    return true;
}

bool filesys_remount()
{
#ifdef MT_LINUX // Doing nothing in Linux.
    return true;
#else //MT_LINUX
    if(s_fs != 0)
    {
        if(!filesys_unmount())
        {
            return false;
        }
    }
    return mount();
#endif //MT_LINUX
}

#ifdef MT_LINUX
static off_t get_size(char const * const path)
{
    assert(path != NULL);

    struct stat s;

    if(stat(path, &s) == 0)
    {
        return s.st_size;
    }
    return -1;
}

static uint8_t* load(
    char const * const full_path, uint32_t * const out_byte_count)
{
    assert(out_byte_count != 0);

    *out_byte_count = -1;

    off_t const signed_size = get_size(full_path);

    if(signed_size == -1)
    {
#ifndef NDEBUG
        console_writeline("load : Error: Failed to get size of file!");
#endif //NDEBUG
        return 0;
    }

    FILE * const file = fopen(full_path, "rb");

    if(file == 0)
    {
#ifndef NDEBUG
        console_writeline("load : Error: Failed to open source file!");
#endif //NDEBUG
        return 0;
    }

    size_t const size = (size_t)signed_size;
    uint8_t * const buf = alloc_alloc(size * sizeof *buf);

    if(fread(buf, sizeof(*buf), size, file) != size)
    {
#ifndef NDEBUG
        console_writeline(
            "load : Error: Failed to completely load file content!");
#endif //NDEBUG
        return 0;
    }

    fclose(file); // TODO: Add error check!
    *out_byte_count = (uint32_t)signed_size;
    return buf;
}
#else //MT_LINUX
static uint8_t* load(
    char const * const full_path, uint32_t * const out_byte_count)
{
    assert(out_byte_count != 0);

    FIL fil;
    UINT read_len = 0;

    if(f_open(&fil, full_path, FA_READ) != FR_OK)
    {
#ifndef NDEBUG
        console_write("load : Error: Opening file \"");
        console_write(full_path);
        console_writeline("\" failed!");
#endif //NDEBUG
        *out_byte_count = 0;
        return 0;
    }

    uint32_t const count = (uint32_t)f_size(&fil);

    uint8_t * const bytes = alloc_alloc(count * sizeof *bytes + 1);
    //
    // (+1 to support empty files)

    f_read(&fil, bytes, (UINT)count, &read_len); // TODO: Add error check!

#ifndef NDEBUG
    assert(count == read_len);

    console_write("load: Read ");
    console_write_dword_dec(read_len);
    console_writeline(" byte(-s).");
#endif

    f_close(&fil); // TODO: Add error check!
    *out_byte_count = read_len;
    return bytes;
}
#endif //MT_LINUX

uint8_t* filesys_load(
    char const * const dir_path,
    char const * const filename,
    uint32_t * const out_byte_count)
{
    assert(out_byte_count != 0);

    filesys_remount();
    dir_reinit(dir_path);

    char * const full_path = dir_create_full_path(dir_path, filename);
    uint8_t * const ret_val = load(full_path, out_byte_count);
   
    // (called function debug-logs on error)
    assert((ret_val == 0) == (*out_byte_count == 0));
    alloc_free(full_path);
    dir_deinit();
    filesys_unmount();
    return ret_val;
}

#ifdef MT_LINUX
static bool remove_from(char const * const full_path)
{
    return unlink(full_path) == 0;
}
#else //MT_LINUX
static bool remove_from(char const * const full_path)
{
    // It seems to be better to make sure that the file exists, before this:
    //
    return f_unlink(full_path) == FR_OK;
}
#endif //MT_LINUX

#ifdef MT_LINUX
static bool save(
    char const * const full_path,
    uint8_t const * const bytes,
    uint32_t const byte_count,
    bool const overwrite)
{
    assert(full_path != 0);
    
    if(overwrite && dir_is_file(full_path))
    {
        if(!remove_from(full_path))
        {
#ifndef NDEBUG
            console_writeline(
                "save : Error: Failed to remove existing file, first!");
#endif //NDEBUG
            return false;   
        }
    }

    FILE * const file = fopen(full_path, "wb");

    if(file == 0)
    {
#ifndef NDEBUG
        console_writeline("save : Error: Failed to create file!");
#endif //NDEBUG
        return false;
    }

    if(byte_count == 0)
    {
        // Empty file.

        assert(bytes == 0);
        fclose(file);
        return true;
    }

    assert(bytes != 0);

    size_t const written_bytes = fwrite(
        bytes, sizeof *bytes, (size_t)byte_count, file);
    
    assert(sizeof *bytes == 1);
    if(written_bytes != (size_t)byte_count)
    {
#ifndef NDEBUG
        console_writeline("save : Error: Failed to write all bytes to file!");
#endif //NDEBUG
        return false;   
    }

    if(fclose(file) != 0)
    {
#ifndef NDEBUG
        console_writeline("save : Error: Failed to close file!");
#endif //NDEBUG
        return false;   
    }
    return true;
}
#else //MT_LINUX
static bool save(
    char const * const full_path,
    uint8_t const * const bytes,
    uint32_t const byte_count,
    bool const overwrite)
{
    bool ret_val = false,
        file_is_open = false;
    FIL fil;

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

        FRESULT const open_result = f_open(&fil, full_path, mode);

        if(open_result != FR_OK)
        {
#ifndef NDEBUG
            console_write("save : Error: Opening file \"");
            console_write(full_path);
            console_write("\" with mode 0x");
            console_write_byte((uint8_t)mode);
            console_write(" failed with err. nr. ");
            console_write_dword_dec((uint32_t)open_result);
            console_writeline("!");
#endif //NDEBUG
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
            f_unlink(full_path);
        }
    }
    return ret_val;
}
#endif //MT_LINUX

bool filesys_save(
    char const * const dir_path,
    char const * const filename,
    uint8_t const * const bytes,
    uint32_t const byte_count,
    bool const overwrite)
{
    bool ret_val = false;

    filesys_remount();
    dir_reinit(dir_path);

    char * const full_path = dir_create_full_path(dir_path, filename);

    ret_val = save(full_path, bytes, byte_count, overwrite);
    alloc_free(full_path);
    dir_deinit();
    filesys_unmount();
    return ret_val;
}

bool filesys_remove(char const * const dir_path, char const * const filename)
{
    filesys_remount();
    dir_reinit(dir_path);
    
    char * const full_path = dir_create_full_path(dir_path, filename);
    bool const ret_val = dir_is_file(filename) && remove_from(full_path);

    alloc_free(full_path);
    dir_deinit();
    filesys_unmount();

    return ret_val;
}
