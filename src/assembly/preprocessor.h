/**
 * @file preprocessor.h
 * @brief Header file for the preprocessor module.
 * @details This file contains the definition of the Preprocessor structure and function prototypes for
 *         initializing, destroying, and using the preprocessor. The preprocessor handles macro definitions,
 *         expansions, and removes comments and empty lines from the source code.
 *         The preprocessor is responsible for detecting all macro related errors (+ a few others).
 */

#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include "../main/assembler.h"
#include "../common/string_view.h"

#define MAX_LINE_LEN 81
#define MAX_MCRO_NAME_LEN 31
#define MACRO_DEF "mcro"
#define MACRO_END "mcroend"

#define SKIP_WHITESPACE(str)                                    \
    while (*str == ' ' || *str == '\t')                         \
        str++;

#define DEF_LINE(pp)                                            \
    pp->line_number - array_list_size(pp->current_macro) - 1

/* Preprocessor state */
typedef enum {
    STATE_DEFAULT,
    STATE_MACRO
} PreprocessorState;

typedef struct {
    ArrayList *raw_lines;                   /* Array of raw lines as read from the input file */
    StringView current_line;                /* Current line being processed */
    size_t line_number;                     /* Current line number in the raw lines */
    PreprocessorState state;                /* Current state of the preprocessor */
    ArrayList *current_macro;               /* Array of lines in the current macro */
    HashMap *macros;                        /* Macro table */
} Preprocessor;

/**
 * @brief Initializes the preprocessor.
 * @param pp Pointer to the Preprocessor structure to initialize.
 * @note This function allocates memory for the preprocessor and initializes its fields.
 * @note The caller is responsible for freeing the preprocessor using preprocessor_destroy().
 */
void preprocessor_init(Preprocessor *pp);

/**
 * @brief Destroys the preprocessor and frees allocated memory.
 * @param pp Pointer to the Preprocessor structure to destroy.
 * @note This function frees all the resources associated with the preprocessor.
 */
void preprocessor_destroy(Preprocessor *pp);

/**
 * @brief Reads the next line from the raw lines and sets it as the current line, updating the line number.
 * @param pp Pointer to the Preprocessor structure.
 * @param ctx Pointer to the AssemblerContext structure.
 * @return 1 if a line was read successfully, 0 if EOF is reached.
 */
int next_line(Preprocessor *pp, AssemblerContext *ctx);

/**
 * @brief Checks if the current line is a comment i.e starts with a semicolon.
 * @param sv The current line as a StringView.
 * @return 1 if the line is a comment, 0 otherwise.
 */
int is_comment(StringView sv);

/**
 * @brief Checks if the current line is empty (contains only whitespace).
 * @param sv The current line as a StringView.
 * @return 1 if the line is empty, 0 otherwise.
 */
int is_empty_line(StringView sv);

/**
 * @brief Checks if the current line is a macro definition i.e starts with the 'mcro' keyword.
 * @param sv The current line as a StringView.
 * @return 1 if the line is a macro definition, 0 otherwise.
 */
int is_macro_def(StringView sv);

/**
 * @brief Checks if the current line is a macro end i.e starts with the 'mcroend' keyword.
 * @param sv The current line as a StringView.
 * @return 1 if the line is a macro end, 0 otherwise.
 */
int is_macro_end(StringView sv);

/**
 * @brief Checks if the current line is a macro call i.e if the macro name exists in the macro table.
 * @param pp Pointer to the Preprocessor structure.
 * @param macro_name The name of the macro to check.
 * @return 1 if the line is a macro call, 0 otherwise.
 */
int is_macro_call(Preprocessor *pp, const char *macro_name);

/**
 * @brief Validates the macro definition and checks for errors.
 * @param pp Pointer to the Preprocessor structure.
 * @param ctx Pointer to the AssemblerContext structure.
 * @param macro_def The macro definition line.
 * @param macro_end The macro end line.
 * @param macro_name_out Pointer to store the validated macro name.
 * @return 1 if the macro definition is valid, 0 otherwise.
 */
int validate_macro(Preprocessor *pp, AssemblerContext *ctx, char *macro_def, char *macro_end, char **macro_name_out);

/**
 * @brief Defines a macro by storing its body in the macro table.
 * @param pp Pointer to the Preprocessor structure. 
 * @param ctx Pointer to the AssemblerContext structure.
 * @note This function extracts the macro name and body from the current macro and stores it in the macro table.
 * @note The macro name is validated and the macro body is stored in the hash map.
 */
void define_macro(Preprocessor *pp, AssemblerContext *ctx);

/**
 * @brief Expands a macro by replacing its name with its body in the preprocessed lines.
 * @param pp Pointer to the Preprocessor structure.
 * @param ctx Pointer to the AssemblerContext structure.
 * @param macro_name The name of the macro to expand.
 * @note This function retrieves the macro body from the macro table and replaces the macro name with its body in the preprocessed lines.
 */
void expand_macro(Preprocessor *pp, AssemblerContext *ctx, const char *macro_name);

/**
 * @brief Preprocesses the source file by removing empty lines, comments, and expanding macros.
 * @param ctx Pointer to the AssemblerContext structure.
 * @note This functions updates the preprocessed lines field of the context with the processed lines so that they can be used in the first pass.
 */

void preprocess(AssemblerContext *ctx);

#endif /* PREPROCESSOR_H */