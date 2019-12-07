
// Marcel Timm, RhinoDevel, 2019nov29

#include "dir.h"
#include "../alloc/alloc.h"
#include "../sort/sort.h"
#include "../str/str.h"
#include "../ff14/source/ff.h"

#include <stdbool.h>
#include <stdint.h>

static DIR * s_dir = 0;
static char * s_dir_path = 0;

static bool rewind()
{
    if(s_dir == 0)
    {
        return false;
    }
    if(s_dir_path == 0)
    {
        return false;
    }
    if(f_readdir(s_dir, 0) != FR_OK)
    {
        return false;
    }
    return true;
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

char const * dir_get_dir_path()
{
    return s_dir_path;
}

char* dir_create_name_of_next_entry(bool * const is_dir)
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

    *is_dir = (info.fattrib & AM_DIR) != 0;

    return str_create_copy(info.fname);
}

/**
 * - Caller takes ownership of returned object.
 */
static struct dir_entry * create_entry_from_next_entry()
{
    struct dir_entry * entry = 0;

    if(s_dir == 0)
    {
        return 0;
    }
    if(s_dir_path == 0)
    {
        return 0;
    }

    entry = alloc_alloc(sizeof *entry);
    if(entry == 0)
    {
        return 0;
    }

    entry->name = dir_create_name_of_next_entry(&entry->is_dir);
    if(entry->name == 0)
    {
        alloc_free(entry);
        return 0;
    }

    return entry;
}

static int cmp_dir_entry(void const * const a, void const * const b)
{
    struct dir_entry const * const e_a = *(struct dir_entry const * const *)a;
    struct dir_entry const * const e_b = *(struct dir_entry const * const *)b;

    if(e_a->is_dir != e_b->is_dir)
    {
        if(e_a->is_dir)
        {
            return -1;
        }
        return 1;
    }
    return str_cmp(e_a->name, e_b->name);
}

/**
 * - Rewinds to first entry!
 * - Returns -1 in error.
 */
static int get_entry_count()
{
    int count = 0;
    FILINFO info;

    if(s_dir == 0)
    {
        return -1;
    }
    if(s_dir_path == 0)
    {
        return -1;
    }

    if(!rewind())
    {
        return -1;
    }

    do
    {
        if(f_readdir(s_dir, &info) != FR_OK)
        {
            return -1;
        }
        if(info.fname[0] == '\0')
        {
            break;
        }

        ++count;
    }while(true);

    if(!rewind())
    {
        return -1;
    }

    return count;
}

void dir_free_entry_arr(struct dir_entry * * arr, int const len)
{
    for(int i = 0;i < len;++i)
    {
        alloc_free(arr[i]->name);
        arr[i]->name = 0;

        alloc_free(arr[i]);
    }
    alloc_free(arr);
}

struct dir_entry * * dir_create_entry_arr(int * const count)
{
    struct dir_entry * * arr = 0;

    *count = get_entry_count(); // (rewinds)
    if(*count <= 0) // -1 <=> error, 0 <=> no entries.
    {
        return 0;
    }

    arr = alloc_alloc(*count * sizeof *arr);
    if(arr == 0)
    {
        *count = -1;
        return 0;
    }

    for(int i = 0;i < *count; ++i)
    {
        arr[i] = create_entry_from_next_entry();
        if(arr[i] == 0)
        {
            dir_free_entry_arr(arr, i);
            *count = -1;
            return 0;
        }
    }

    if(!rewind())
    {
        dir_free_entry_arr(arr, *count);
        *count = -1;
        return 0;
    }

    sort_insertion(arr, *count, sizeof *arr, cmp_dir_entry);

    return arr;
}

bool dir_has_sub_dir(char const * const name)
{
    FILINFO info;

    if(s_dir == 0)
    {
        return false;
    }
    if(s_dir_path == 0)
    {
        return false;
    }

    if(f_stat(name, &info) != FR_OK)
    {
        return false;
    }
    return (info.fattrib & AM_DIR) != 0;
}

bool dir_deinit()
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

bool dir_reinit(char const * const dir_path)
{
    if(s_dir != 0)
    {
        if(!dir_deinit())
        {
            return false;
        }
    }
    return init(dir_path);
}
