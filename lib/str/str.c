
// Marcel Timm, RhinoDevel, 2019jan05

#include "str.h"
#include "../alloc/alloc.h"

#include <stdint.h>
#include <stdbool.h>

static bool is_upper_case_letter(char const c)
{
    return c >= 'A' && c <= 'Z';
}
static bool is_lower_case_letter(char const c)
{
    return c >= 'a' && c <= 'z';
}
static char get_upper_case(char const c)
{
    static char const diff = 'a' - 'A';

    if(is_lower_case_letter(c))
    {
        return c - diff;
    }
    return '?';
}
static char get_lower_case(char const c)
{
    static char const diff = 'a' - 'A';

    if(is_upper_case_letter(c))
    {
        return c + diff;
    }
    return '?';
}

uint32_t str_get_len(char const * const s)
{
    uint32_t ret_val = 0;

    while(s[ret_val] != '\0')
    {
        ++ret_val;
    }
    return ret_val;
}

uint32_t str_get_len_sum(
    char const * const * const s_arr, uint32_t const s_count)
{
    uint32_t ret_val = 0;

    for(uint32_t i = 0;i < s_count;++i)
    {
        ret_val += str_get_len(s_arr[i]);
    }
    return ret_val;
}

uint32_t str_get_len_max(
    char const * const * const s_arr, uint32_t const s_count)
{
    uint32_t ret_val = 0;

    for(uint32_t i = 0;i < s_count;++i)
    {
        uint32_t const len = str_get_len(s_arr[i]);

        if(len > ret_val)
        {
            ret_val = len;
        }
    }
    return ret_val;
}

void str_to_upper(char * const s_out, char const * const s_in)
{
    int i = 0;

    for(;s_in[i] != '\0';++i)
    {
        if(is_lower_case_letter(s_in[i]))
        {
            s_out[i] = get_upper_case(s_in[i]);
            continue;
        }
        s_out[i] = s_in[i];
    }
    s_out[i] = '\0';//s_in[i];
}

void str_to_lower(char * const s_out, char const * const s_in)
{
    int i = 0;

    for(;s_in[i] != '\0';++i)
    {
        if(is_upper_case_letter(s_in[i]))
        {
            s_out[i] = get_lower_case(s_in[i]);
            continue;
        }
        s_out[i] = s_in[i];
    }
    s_out[i] = '\0';//s_in[i];
}

void str_copy(char * const s_out, char const * const s_in)
{
    int i = 0;

    for(;s_in[i] != '\0';++i)
    {
        s_out[i] = s_in[i];
    }
    s_out[i] = '\0';//s_in[i];
}

char* str_create_copy(char const * const s)
{
    uint32_t const len = str_get_len(s);
    char * const ret_val = alloc_alloc((len + 1) * sizeof *ret_val);

    str_copy(ret_val, s);

    return ret_val;
}

bool str_starts_with(char const * const s, char const * const start)
{
    uint32_t const start_len = str_get_len(start);

    if(start_len == 0)
    {
        return s[0] == '\0';
    }

    for(uint32_t i = 0;i < start_len;++i)
    {
        if(s[i] != start[i])
        {
            return false;
        }
    }
    return true;
}

int str_get_index_of_trailing(char const * const s, char const c)
{
    int ret_val = -1,
        i = 0;

    for(;s[i] != '\0';++i)
    {
        if(s[i] != c)
        {
            ret_val = -1;
            continue;
        }
        if(ret_val > -1)
        {
            continue;
        }
        ret_val = i;
    }
    return ret_val;
}

char* str_create_concat(char const * const first, char const * const last)
{
    uint32_t const len_first = str_get_len(first);

    if(len_first == 0)
    {
        return str_create_copy(last);
    }

    uint32_t const len_last = str_get_len(last);

    if(len_last == 0)
    {
        return str_create_copy(first);
    }

    char* const s = alloc_alloc((len_first + len_last + 1) * sizeof *s);

    str_copy(s, first);
    str_copy(s + len_first, last);

    return s;
}
