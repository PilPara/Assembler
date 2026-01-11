/**
 * @file first_pass.c
 * @brief Implementation of the first pass of the assembler.
 * @details This file contains functions for creating, destroying, and validating symbols,
 *          as well as functions for validating labels, defining symbols, and calculating addresses.
 *          The first pass processes the source code, identifies labels, and prepares the symbol table
 *          for the second pass, where the symbol table will be resolved. It also tracks Instruction and Data Counters (IC and DC).
 */

#include "./first_pass.h"
#include "../common/lexer.h"
#include "../common/error.h"
#include "../common/isa.h"
#include "../common/parser.h"
#include "../common/util.h"
#include "../common/string_view.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


Symbol *symbol_create(StringView sv, size_t address, int external, int entry)
{
    Symbol *symbol = NULL;

    if (!sv.str)
        return NULL;

    symbol = (Symbol *)MALLOC(sizeof(Symbol));
    if (!symbol)
        return NULL;

    symbol->sv = sv;
    symbol->address = address;
    symbol->external = external;
    symbol->entry = entry;

    return symbol;
}

void symbol_destroy(void *symbol)
{
    Symbol *sym = (Symbol *)symbol;

    if (!symbol)
        return;

    FREE(sym);
}

Symbol *symbol_copy(void *symbol)
{
    Symbol *sym = (Symbol *)symbol;
    Symbol *copy = NULL;

    if (!symbol)
        return NULL;

    copy = symbol_create(sym->sv, sym->address, sym->external, sym->entry);
    if (!copy)
        return NULL;

    return copy;
}

int validate_label(StringView sv, AssemblerContext *ctx)
{
    size_t i;
    char *label_str = NULL;
    Symbol *existing_symbol = NULL;
    
    /* Check if label is empty */
    if (sv.length == 0) 
    {
        error_report(ctx->errors, ERR_EMPTY_LABEL,
                     "%s:%d: Empty label name", ctx->ir_filename, ctx->line_number);
        return 0;
    }
    
    /* Check if label exceeds maximum length (31 characters) */
    if (sv.length > MAX_LABEL_LEN) 
    {
        error_report(ctx->errors, ERR_LABEL_MAX_LEN, 
                     "%s:%d: Label name '%.*s' exceeds maximum length of 31 characters", 
                     ctx->ir_filename, ctx->line_number, (int)sv.length, sv.str);
        return 0;
    }
    
    /* Check if first character is a letter */
    if (!isalpha(sv.str[0])) 
    {
        error_report(ctx->errors, ERR_LABEL_NAME_DIG, 
                     "%s:%d: Label name '%.*s' must start with a letter", 
                     ctx->ir_filename, ctx->line_number, (int)sv.length, sv.str);
        return 0;
    }
    
    /* Check if remaining characters are alphanumeric or underscore */
    for (i = 1; i < sv.length; i++) 
    {
        if (!isalnum(sv.str[i]) && sv.str[i] != '_') 
        {
            error_report(ctx->errors, ERR_LABEL_NAME_INVALID_CHAR, 
                         "%s:%d: Label name '%.*s' contains invalid character '%c'", 
                         ctx->ir_filename, ctx->line_number, (int)sv.length, sv.str, sv.str[i]);
            return 0;
        }
    }
    
    /* Create a null-terminated copy of the label for comparison */
    label_str = (char *)MALLOC(sv.length + 1);
    if (!label_str)
        return 0;
    
    /* Copy the label string */
    memcpy(label_str, sv.str, sv.length);
    label_str[sv.length] = '\0';
    
    /* Check if label is a reserved word */
    if (find_instruction(label_str) != NULL) 
    {
        error_report(ctx->errors, ERR_LABEL_NAME_INSTRUCTION, 
                     "%s:%d: Label name '%s' cannot be an instruction name", 
                     ctx->ir_filename, ctx->line_number, label_str);
        FREE(label_str);
        return 0;
    }

    /* Check if label is a register name */
    if (find_register(label_str) != NULL) 
    {
        error_report(ctx->errors, ERR_LABEL_NAME_REGISTER, 
                     "%s:%d: Label name '%s' cannot be a register name", 
                     ctx->ir_filename, ctx->line_number, label_str);
        FREE(label_str);
        return 0;
    }
    
    /* Check if label is a directive name */
    if (is_directive(sv_from_str(label_str))) 
    {
        error_report(ctx->errors, ERR_LABEL_NAME_DIRECTIVE, 
                     "%s:%d: Label name '%s' cannot be a directive name", 
                     ctx->ir_filename, ctx->line_number, label_str);
        FREE(label_str);
        return 0;
    }
    
    /* Check if the label is already defined (duplicate) */
    existing_symbol = hash_map_get(ctx->symbol_table, label_str);
    if (existing_symbol != NULL) 
    {
        error_report(ctx->errors, ERR_LABEL_NAME_DUPLICATE, 
                     "%s:%d: Label '%s' already defined", 
                     ctx->ir_filename, ctx->line_number, label_str);
        FREE(label_str);
        return 0;
    }
    
    FREE(label_str);
    return 1;
}

void validate_address(size_t address, AssemblerContext *ctx)
{
    if (address > INT21_MAX) 
    {
        error_report(ctx->errors, ERR_ADD_OUT_OF_BOUNDS, 
                     "%s:%d: Address %lu exceeds maximum allowed value of %lu", 
                     ctx->ir_filename, ctx->line_number, address, INT21_MAX);
    }
}

void define_symbol(AssemblerContext *ctx, ArrayList *tokens)
{
    size_t i = 0;
    Token *token = NULL;
    Token *label = NULL;
    Symbol *symbol = NULL;
    char *str = NULL;
    int is_external = 0;
    int is_entry = 0;
    size_t address = 0;
    StringView sv = {0}, label_sv = {0};

    if (!ctx || !tokens || array_list_size(tokens) == 0)
        return;

    /* Check if the statement has a label */
    token = (Token *)array_list_get(tokens, 0);

    /* Skip label and colon if present */
    if (token && token->type == TOKEN_LABEL)
    {
        label = token;
        label_sv = token->sv;
        i += 2;
    }
    else
        i++;

    /* Check for .directive format */
    token = (Token *)array_list_get(tokens, i);
    if (token && token->type == TOKEN_DOT)
    {
        i++;
        token = (Token *)array_list_get(tokens, i);
    }

    /* For label definitions, validate the label name, colon and directive */
    if (label && !is_entry_statement(tokens) && !is_extern_statement(tokens)) 
    {
        if (!validate_label(label->sv, ctx)) 
            return; /* Invalid label, error already reported */
        
    }

    /* Determine the type of statement and how to handle it */
    if (token && (token->type == TOKEN_INSTRUCTION || token->type == TOKEN_DIR_DATA || token->type == TOKEN_DIR_STRING))
    {
        /* Label defined for instruction or data/string directive */
        if (!label)
            return;
            
        sv = label_sv;  /* Use the label without colon */
        address = ctx->IC;
        validate_address(address, ctx);
    }

    /* External symbol */
    else if (token && token->type == TOKEN_DIR_EXTERN)
    {
        i++;
        token = (Token *)array_list_get(tokens, i);
        sv = token->sv;
        is_external = 1;
    }

    /* Entry symbol */
    else if (token && token->type == TOKEN_DIR_ENTRY)
    {
        i++;
        token = (Token *)array_list_get(tokens, i);
        sv = token->sv;
        is_entry = 1;
    }

    else
        return; 

    /* Create the symbol with the determined attributes */
    symbol = symbol_create(sv, address, is_external, is_entry);
    if (!symbol)
        return;

    /* Add the symbol to the appropriate collections */
    if (is_external || !is_entry)
    {
        str = sv.str;
        str[sv.length] = '\0';
        hash_map_put(ctx->symbol_table, str, symbol);
    }
    
    if (is_external)
        array_list_append(ctx->extern_names, symbol);
    else if (is_entry)
        array_list_append(ctx->entry_names, symbol);
}

void first_pass(AssemblerContext *ctx)
{
    Lexer lexer = {0};
    ArrayList *tokens = NULL;
    ParsedInstruction instruction = {0};
    ParsedDirective directive = {0};
    int is_instruction = 0;
    int is_directive = 0;
   
    if (!ctx)
        return;

    lexer_init(&lexer);
        
    tokens = array_list_create(NULL);
    if (!tokens)
        return;

    ctx->line_number = 1;
    
    /* Iterate through each line of the file */
    while (lexer_next_line(&lexer, ctx))
    {
        /* Tokenize the current line */
        lexer_tokenize_line(&lexer, ctx, tokens);

        /* If a new symbol is encountered, define it */
        if (is_label_statement(tokens) || is_entry_statement(tokens) || is_extern_statement(tokens))
            define_symbol(ctx, tokens);
        
        /* If its an instruction parse it an instruction and update IC */
        if ((is_instruction = is_instruction_statement(tokens)))
        {
            parse_instruction(&instruction, tokens, ctx, 1);
            ctx->IC += instruction.code_word_count;
        }

        /* If its a directive parse it and update DC */
        if ((is_directive = is_directive_statement(tokens)))
        {
            parse_directive(&directive, tokens, ctx);
            ctx->DC += directive.code_word_count;
            ctx->IC += directive.code_word_count;
        }

        /* If the statement is neither an instruction nor a directive, report an error */
        if (!is_instruction && !is_directive)
            error_report(ctx->errors, ERR_INVALID_STATEMENT, "%s:%d: Invalid statement: '%.*s'", 
                         ctx->ir_filename, lexer.line_number, (int)lexer.current_line.length, lexer.current_line.str); 

        /* Clear for next line */
        array_list_clear(tokens);

        /* Update Line number and reset flags */
        ctx->line_number++;
        is_instruction = is_directive = 0;
    }
    
    /* Clean up */
    array_list_destroy(tokens);
}