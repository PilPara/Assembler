
/**
 * @file second_pass.h
 * @brief Header file for the second pass of the assembler.
 * @details This file contains function prototypes and definitions for the second pass of the assembler,
 *          which includes translating instructions and directives into machine code,
 *          resolving symbols, and generating output files (.ob, .ent, .ext) (if assembly is successful).
 */
#ifndef SECOND_PASS_H
#define SECOND_PASS_H

#include "../main/assembler.h"
#include "../common/code_gen.h"
/**
 * @brief Gets the tokens of the current line from the assembler context.
 * @param ctx Pointer to the assembler context.
 * @param tokens Pointer to the array list to store the tokens.
 */
void get_line(AssemblerContext *ctx, ArrayList *tokens);

/**
 * @brief Checks if a token is an identifier referenced as extern or entry, and if so stores the reference in the appropriate list.
 * @param token Pointer to the token to check.
 * @param ctx Pointer to the assembler context.
 * @param address The address of the token.
 * @param is_extern Flag indicating if the token is an extern reference.
 */
void process_token(Token *token, AssemblerContext *ctx, int address, int is_extern);

/**
 * @brief Logs a symbol to the appropriate list based on its type (extern or entry).
 * @param instruction Pointer to the parsed instruction.
 * @param ctx Pointer to the assembler context.
 * @param IC The instruction counter.
 * @param is_externs Flag indicating if the symbol is an extern reference.
 * @param is_entries Flag indicating if the symbol is an entry reference.
 */
void log_symbol(ParsedInstruction *instruction, AssemblerContext *ctx, int IC, int is_externs, int is_entries);

/**
 * @brief Preforms the second pass of the assembler - Translating instructions and directives into machine code.
 * @param ctx Pointer to the assembler context.
 * @note If assembly is successful, generates the output files (.ob, .ent, .ext).
 */

void second_pass(AssemblerContext *ctx);

#endif /* SECOND_PASS_H */