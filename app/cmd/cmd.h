
// Marcel Timm, RhinoDevel, 2019dec03

// Singleton (!) to interpret and react on commands send from Commodore via tape
// SAVE (filename) or with fast load/save PRG.

#ifndef MT_CMD
#define MT_CMD

#include "cmd_output.h"
#include "../tape/tape_input.h"

#include <stdbool.h>

/**
 * - Caller takes ownership of object "returned" via output pointer.
 */
bool cmd_exec(
    char const * const command,
    struct tape_input const * const ti,
    struct cmd_output * * const output);

void cmd_reinit(
    bool (*save_mode)(char const * const),
    char const * const start_dir_path);

/** Helper method to deallocate output object.
 *
 *  - Given pointer is no longer valid, after this.
 *  - Works with 0 given, too (just does nothing).
 */
void cmd_free_output(struct cmd_output * const output);

#endif //MT_CMD
