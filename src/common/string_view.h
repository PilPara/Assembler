/**
 * @file string_view.h
 * @brief Header file for string view operations.
 * @details This file contains the definition of the StringView structure and function prototypes for creating,
 *          manipulating, and comparing string views. A string view is a lightweight representation of a string
 *          which is just a reference to a part of a string and its length.
 */

#ifndef STRING_VIEW_H
#define STRING_VIEW_H

#include <stdio.h>

/* StringView structure */

typedef struct {
    char *str;
    size_t length;
} StringView;

/* Function prototypes */
/**
 * @brief Creates a StringView from a string.
 * @param str The string to create the StringView from.
 * @return A StringView structure containing the string and its length.
 * @note The string is not copied, so it should remain valid for the lifetime of the StringView.
 */
StringView sv_from_str(char *str);

/**
 * @brief Creates a StringView from a part of a string.
 * @param str The string to create the StringView from.
 * @param length The length of the part to create the StringView from.
 * @return A StringView structure containing the string and its length.
 * @note The string is not copied, so it should remain valid for the lifetime of the StringView.
 */
StringView sv_from_parts(char *str, size_t length);
/**
 * @brief Trims leading whitespace from a StringView.
 * @param sv The StringView to trim.
 * @return A new StringView with leading whitespace removed.
 * @note The original StringView is not modified.
 */
StringView sv_trim_left(StringView sv);

/**
 * @brief Trims trailing whitespace from a StringView.
 * @param sv The StringView to trim.
 * @return A new StringView with trailing whitespace removed.
 * @note The original StringView is not modified.
 */
StringView sv_trim_right(StringView sv);
/**
 * @brief Trims leading and trailing whitespace from a StringView.
 * @param sv The StringView to trim.
 * @return A new StringView with leading and trailing whitespace removed.
 * @note The original StringView is not modified.
 */
StringView sv_trim(StringView sv);

/**
 * @brief Compares a StringView with a string.
 * @param sv The StringView to compare.
 * @param str The string to compare with.
 * @return 1 if they are equal, 0 otherwise.
 * @note The comparison is case-sensitive.
 */
int sv_eq_str(StringView sv, char *str);

/**
 * @brief Checks if a StringView have str as a prefix.
 * @param sv The StringView to check.
 * @param str The string to check for as a prefix.
 * @return 1 if sv starts with str, 0 otherwise.
 */
int sv_starts_with(StringView sv, char *str);

/**
 * @brief Checks if a StringView have str as a suffix.
 * @param sv The StringView to check.
 * @param str The string to check for as a suffix.
 * @return 1 if sv ends with str, 0 otherwise.
 */
int sv_ends_with(StringView sv, char *str);

#endif /* STRING_VIEW_H */