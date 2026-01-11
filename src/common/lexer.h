/**
 * @file lexer.h
 * @brief Header file for the lexer module.
 * @details This file contains the definition of the Lexer and token structures and function prototypes for tokenizing
 *         assembly language source code. The lexer is responsible for reading the (pre)processed source code line by line,
 *         and breaking it down into tokens, identifying their types, and storing them for further processing.
 */

#ifndef LEXER_H
#define LEXER_H

#include "./string_view.h"  
#include "../main/assembler.h"
#include "../data_structures/array_list.h"

typedef enum {
    TOKEN_INVALID,
    TOKEN_UNKNOWN,

    /* Single character tokens */

    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_COLON,
    TOKEN_AMPER,
    TOKEN_HASH,
    TOKEN_QUOTE,

    /* Keywords */
    TOKEN_INSTRUCTION,
    TOKEN_REGISTER,
    TOKEN_STR_LIT,
    TOKEN_IMM,
    TOKEN_IDENTIFIER,
    TOKEN_LABEL,
    TOKEN_DIRECTIVE,
    TOKEN_DIR_DATA,
    TOKEN_DIR_STRING,
    TOKEN_DIR_ENTRY,
    TOKEN_DIR_EXTERN,
    TOKEN_INSTRUCTION_LINE,
    TOKEN_DATA_LINE,
    TOKEN_STRING_LINE
} TokenType;

/* Token structure */
typedef struct {
    TokenType type;                     /* Type of the token */
    StringView sv;                      /* String view of the token */
    size_t line_number;                 /* Line number where the token was found */
} Token;

/* Lexer structure */
typedef struct {
    StringView current_line;            /* Current line being processed */
    size_t cursor;                      /* Current position in the line */
    size_t line_number;                 /* Current line number */
} Lexer;

/* Function prototypes */
/**
 * @brief Gets the string representation of the token type.
 * @param type The token type.
 * @return Pointer to the string representation of the token type.
 * @note The returned string is a static string and should not be freed.
 */
char *get_type_str(TokenType type);

/**
 * @brief Creates a new token.
 * @param sv The string view of the token.
 * @param line_number The line number where the token was found.
 * @return Pointer to the newly created token.
 * @note The caller is responsible for freeing the token using token_destroy().
 */
Token *token_create(StringView sv, size_t line_number);

/**
 * @brief Destroys a token and frees its memory.
 * @param token Pointer to the token to destroy.
 * @note Frees the token structure and its contents.
 */
void token_destroy(void *token);

/**
 * @brief Identifies the type of the token.
 * @param token Pointer to the token to identify.
 * @note This function sets the type of the token based on its string view.
 */
void token_identify(Token *token);

/**
 * @brief Checks if the current token is of type identifier.
 * @param sv The string view of the token.
 * @return 1 if the token is an identifier, 0 otherwise.
 * @note An identifier is a valid string that starts with a letter and contains only letters, digits, and underscores.
 */
int is_identifier(StringView sv);

/**
 * @brief Initializes the lexer.
 * @param lexer Pointer to the lexer to initialize.
 * @note This function sets the cursor and line number to 0.
 */
void lexer_init(Lexer *lexer);

/**
 * @brief Identifies the types of tokens that weren't identified as a single token by context of other tokens from that line.
 * @param tokens Pointer to the list of tokens to identify.
 * @param ctx Pointer to the assembler context.
 */
void identify_context(ArrayList *tokens, AssemblerContext *ctx);

/**
 * @brief Reads the next line from the preprocessed lines.
 * @param lexer Pointer to the lexer.
 * @param ctx Pointer to the assembler context.
 * @return 1 if the line was read successfully, 0 if there are no more lines.
 * @note This function updates the current line and line number in the lexer.
 */
int lexer_next_line(Lexer *lexer, AssemblerContext *ctx);

/**
 * @brief Tokenizes the current line and adds tokens to the list.
 * @param lexer Pointer to the lexer.
 * @param ctx Pointer to the assembler context.
 * @param tokens Pointer to the list of tokens to add to.
 */
void lexer_tokenize_line(Lexer *lexer , AssemblerContext *ctx, ArrayList *tokens);

#endif /* LEXER_H */