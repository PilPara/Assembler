/**
 * @file second_pass.c
 * @brief Implementation of the second pass of the assembler.
 * @details This file contains functions for the second pass of the assembler,
 *          which includes translating instructions and directives into machine code,
 *          resolving symbols, and generating output files (.ob, .ent, .ext) (if assembly is successful).
 *          The second pass also collects entry and external symbols if encountered.
 */

#include "./second_pass.h"
#include "./first_pass.h"
#include "../common/code_gen.h"
#include "../common/lexer.h"
#include "../common/error.h"
#include "../common/file_io.h"
#include "../common/lexer.h"
#include "../common/util.h"
#include "../common/isa.h"
#include "../common/parser.h"
#include "../data_structures/array_list.h"

#include <stdio.h>
#include <string.h>

void get_line(AssemblerContext *ctx, ArrayList *tokens)
{
    size_t current_line_number = 0;
    size_t i = 0;
    Token *token = NULL;

    if (!ctx || !tokens)
        return;

    current_line_number = ctx->line_number;

    /* Add the tokens from the current line to the tokens list */
    while (i < array_list_size(ctx->tokens))
    {
        token = (Token *)array_list_get(ctx->tokens, i);
        
        /* Check if the token belongs to the current line */
        if (token->line_number < current_line_number)
        {
            i++;
            continue;
        }
        
        /* If the token is from a different line, break the loop */
        if (token->line_number > current_line_number)
            break;
            
        array_list_append(tokens, token);
        i++;
    }
    
    ctx->line_number++;
}

void process_token(Token *token, AssemblerContext *ctx, int address, int is_extern)
{
    Symbol *symbol = NULL;
    char *str = NULL;
    size_t i = 0;
    ArrayList *names_list = NULL;
    ArrayList *target_list = NULL;
    
    if (!token || !ctx)
        return;
    
    /* Set the names list and target list based on whether it's extern or entry */
    names_list = is_extern ? ctx->extern_names : ctx->entry_names;
    target_list = is_extern ? ctx->externals : ctx->entries;
    
    /* Get the string representation of the token */
    str = token->sv.str;
    str[token->sv.length] = '\0';
    
    /* Check if the symbol already exists in the symbol table */
    for (i = 0; i < array_list_size(names_list); i++)
    {
        Symbol *list_symbol = (Symbol *)array_list_get(names_list, i);
        if (strcmp(str, list_symbol->sv.str) == 0)
        {
            Symbol *symbol_copy = NULL;
            symbol = (Symbol *)hash_map_get(ctx->symbol_table, str);

            if (!symbol)
                return;

            /* For entries, use the address from the symbol table.
               For externals, use the address calculated during the second pass */
            symbol_copy = symbol_create(token->sv, 
                                      is_extern ? address : symbol->address, 
                                      0, 1);

            array_list_append(target_list, symbol_copy);
            break;
        }
    }
}

void log_symbol(ParsedInstruction *instruction, AssemblerContext *ctx, int IC, int is_externs, int is_entries)
{
    if (!instruction || !ctx)
        return;

    /* Process label if present */
    if (is_label_statement(instruction->tokens))
    {
        Token *label = (Token *)array_list_get(instruction->tokens, 0);
        
        if (is_externs)
            process_token(label, ctx, IC, 1);
            
        if (is_entries)
            process_token(label, ctx, IC, 0);
    }

    /* Process source register operand if it's an identifier */
    if (instruction->rs && instruction->rs->type == TOKEN_IDENTIFIER)
    {
        if (is_externs)
            process_token(instruction->rs, ctx, IC + 1, 1);
            
        if (is_entries)
            process_token(instruction->rs, ctx, IC + 1, 0);
    }

    /* Process target register operand if it's an identifier */
    if (instruction->rt && instruction->rt->type == TOKEN_IDENTIFIER)
    {
        /* For target operand, we need to calculate its position based on source operand */
        int rt_position = IC + 1;
        
        if (instruction->rs && (instruction->rs->type == TOKEN_IMM || 
                               instruction->rs->type == TOKEN_IDENTIFIER))
            rt_position = IC + 2;
            
        if (is_externs)
            process_token(instruction->rt, ctx, rt_position, 1);
            
        if (is_entries)
            process_token(instruction->rt, ctx, rt_position, 0);
    }
}


void second_pass(AssemblerContext *ctx)
{
    ParsedInstruction instruction = {0};
    ParsedDirective directive = {0};
    ArrayList *line = NULL;
    size_t max_line_number = 0;
    int IC = 100;
    int DC = 0;
    int is_externs = 0;
    int is_entries = 0;

    line = array_list_create(NULL);

    if (!line)
        return;

    /* Initialize the line number and get the maximum line number */
    max_line_number = ((Token *)array_list_get(ctx->tokens, array_list_size(ctx->tokens) - 1))->line_number + 1;
    ctx->line_number = 1;

    /* Check if there are any entries or externals */
    if (array_list_size(ctx->entry_names))
        is_entries = 1;
    
    if (array_list_size(ctx->extern_names))
        is_externs = 1;
    
    /* Process each line until the maximum line number is reached */
    while (ctx->line_number <= max_line_number)
    {
        /* Get the tokens for the current line */
        get_line(ctx, line);

        /* Instruction statement */
        if (is_instruction_statement(line))
        {
            parse_instruction(&instruction, line, ctx, 2);

            /* If one of the operands is an identifier - check if entry or extern */
            if ((instruction.rs && instruction.rs->type == TOKEN_IDENTIFIER) || 
                (instruction.rt && instruction.rt->type == TOKEN_IDENTIFIER) || 
                (is_label_statement(line)))
            {
                log_symbol(&instruction, ctx, IC, is_externs, is_entries);
            }

            /* Translate the instruction */
            encode_instruction(&instruction, ctx, &IC);
        }

        /* Directive statement */
        if (is_directive_statement(line))
        {
            parse_directive(&directive, line, ctx);
            encode_data(&directive, ctx, &IC, &DC);
        }
    
        array_list_clear(line);
    }

    /* Check if there are any errors */
    if (array_list_size(ctx->errors) == 0)
        generate_output(ctx, 2);

    array_list_destroy(line);
}