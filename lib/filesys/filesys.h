
// Marcel Timm, RhinoDevel, 2019dec03

// Singleton for SINGLE file system mount and unmount.

#ifndef MT_FILESYS
#define MT_FILESYS

#include <stdbool.h>
#include <stdint.h>

/**
 * - OK to be called, if already unmounted (returns true).
 * - Does absolutely nothing, but returning NULL, in Linux port.
 */
bool filesys_unmount();

/**
 * - Does absolutely nothing, but returning NULL, in Linux port.
 */
bool filesys_remount();

/** Load full file content and set byte count.
 * 
 *  - Supports empty files.
 *  - Caller takes ownership of returned object.
 *  - Returns 0 on error.
 */
uint8_t* filesys_load(
    char const * const dir_path,
    char const * const filename,
    uint32_t * const out_byte_count);

/** Saves given count of bytes starting at given pointer at given directory with
 *  given file name.
 * 
 *  - Supports empty files.
 *  - Returns, if successful or not.
 */
bool filesys_save(
    char const * const dir_path,
    char const * const filename,
    uint8_t const * const bytes,
    uint32_t const byte_count,
    bool const overwrite);

/** Remove file.
 * 
 *  - Returns, if successful or not.
 *  - No support for deletion of empty subfolders.
 */
bool filesys_remove(char const * const dir_path, char const * const filename);

#endif //MT_FILESYS
