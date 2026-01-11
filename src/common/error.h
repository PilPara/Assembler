/**
 *  @file error.h
 *  @brief Header file for error handling in the assembler. 
 *  @details This file contains the definition of the Error structure, error types, 
 * *          and function prototypes for creating, destroying, and reporting errors.
 *  @note The error handling system is designed to be flexible and extensible, allowing
 *        for easy addition of new error types and messages.
 */

#ifndef ERROR_H
#define ERROR_H

#include <stddef.h>
#include <stdarg.h>

#include "../data_structures/array_list.h"

#define ERR_MSG_MAX_LEN 256

/* Error types */

typedef enum {
    ERR_NONE,

    /* File errors */
    ERR_FILE_OPEN,
    ERR_FILE_READ,

    /* Preprocessor errors */
    ERR_MAX_LINE_LEN,

    /* Macro errors */
    ERR_MCRO_DEF_EXTRA,
    ERR_MCRO_NAME,
    ERR_MCRO_SPACE_MISSING,
    
    /* Lexer errors */
    ERR_INVALID_TOKEN,
    ERR_INVALID_STATEMENT,

    /* Parser errors */
    ERR_MISSING_SPACE,

    /* Label errors */
    ERR_EMPTY_LABEL,
    ERR_LABEL_MAX_LEN,
    ERR_LABEL_NAME_DIG,
    ERR_LABEL_NAME_INVALID_CHAR,
    ERR_LABEL_NAME_INSTRUCTION,
    ERR_LABEL_NAME_REGISTER,
    ERR_LABEL_NAME_DIRECTIVE,
    ERR_LABEL_NAME_DUPLICATE,
    ERR_LABEL_MISSING_COLON,
    ERR_LABEL_MISSING_SPACE,

    /* Instruction errors */
    ERR_INST_ILLEGAL_COMMA,
    ERR_INST_ILLEGAL_NUM_COMMA,

    ERR_ADD_OUT_OF_BOUNDS,
    ERR_IMM_OUT_OF_BOUNDS,
    ERR_INVALID_IMM,
    ERR_INVALID_DATA,

    /* Directive errors */
    ERR_DIR_DOT_MISSING,
    ERR_DIR_DATA_ILLEGAL_COMMA,
    ERR_DIR_STR_ILLEGAL_COMMA,
    ERR_DIR_STR_MISSING_QUOTE,
    ERR_DIR_MULTY_COMMAS,

    /* General Instruction errors */
    ERR_SYNTAX_NUM_OPERANDS,
    ERR_SYNTAX_ADD_MOD,

    ERR_SYMBOL_NOT_FOUND
} ErrorType;


/* Error structure */
typedef struct {
    ErrorType type;
    char message[ERR_MSG_MAX_LEN];
} Error;

/* Function prototypes */
/**
 * @brief Creates a new error.
 * @param type The type of the error.
 * @param message The error message format string.
 * @param args The variable argument list for the error message.
 * @return Pointer to the newly created error.
 * @note The caller is responsible for freeing the error using error_destroy().
 */
Error *error_create(ErrorType type, const char *message, va_list args);

/**
 * @brief Destroys an error and frees its memory.
 * @param error Pointer to the error to destroy.
 * @note Frees the error structure and its contents.
 */
void error_destroy(void *error);

/**
 * @brief Prints the error message to stderr.
 * @param error Pointer to the error to print.
 */
void error_print(void *error);

/**
 * @brief Reports an error and adds it to the list of errors.
 * @param errors Pointer to the list of errors.
 * @param type The type of the error.
 * @param fmt The format string for the error message.
 * @param ... The variable arguments for the error message.
 * @note The error is created and added to the list of errors.array_list
 *       This function does not print the error message.
 */
void error_report(ArrayList *errors, ErrorType type, const char *fmt, ...);

/**
 * @brief Prints all errors in the list and clears the list.
 * @param errors Pointer to the list of errors.
 * @note This function prints all errors in the list and frees them.
 */
void error_report_all();

#endif /* ERROR_H */