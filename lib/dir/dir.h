
// Marcel Timm, RhinoDevel, 2019nov29

// Singleton to get names of subfolders and files in a directory.

#ifndef MT_DIR
#define MT_DIR

#include <stdbool.h>

char const * dir_get_dir_path();

/** Return '\0' escaped string with the name of the next entry in directory.
 *
 * - Caller takes ownership of returned object.
 * - Returns "\0", if no next entry exists.
 * - Returns 0 on error.
 */
char* dir_create_name_of_next_entry(bool * const is_dir);

/**
 * - OK to be called, if already deinitialized (returns true).
 */
bool dir_deinit();

/**
 * - Filesystem object must already have been mounted [f_mount()].
 */
bool dir_reinit(char const * const dir_path);

#endif //MT_DIR
