
// Marcel Timm, RhinoDevel, 2019dec03

#include "cmd.h"
#include "../config.h"
#include "../tape/tape_input.h"
#include "../../lib/assert.h"
#include "../../lib/alloc/alloc.h"
#include "../../lib/str/str.h"
#include "../../lib/filesys/filesys.h"
#include "../../lib/dir/dir.h"
#include "../../lib/basic/basic.h"
#include "../../lib/basic/basic_addr.h"
#include "../../lib/ff14/source/ff.h"

#ifndef NDEBUG
    #include "../../lib/console/console.h"
#endif //NDEBUG

#include <stdbool.h>

// - 16 characters available
// - File system support (by choice) limited to 8.3 format.
// => 16 - 8 - 1 - 3 = 4 characters available for commands.
//
//                                      "   thegreat.prg "
static char const * const s_dir   =   "$"; // (no parameters)
static char const * const s_rm    =   "rm ";
static char const * const s_load  =   "*"; // (no space)
static char const * const s_cd   =    "cd "; // Supports "..", too.
// static char const * const s_cp   =    "cp "; // Outp. file name by Pi.
// static char const * const s_mv   =    "mv "; // New file name by Pi.
//
// Anything else. => Really save file.

static char * s_cur_dir_path = 0;
//
// Initialized by cmd_reinit(). Changed by exec_cd().

static struct dir_entry * * create_dir_entry_arr(int * const entry_count)
{
    struct dir_entry * * ret_val = 0;

    *entry_count = -1;

    filesys_remount();
    dir_reinit(s_cur_dir_path);

    ret_val = dir_create_entry_arr(entry_count);

    dir_deinit();
    filesys_unmount();

    return ret_val;
}

static struct cmd_output * exec_dir()
{
    struct cmd_output * ret_val = alloc_alloc(sizeof *ret_val);
    int entry_count = -1;
    struct dir_entry * * entry_arr = create_dir_entry_arr(&entry_count);
    int name_count = 1 + entry_count;
    char * * name_arr = alloc_alloc(name_count * sizeof *name_arr);

    name_arr[0] = str_create_concat(s_cur_dir_path, ":");
    for(int i = 1;i < name_count;++i)
    {
        name_arr[i] = str_create_concat(
            entry_arr[i - 1]->is_dir ? "DIR " : "    ",
            entry_arr[i - 1]->name);
    }

    dir_free_entry_arr(entry_arr, entry_count);
    entry_arr = 0;
    entry_count = -1;

    ret_val->name = str_create_copy("DIRECTORY");
    ret_val->bytes = basic_get_prints(
        MT_BASIC_ADDR_PET, // (should also be OK for VIC20, C64, etc.)
        (char const * *)name_arr,
        name_count,
        MT_PETSCII_REPLACER,
        &(ret_val->count));

    for(int i = 0;i < name_count;++i)
    {
        alloc_free(name_arr[i]);
        name_arr[i] = 0;
    }
    alloc_free(name_arr);
    name_arr = 0;
    name_count = -1;

    return ret_val;
}

/**
 * - No support for deletion of empty subfolders.
 */
static bool exec_remove(char const * const command)
{
    FRESULT r = FR_NO_FILE;
    char const * const name_only = command + str_get_len(s_rm);

    filesys_remount();
    dir_reinit(s_cur_dir_path);

    if(dir_is_file(name_only))
    {
        char * const full_path = dir_create_full_path(
            s_cur_dir_path, name_only);

        r = f_unlink(full_path);

        alloc_free(full_path);
    }

    dir_deinit();
    filesys_unmount();

    return r == FR_OK;
}

static struct cmd_output * exec_load(char const * const command)
{
    char const * const name_only = command + str_get_len(s_load);
    struct cmd_output * o = 0;
    FIL fil;
    UINT read_len = 0;

    filesys_remount();
    dir_reinit(s_cur_dir_path);

    char * const full_path = dir_create_full_path(
        s_cur_dir_path, name_only);

    if(f_open(&fil, full_path, FA_READ) != FR_OK)
    {
        alloc_free(full_path);
        dir_deinit();
        filesys_unmount();
        return 0;
    }

    o = alloc_alloc(sizeof *o);

    o->name  = str_create_copy(name_only);
    o->count = (uint32_t)f_size(&fil);
    o->bytes = alloc_alloc(o->count * sizeof *(o->bytes));

    f_read(&fil, o->bytes, (UINT)o->count, &read_len);

    f_close(&fil);
    alloc_free(full_path);
    dir_deinit();
    filesys_unmount();
    return o;
}

static bool exec_cd(char const * const command)
{
    bool ret_val = false;
    char const * const name_only = command + str_get_len(s_cd);

    do
    {
        bool const is_back_cmd = str_are_equal(name_only, "..");
        char* buf = 0;

        if(is_back_cmd && str_are_equal(s_cur_dir_path, MT_FILESYS_ROOT))
        {
            break;
        }

        if(is_back_cmd)
        {
            int const last_slash_i = str_get_last_index(s_cur_dir_path, '/');

            assert(last_slash_i >= 0);

            buf = str_create_partial_copy(s_cur_dir_path, 0, last_slash_i + 1);

            if(!str_are_equal(buf, MT_FILESYS_ROOT))
            {
                buf[last_slash_i] = '\0'; // (minimal memory waste..)
            }
        }
        else
        {
            filesys_remount();
            dir_reinit(s_cur_dir_path);

            bool const has_sub_dir = dir_has_sub_dir(name_only);

            dir_deinit();
            filesys_unmount();

            if(!has_sub_dir)
            {
                break;
            }

            buf = dir_create_full_path(s_cur_dir_path, name_only);
        }

        alloc_free(s_cur_dir_path);
        s_cur_dir_path = buf;
        buf = 0;
        ret_val = true;
    }while(false);

#ifndef NDEBUG
    if(ret_val)
    {
        console_write("cmd/exec_cd : Changed to \"");
        console_write(s_cur_dir_path);
        console_writeline("\".");
    }
    else
    {
        console_write("cmd/exec_cd : Failed to change to \"");
        console_write(name_only);
        console_write("\" from \"");
        console_write(s_cur_dir_path);
        console_writeline("\".");
    }
#endif //NDEBUG

    return ret_val;
}

static bool exec_save(
    char const * const command, struct tape_input const * const ti)
{
    bool ret_val = false,
        file_is_open = false;
    FIL fil;

    filesys_remount();
    dir_reinit(s_cur_dir_path);

    char * const full_path = dir_create_full_path(
        s_cur_dir_path, command);

    do
    {
        UINT write_count;
        uint8_t buf;

        // (no overwrite)
        //
        if(f_open(&fil, full_path, FA_CREATE_NEW | FA_WRITE) != FR_OK)
        {
            break;
        }
        file_is_open = true;

        buf = (uint8_t)(ti->addr & 0x00FF);
        if(f_write(&fil, &buf, 1, &write_count) != FR_OK)
        {
            break;
        }
        if(write_count != 1)
        {
            break;
        }

        buf = (uint8_t)(ti->addr >> 8);
        if(f_write(&fil, &buf, 1, &write_count) != FR_OK)
        {
            break;
        }
        if(write_count != 1)
        {
            break;
        }

        if(f_write(&fil, ti->bytes, ti->len, &write_count) != FR_OK)
        {
            break;
        }
        if(write_count != ti->len)
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
            f_unlink(command);
        }
    }
    alloc_free(full_path);
    dir_deinit();
    filesys_unmount();
    return ret_val;
}

bool cmd_exec(
    char const * const command,
    struct tape_input const * const ti,
    struct cmd_output * * const output)
{
    *output = 0;

    if(s_cur_dir_path == 0)
    {
        return false;
    }

    if(str_starts_with(command, s_dir))
    {
        *output = exec_dir();
        return true;
    }
    if(str_starts_with(command, s_rm))
    {
        return exec_remove(command);
    }
    if(str_starts_with(command, s_load))
    {
        *output = exec_load(command);
        return *output != 0;
    }
    if(str_starts_with(command, s_cd))
    {
        return exec_cd(command);
    }
    return exec_save(command, ti);
}

void cmd_reinit(char const * const start_dir_path)
{
    if(s_cur_dir_path != 0)
    {
        alloc_free(s_cur_dir_path);
        s_cur_dir_path = 0;
    }
    s_cur_dir_path = str_create_copy(start_dir_path);
}

void cmd_free_output(struct cmd_output * const output)
{
    alloc_free(output->name);
    output->name = 0;

    alloc_free(output->bytes);
    output->bytes = 0;

    output->count = 0;

    alloc_free(output);
}
