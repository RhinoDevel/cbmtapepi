
// Marcel Timm, RhinoDevel, 2019dec03

#include "cmd.h"
#include "../../lib/alloc/alloc.h"
#include "../../lib/str/str.h"
#include "../../lib/filesys/filesys.h"
#include "../../lib/dir/dir.h"
#include "../../lib/basic/basic.h"
#include "../../lib/basic/basic_addr.h"

static char * s_cur_dir_path = 0; // Initialized by cmd_reinit().

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

static struct cmd_output * create_output_dir()
{
    struct cmd_output * ret_val = alloc_alloc(sizeof *ret_val);
    int entry_count = -1;
    struct dir_entry * * entry_arr = create_dir_entry_arr(&entry_count);
    int name_count = 1 + entry_count;
    char * * name_arr = alloc_alloc(name_count * sizeof *name_arr);

    name_arr[0] = str_create_concat(s_cur_dir_path, ":");
    for(int i = 1;i < name_count;++i)
    {
        name_arr[i] = entry_arr[i - 1]->name;
    }

    dir_free_entry_arr(entry_arr, entry_count);
    entry_arr = 0;
    entry_count = -1;

    ret_val->name = str_create_copy("DIRECTORY");
    ret_val->bytes = basic_get_prints(
        MT_BASIC_ADDR_PET, // (should also be OK for VIC20, C64, etc.)
        (char const * *)name_arr,
        name_count,
        &(ret_val->count));

    alloc_free(name_arr[0]);
    name_arr[0] = 0;
    alloc_free(name_arr);
    name_arr = 0;
    name_count = -1;

    return ret_val;
}

struct cmd_output * cmd_create_output(char const * const command)
{
    // - 16 characters available
    // - File system support (by choice) limited to 8.3 format.
    // => 16 - 8 - 1 - 3 = 4 characters available for commands.
    //
    //                                      "   THEGREAT.PRG "
    static char const * const c_dir   =   "$"; // (no parameters)
    // static char const * const c_rm    =   "RM ";
    // static char const * const c_load  =   "*"; // (no space)
    // static char const * const c_cd   =    "cd "; // Supports "..", too.
    // static char const * const c_cp   =    "cp "; // Outp. file name by Pi.
    // static char const * const c_mv   =    "mv "; // New file name by Pi.
    //
    // Anything else. => Really save file.

    if(s_cur_dir_path == 0)
    {
        return 0;
    }
    if(str_starts_with(command, c_dir))
    {
        return create_output_dir();
    }
    return 0; // TODO: Implement!
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
