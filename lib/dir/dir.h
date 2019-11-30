
// Marcel Timm, RhinoDevel, 2019nov29

// Singleton to get names of subfolders and files in a directory.

#ifndef MT_DIR
#define MT_DIR

#include <stdbool.h>

struct dir_entry
{
    char* name;
    bool is_dir;
};

/**
 * - Helper method that has nothing to do with the singleton.
 */
void dir_free_entry_arr(struct dir_entry * * arr, int const len);

char const * dir_get_dir_path();

/** Return '\0' escaped string with the name of the next entry in directory.
 *
 * - An order of any kind is not guaranteed.
 * - Caller takes ownership of returned object.
 * - Returns "\0", if no next entry exists.
 * - Returns 0 on error.
 */
char* dir_create_name_of_next_entry(bool * const is_dir);

/**
 * - Returns 0 on error (count == -1)
 *   or count of entries being zero (count == 0).
 * - Assumes given pointer not to be set to 0 (no error check).
 * - Rewinds (before array creation and after, if no error occurred).
 */
struct dir_entry * * dir_create_entry_arr(int * const count);

/**
 * - OK to be called, if already deinitialized (returns true).
 */
bool dir_deinit();

/**
 * - Filesystem object must already have been mounted [f_mount()].
 */
bool dir_reinit(char const * const dir_path);

#endif //MT_DIR
