/**
 * @file util.c
 * @brief Implementation of utility functions for memory management.
 * @details This file contains functions for memory allocation, reallocation,
 *          freeing memory, and string duplication. It also includes functions
 *          for normalizing strings and freeing dynamically allocated strings.
 *          all functions in this file are used only through macros defined in util.h.
 */

#include "./util.h"
#include "../main/assembler.h"

#include <stdio.h>
#include <stdlib.h>

void *xmalloc(size_t size, const char *file, int line) 
{
    void *ptr = NULL;

    ptr = malloc(size);

    if (!ptr) 
    {
        fprintf(stderr, "Memory allocation failed at %s:%d\n", file, line);
        return NULL;
    }

    return ptr;
}

void *xrealloc(void *ptr, size_t size, const char *file, int line) 
{
    void *new_ptr = NULL;

    new_ptr = realloc(ptr, size);

    if (!new_ptr) 
    {
        fprintf(stderr, "Memory allocation failed at %s:%d\n", file, line);
        return NULL;
    }

    return new_ptr;
}


void xfree(void **ptr, const char *file, int line) 
{
    if (ptr && *ptr) 
    {
        free(*ptr);
        *ptr = NULL;
    }
}

void free_string(void *str) 
{
    if (!str)
        return;

    FREE(str);
}

char *xstrdup(const char *str, const char *file, int line) 
{
    char *dup = NULL;
    size_t len = 0, i;

    if (!str)
        return NULL;

    while (str[len])
        len++;

    dup = (char *)xmalloc(len + 1, file, line); 

    if (!dup) 
    {
        fprintf(stderr, "Memory allocation failed at %s:%d\n", file, line);
        return NULL;
    }

    /* Copy the string */
    for (i = 0; i < len; i++) 
        dup[i] = str[i];

    dup[len] = '\0';

    return dup;
}

char *xstrdup_norm(const char *str, const char *file, int line)
{
    char *norm_str = NULL;
    size_t len = 0, i = 0, j = 0;

    if (!str)
        return NULL;

    while (str[len])
        len++;

    norm_str = (char *)xmalloc(len + 1, file, line); 

    if (!norm_str) 
    {
        fprintf(stderr, "Memory allocation failed at %s:%d\n", file, line);
        return NULL;
    }

    while (i < len) 
    {
        /* Normalize spaces */
        if (str[i] == ' ' || str[i] == '\t') 
        {
            if (j > 0 && norm_str[j - 1] != ' ')
                norm_str[j++] = ' ';
        } 

        /* Copy other characters */
        else 
            norm_str[j++] = str[i];
        
        i++;
    }

    /* Null-terminate the string */
    norm_str[j] = '\0';

    return norm_str;
}