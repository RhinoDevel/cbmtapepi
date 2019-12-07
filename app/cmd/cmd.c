
// Marcel Timm, RhinoDevel, 2019dec03

#include "cmd.h"
#include "../config.h"
#include "../tape/tape_input.h"
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
//                                      "   THEGREAT.PRG "
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

static void exec_remove(char const * const command)
{
    char const * const name_only = command + str_get_len(s_rm);

    filesys_remount();
    dir_reinit(s_cur_dir_path);

    f_unlink(name_only);

    dir_deinit();
    filesys_unmount();
}

static struct cmd_output * exec_load(char const * const command)
{
    char const * const name_only = command + str_get_len(s_load);
    struct cmd_output * o = 0;
    FIL fil;
    UINT read_len = 0;

    filesys_remount();
    dir_reinit(s_cur_dir_path);

    if(f_open(&fil, name_only, FA_READ) != FR_OK)
    {
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
    dir_deinit();
    filesys_unmount();
    return o;
}

static bool exec_cd(char const * const command)
{
    char const * const name_only = command + str_get_len(s_cd);

    filesys_remount();
    dir_reinit(s_cur_dir_path);

    bool const has_sub_dir = dir_has_sub_dir(name_only);

    dir_deinit();
    filesys_unmount();

    if(has_sub_dir)
    {
        char* buf = str_create_concat(s_cur_dir_path, name_only);

        alloc_free(s_cur_dir_path);
        s_cur_dir_path = str_create_concat(buf, "/");
        alloc_free(buf);
        buf = 0;

#ifndef NDEBUG
        console_write("cmd/exec_cd : Changed to \"");
        console_write(s_cur_dir_path);
        console_writeline("\" (full path).");
#endif //NDEBUG

        return true;
    }

#ifndef NDEBUG
        console_write("cmd/exec_cd : Failed to change to \"");
        console_write(name_only);
        console_writeline("\" (sub dir. name).");
#endif //NDEBUG

    return false;
}

static bool exec_save(
    char const * const command, struct tape_input const * const ti)
{
    bool ret_val = false,
        file_is_open = false;
    FIL fil;

    filesys_remount();
    dir_reinit(s_cur_dir_path);

    do
    {
        UINT write_count;
        uint8_t buf;

        // (no overwrite)
        //
        if(f_open(&fil, command, FA_CREATE_NEW | FA_WRITE) != FR_OK)
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
        exec_remove(command);
        return true;
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
