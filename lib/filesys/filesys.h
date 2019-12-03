
// Marcel Timm, RhinoDevel, 2019dec03

// Singleton for SINGLE file system mount and unmount.

#ifndef MT_FILESYS
#define MT_FILESYS

#include <stdbool.h>

/**
 * - OK to be called, if already unmounted (returns true).
 */
bool filesys_unmount();

bool filesys_remount();

#endif //MT_FILESYS
