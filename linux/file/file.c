
// Marcel Timm, RhinoDevel, 2024feb03

#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>

#include "file.h"
#include "../../lib/console/console.h"

static int s_max_size = -1; // See file_init().

static off_t get_size(char const * const path)
{
    assert(path != NULL);

    struct stat s;

    if(stat(path, &s) == 0)
    {
        return s.st_size;
    }
    return -1;
}

/** Return content of file at given path.
 *
 *  - Returns NULL on error.
 *  - Caller takes ownership of return value.
 */
unsigned char * file_load(char const * const path, off_t * const out_size)
{
    *out_size = -1;

    off_t const signed_size = get_size(path);

    if(signed_size == -1)
    {
        console_writeline("file_load : Error: Failed to get size of file!");
        return NULL;
    }
    if(signed_size > (off_t)s_max_size)
    {
        console_writeline("file_load : Error: File is too big!");
        return NULL;
    }

    FILE * const file = fopen(path, "rb");

    if(file == NULL)
    {
        console_writeline("file_load : Error: Failed to open source file!");
        return NULL;
    }

    size_t const size = (size_t)signed_size;
    unsigned char * const buf = alloc_alloc(size * sizeof *buf);

    if(fread(buf, sizeof(*buf), size, file) != size)
    {
        console_writeline(
            "file_load : Error: Failed to completely load file content!");
        return NULL;
    }

    fclose(file);
    *out_size = signed_size;
    return buf;
}

void file_init(int const max_size)
{
    assert(0 < max_size);

    s_max_size = max_size;
}
