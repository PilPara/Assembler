/**
 * @file first_pass.h
 * @brief Header file for the first pass of the assembler.
 * @details This file contains function prototypes and definitions for the first pass of the assembler,
 *          which includes parsing, building the symbol table, calculating addresses, and error checking.
 *          The first pass processes the source code, identifies labels, and prepares the symbol table
 *          for the second pass, where the symbol table will be resolved.
 *          If errors are found during the first pass, they are reported and the assembly process is halted.
 */

#ifndef FIRST_PASS_H
#define FIRST_PASS_H

#include "../main/assembler.h"
#include "../common/string_view.h"

#define MAX_LABEL_LEN 31


/* Symbol structure */
/* This structure represents a symbol in the assembler and is used as an entry in the symbol table. */

typedef struct {
    StringView sv;                  /* The symbol name */
    size_t address;                 /* The address of the symbol in memory */
    int external;                   /* Flag indicating if the symbol is external */
    int entry;                      /* Flag indicating if the symbol is defined with .entry */
} Symbol;

/**
 * @brief Creates a new symbol.
 * @param sv The symbol name as a StringView.
 * @param address The address of the symbol.
 * @param external Flag indicating if the symbol is external.
 * @param entry Flag indicating if the symbol is defined with .entry.
 */
Symbol *symbol_create(StringView sv, size_t address, int external, int entry);

/**
 * @brief Destroys a symbol and frees its memory.
 * @param symbol Pointer to the symbol to destroy.
 * @note Frees the symbol structure and its contents.
 */
void symbol_destroy(void *symbol);

/**
 * @brief Copies a symbol.
 * @param symbol Pointer to the symbol to copy.
 * @return Pointer to the copied symbol.
 * @note The caller is responsible for freeing the copied symbol using symbol_destroy().
 */
Symbol *symbol_copy(void *symbol);

/**
 * @brief Preforms the first pass of the assembler - Parsing, Building the symbol table, calculating addresses and error checking.
 * @param ctx Pointer to the assembler context.
 */
void first_pass(AssemblerContext *ctx);

/**
 * @brief Validates a label name.
 * @param sv The label name as a StringView.
 * @param ctx Pointer to the assembler context.
 * @return 1 if the label is valid, 0 otherwise.
 */
int validate_label(StringView sv, AssemblerContext *ctx);

/**
 * @brief Validates an address.
 * @param address The address to validate.
 * @param ctx Pointer to the assembler context.
 */
void validate_address(size_t address, AssemblerContext *ctx);

/**
 * @brief Defines a symbol in the symbol table.
 * @param ctx Pointer to the assembler context.
 * @param tokens The list of tokens to process.
 */
void define_symbol(AssemblerContext *ctx, ArrayList *tokens);

#endif /* FIRST_PASS_H */