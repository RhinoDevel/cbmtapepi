
// Marcel Timm, RhinoDevel, 2019dec05

#include "petasc.h"

#include <stdbool.h>
#include <stddef.h>

static char const s_not_found = 128;

/** Returns true, if given character's code and meaning are equal in PETSCII and
 *  ASCII.
 */
static bool is_equal(char const c)
{
    // 0 - 31 (different control characters or unused)

    if(32/*' '*/ <= c && c <= 64/*'@'*/)
    {
        return true;
    }

    // 65 - 90 (ASCII: 'A' to 'Z', PETSCII: Mode-dependent, either 'A' to 'Z'
    //          or 'a' to 'z').

    if(c == 91/*'['*/)
    {
        return true;
    }

    // 92 (ASCII: '\', PETSCII: Pound symbol).

    if(c == 93/*']'*/)
    {
        return true;
    }

    // 94 - 96 (ASCII: '^', '_' and '`', PETSCII: Up-arrow, left-arrow and a
    //          graphical symbol)

    // 97 - 122 (ASCII: 'a' to 'z', PETSCII: Mode-dependent, either graphical
    //           symbols or 'A' to 'Z').

    // 123 - 125 (ASCII: '{', '|', '}', PETSCII: Graphical symbols).

    // 126 (ASCII: '~', PETSCII: Mode-dependent, either Pi or a graphical
    //      symbol).

    // 127 (ASCII: Control code, PETSCII: Mode-dependent graphical symbol).

    return false;
}

static bool is_ascii_letter(char const c)
{
    if(97/*'a'*/ <= c && c <= 122/*'z'*/)
    {
        return true;
    }

    if(65/*'A'*/ <= c && c <= 90/*'Z'*/)
    {
        return true;
    }

    return false;
}

/**
 * - Only upper case letters in upper case mode and lower case letters in lower
 *   case mode are interpreted as PETSCII letters, here (not the upper case
 *   letters in lower case mode between 97 and 122).
 */
static bool is_petscii_letter(char const c)
{
    return 65/* A or a */ <= c && c <= 90/* z or Z */;
}

/**
 * - Works correctly for given ASCII letters from 'a' to 'z' or 'A' to 'Z',
 *   only.
 */
static char get_petscii_letter(char const c)
{
    if(c < 97/*'a'*/)
    {
        return c; // Assumed to be between 65 ('A') and 90 ('Z').
    }

    return c - (97/*'a'*/ - 65/*'A'*/);
    //
    // Assumed to be between 97 ('a') and 122 ('z').
}

/**
 * - Works correctly for given PETSCII letters from 65 to 90, only.
 * - Always returns lower case letters.
 */
static char get_ascii_letter(char const c)
{
    return c + (97/*'a'*/ - 65/*'A'*/);
}

static char get_petscii(char const c)
{
    if(is_equal(c))
    {
        return c;
    }
    if(is_ascii_letter(c))
    {
        return get_petscii_letter(c);
    }
    return s_not_found;
}

static char get_ascii(char const c)
{
    if(is_equal(c))
    {
        return c;
    }
    if(is_petscii_letter(c))
    {
        return get_ascii_letter(c);
    }
    return s_not_found;
}

char petasc_get_petscii(char const c, char const not_found_replacer)
{
    char petscii = get_petscii(c);

    if(petscii == s_not_found)
    {
        return not_found_replacer;
    }
    return petscii;
}

char petasc_get_ascii(char const c, char const not_found_replacer)
{
    char ascii = get_ascii(c);

    if(ascii == s_not_found)
    {
        return not_found_replacer;
    }
    return ascii;
}
