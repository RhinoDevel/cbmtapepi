
// Marcel Timm, RhinoDevel, 2019dec05

// Convert between PETSCII and ASCII.

#ifndef MT_PETASC
#define MT_PETASC

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * - Upper case ASCII letters given are interpreted as lower case.
 * - No support for control codes of any kind (CR, LF, etc.).
 */
char petasc_get_petscii(char const c, char const not_found_replacer);

/**
 * - Assumes PETSCII upper-case-letters-only mode.
 * - No support for control codes of any kind (CR, LF, etc.).
 */
char petasc_get_ascii(char const c, char const not_found_replacer);

#ifdef __cplusplus
}
#endif

#endif //MT_PETASC
