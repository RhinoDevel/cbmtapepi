
// Marcel Timm, RhinoDevel, 2019nov29

#include "dir.h"
#include "../alloc/alloc.h"
#include "../sort/sort.h"
#include "../str/str.h"

#ifdef MT_LINUX
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <dirent.h>
    #include <errno.h>
#else //MT_LINUX
    #include "../ff14/source/ff.h"
#endif //MT_LINUX

#ifndef NDEBUG
    #include "../console/console.h"
#endif //NDEBUG

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

#ifdef MT_LINUX
    rewinddir(s_dir);
#else //MT_LINUX
    if(f_readdir(s_dir, 0) != FR_OK)
    {
        return false;
    }
    return true;
#endif //MT_LINUX
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

#ifdef MT_LINUX
    s_dir = opendir(dir_path);
    if(s_dir == 0)
    {
        return false;
    }
#else //MT_LINUX
    s_dir = alloc_alloc(sizeof *s_dir);
    if(s_dir == 0)
    {
        return false;
    }

    FRESULT const r = f_opendir(s_dir, dir_path);
    //
    if(r != FR_OK)
    {
#ifndef NDEBUG
        console_write("dir/init : Error: Failed to open dir. at \"");
        console_write(dir_path);
        console_write("\" (error code: ");
        console_write_dword_dec((uint32_t)r);
        console_writeline(")!");
#endif //NDEBUG
        return false;
    }
#endif //MT_LINUX

    s_dir_path = str_create_copy(dir_path);

    return true;
}

char const * dir_get_dir_path()
{
    return s_dir_path;
}

char* dir_create_name_of_next_entry(bool * const is_dir)
{
#ifdef MT_LINUX
    struct dirent * info;
#else //MT_LINUX
    FILINFO info;
#endif //MT_LINUX

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

#ifdef MT_LINUX
    errno = 0;
    info = readdir(s_dir);
    if(info == 0)
    {
        *is_dir = false;
        if(errno != 0)
        {
            return 0;
        }
        return str_create_copy("");
    }
    
    // Better use dir_is_file(), here?
    //
    if(info->d_type == DT_UNKNOWN)
    {
        *is_dir = false;
        return 0;
    }
    *is_dir = info->d_type == DT_DIR;

    return str_create_copy(info->d_name);
#else //MT_LINUX
    if(f_readdir(s_dir, &info) != FR_OK)
    {
        return 0;
    }

    *is_dir = (info.fattrib & AM_DIR) != 0;

    return str_create_copy(info.fname);
#endif //MT_LINUX
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
#ifndef MT_LINUX
    FILINFO info;
#endif //MT_LINUX

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
#ifdef MT_LINUX
        errno = 0;
        if(readdir(s_dir) == 0)
        {
            if(errno != 0)
            {
                return -1;
            }
            break;
        }
#else //MT_LINUX
        if(f_readdir(s_dir, &info) != FR_OK)
        {
            return -1;
        }
        if(info.fname[0] == '\0')
        {
            break;
        }
#endif //MT_LINUX

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
#ifdef MT_LINUX
    struct stat info;
#else //MT_LINUX
    FILINFO info;
#endif //MT_LINUX
    char* full_path = 0;

    if(s_dir == 0)
    {
        return false;
    }
    if(s_dir_path == 0)
    {
        return false;
    }

    full_path = dir_create_full_path(s_dir_path, name);

#ifdef MT_LINUX
    int const r = stat(full_path, &info);
#else //MT_LINUX
    FRESULT const r = f_stat(full_path, &info);
#endif //MT_LINUX

    alloc_free(full_path);
    full_path = 0;

#ifdef MT_LINUX
    if(r != 0)
#else //MT_LINUX
    if(r != FR_OK)
#endif //MT_LINUX
    {
        return false;
    }
#ifdef MT_LINUX
    return (info.st_mode & S_IFDIR) != 0;
#else //MT_LINUX
    return (info.fattrib & AM_DIR) != 0;
#endif //MT_LINUX
}

bool dir_is_file(char const * const name)
{
#ifdef MT_LINUX
    struct stat info;
#else //MT_LINUX
    FILINFO info;
#endif //MT_LINUX
    char* full_path = 0;

    if(s_dir == 0)
    {
        return false;
    }
    if(s_dir_path == 0)
    {
        return false;
    }

    full_path = dir_create_full_path(s_dir_path, name);

#ifdef MT_LINUX
    int const r = stat(full_path, &info);
#else //MT_LINUX
    FRESULT const r = f_stat(full_path, &info);
#endif //MT_LINUX

    alloc_free(full_path);
    full_path = 0;

#ifdef MT_LINUX
    if(r != 0)
#else //MT_LINUX
    if(r != FR_OK)
#endif //MT_LINUX
    {
        return false;
    }
#ifdef MT_LINUX
    return (info.st_mode & S_IFDIR) == 0;
#else //MT_LINUX
    return (info.fattrib & AM_DIR) == 0;
#endif //MT_LINUX
}

bool dir_deinit()
{
    if(s_dir_path == 0)
    {
        return true;
    }
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

#ifdef MT_LINUX
    if(closedir(s_dir) != 0)
#else //MT_LINUX
    if(f_closedir(s_dir) != FR_OK)
#endif //MT_LINUX
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

char* dir_create_full_path(
    char const * const dir_path, char const * const entry_name)
{
    uint32_t const dir_path_len = str_get_len(dir_path);

    if(dir_path_len < 1)
    {
        return 0;
    }

    if(dir_path[dir_path_len - 1] == '/')
    {
        return str_create_concat(dir_path, entry_name);
    }
    return str_create_concat_three(dir_path, "/", entry_name);
}
