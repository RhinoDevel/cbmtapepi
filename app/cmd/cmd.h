
// Marcel Timm, RhinoDevel, 2019dec03

// Singleton (!) to interpret and react on commands send from Commodore via tape
// SAVE (filename).

#ifndef MT_CMD
#define MT_CMD

#include "cmd_output.h"

/**
 * - Caller takes ownership of returned object.
 */
struct cmd_output * cmd_create_output(char const * const command);

void cmd_reinit(char const * const start_dir_path);

/** Helper method to deallocate output object.
 *
 *  - Given pointer is no longer valid, after this.
 */
void cmd_free_output(struct cmd_output * const output);

#endif //MT_CMD
