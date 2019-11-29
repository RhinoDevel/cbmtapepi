
// Marcel Timm, RhinoDevel, 2019nov29

#ifndef MT_DIRECTOR
#define MT_DIRECTOR

#include <stdbool.h>

/** Return '\0' escaped string with the name of the next entry in directory.
 *
 * - Caller takes ownership of returned object.
 * - Returns "\0", if no next entry exists.
 * - Returns 0 on error.
 */
char* director_create_name_of_next_entry(bool * const is_dir);

/**
 * - OK to be called, if already deinitialized (returns true).
 */
bool director_deinit();

/**
 * - Filesystem object must already have been mounted [f_mount()].
 */
bool director_reinit(char const * const dir_path);

#endif //MT_DIRECTOR
