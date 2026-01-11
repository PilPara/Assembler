/**
 * @file error.c
 * @brief Implementation of error handling functions.
 * @details This file contains functions for creating, destroying, and reporting errors.
 *          It includes functions for printing error messages and reporting errors
 *          to an array list. The error handling system is designed to be flexible and
 *          extensible, allowing for easy addition of new error types and messages.
 */

#include "./error.h"
#include "./util.h"
#include "../data_structures/array_list.h"

#include <stdio.h>
#include <stdarg.h>

/* Error types string LUT */
static const char *error_type_str[] = {
    "None",
    "File Open",
    "File Read",
    "Line Len",
    "Macro Extra",
    "Macro Name",
    "Sapce Missing",
    "Invalid Token",
    "Invalid Statement",
    "Missing Space",
    "Empty Label",
    "Label Max Len",
    "Label Start Digit",
    "Label Invalid Char",
    "Label Instruction",
    "Label Register",
    "Label Directive",
    "Label Duplicate",
    "Label Missing Colon",
    "Label Missing Space",
    "Instruction Illegal Comma",
    "Instruction Illegal Number of Commas",
    "Address Out of Bounds",
    "Immediate Out of Bounds",
    "Immediate Invalid",
    "Data Invalid",
    "Directive Dot Missing",
    "Data Illegal Comma",
    "String Illegal Comma",
    "String Missing Quote",
    "Multiple Commas",
    "Syntax Number of Operands",
    "Syntax Addressing Mode",
    "Symbol Not Found"
};

Error *error_create(ErrorType type, const char *message, va_list args) 
{
    Error *error = NULL;

    if (!message) 
        return NULL;
    
    error = (Error *)MALLOC(sizeof(Error));
    
    if (!error) 
        return NULL;

    /* Format the message with the provided arguments */
    error->type = type;
    vsprintf(error->message, message, args);
    error->message[sizeof(error->message) - 1] = '\0'; 

    return error;
}

void error_destroy(void *error) 
{
    Error *err = (Error *)error;

    if (!error) 
        return;
    
    FREE(err);
}  


void error_print(void *error) 
{
    Error *err = (Error *)error;

    if (!error) 
        return;
    
    fprintf(stderr, "[%s] %s\n", error_type_str[err->type], err->message);
}

void error_report(ArrayList *errors, ErrorType type, const char *fmt, ...) 
{
    va_list args = {0};
    Error *error = NULL;

    if (!errors) 
        return;

    /* Create a new error */
    va_start(args, fmt);
    error = error_create(type, fmt, args);
    va_end(args);

    /* add to the list */
    array_list_append(errors, error);
}


void error_report_all(ArrayList *errors)
{
    size_t i;
    Error *error = NULL;

    if (!errors || array_list_size(errors) == 0) 
        return;

    fprintf(stderr, "================================================ ERROR REPORT =================================================\n");
    for (i = 0; i < array_list_size(errors); i++) {
        error = (Error *)array_list_get(errors, i);
        if (error) 
            error_print(error);
    }
    fprintf(stderr, "===============================================================================================================\n");

    array_list_clear(errors);
}
