/**
 * @file string_view.c
 * @brief Implementation of string view operations.
 * @details This file contains the implementation of functions for creating,
 *          manipulating, and comparing string views, checking prefixes and
 *         suffixes, and trimming whitespace from string views. 
 */

#include "./string_view.h"  
#include <stdio.h>
#include <string.h>

StringView sv_from_str(char *str) 
{
    StringView sv = {0};

    if (!str) 
        return sv;

    sv.str = str;
    sv.length = strlen(str);

    return sv;
}

StringView sv_from_parts(char *str, size_t length) 
{
    StringView sv = {0};

    if (!str || length == 0) 
        return sv;

    sv.str = str;
    sv.length = length;

    return sv;
}

StringView sv_trim_left(StringView sv) 
{
    size_t i = 0;
    StringView result;

    while (i < sv.length && (sv.str[i] == ' ' || sv.str[i] == '\t')) 
        i++;

    result.str = sv.str + i;
    result.length = sv.length - i;

    return result;
}

StringView sv_trim_right(StringView sv) 
{
    size_t i = 0;
    StringView result;

    while (i < sv.length && (sv.str[sv.length - 1 - i] == ' ' || sv.str[sv.length - 1 - i] == '\t')) 
        i++;

    result.str = sv.str;
    result.length = sv.length - i;

    return result;
}

StringView sv_trim(StringView sv) 
{
    return sv_trim_right(sv_trim_left(sv));
}


int sv_eq(StringView a, StringView b) 
{
    if (a.length != b.length) 
        return 0;

    return memcmp(a.str, b.str, a.length) == 0;
}
/**
 * * @brief Compares a StringView with a string.
 * * @param sv The StringView to compare.
 * * @param str The string to compare with.
 * * @return 1 if they are equal, 0 otherwise.
 */

int sv_eq_str(StringView sv, char *str) 
{
    size_t len;
    
    if (!str)
        return 0;
        
    len = strlen(str);

    if (len != sv.length) 
        return 0;
        
    return memcmp(sv.str, str, len) == 0;
}

int sv_starts_with(StringView sv, char *str) 
{
    size_t len = 0;

    if (!str) 
        return 0;

    len = strlen(str);

    if (len > sv.length) 
        return 0;

    return memcmp(sv.str, str, len) == 0;
}

int sv_ends_with(StringView sv, char *str) 
{
    size_t len = strlen(str);
    StringView suffix = {0};

    if (len > sv.length) 
        return 0;

    suffix.str = sv.str + sv.length - len;
    suffix.length = len;

    return sv_eq_str(suffix, str);
}
