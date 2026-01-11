/**
 * @file util.h
 * @brief Header file for utility functions - mainly memory management.
 * @details This file contains function prototypes for memory management,
 *          string duplication, and normalization. It also includes macros for easy
 *          memory management with error checking and reporting.
 */

#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>
 
/* Macros for easy memory management */
#define MALLOC(size) xmalloc(size, __FILE__, __LINE__)  
#define REALLOC(ptr, size) xrealloc(ptr, size, __FILE__, __LINE__)
#define FREE(ptr) xfree((void **)&ptr, __FILE__, __LINE__)
#define STRDUP(s) xstrdup(s, __FILE__, __LINE__)
#define STRDUP_NORM(s) xstrdup_norm(s, __FILE__, __LINE__)

/**
 * @brief Allocates memory and checks for allocation failure.
 * @param size Size of memory to allocate.
 * @param file File where the function is called.
 * @param line Line number where the function is called.
 * @return Pointer to the allocated memory.
 * @note The caller is responsible for freeing the memory using FREE().
 */

void *xmalloc(size_t size, const char *file, int line); 

/**
 * @brief Reallocates memory and checks for allocation failure.
 * @param ptr Pointer to the memory to reallocate.
 * @param size New size of memory to allocate.
 * @param file File where the function is called.
 * @param line Line number where the function is called.
 * @return Pointer to the reallocated memory.
 * @note The caller is responsible for freeing the memory using FREE().
 */
void *xrealloc(void *ptr, size_t size, const char *file, int line);

/**
 * @brief Frees memory and sets the pointer to NULL.    
 * @param ptr Pointer to the memory to free.
 * @param file File where the function is called.
 * @param line Line number where the function is called.
 */
void xfree(void **ptr, const char *file, int line);

/**
 * @brief Frees a dynamically allocated string.
 * @param str Pointer to the string to free.
 * @note This is not a stand alone function in is only meant to be used with the array_list_create() function.
 */

void free_string(void *str);

/**
 * @brief Duplicates a string and checks for allocation failure.
 * @param s The string to duplicate.
 * @param file File where the function is called.
 * @param line Line number where the function is called.
 * @return Pointer to the duplicated string.
 */
char *xstrdup(const char *s, const char *file, int line);

/**
 * @brief Normalizes a string by replacing sequences of whitespace with a single space.
 * @param s The string to normalize.
 * @param file File where the function is called.
 * @param line Line number where the function is called.
 * @return A new string with normalized whitespace.
 *        The caller is responsible for freeing the returned string.
 */
char *xstrdup_norm(const char *s, const char *file, int line);

#endif /* UTIL_H */