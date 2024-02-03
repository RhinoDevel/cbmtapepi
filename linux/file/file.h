
// Marcel Timm, RhinoDevel, 2024feb03

#ifndef MT_FILE
#define MT_FILE

#define MT_FILE_PATH_SEP '/'

#include <sys/types.h>

/** Return content of file at given path.
 *
 *  - Returns NULL on error.
 *  - Caller takes ownership of return value.
 */
unsigned char * file_load(char const * const path, off_t * const out_size);

void file_init(int const max_size);

#endif //MT_FILE
