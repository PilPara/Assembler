/**
 * @file lexer.c
 * @brief Implementation of the lexer module.
 * @details This file contains the implementation of the lexer for the assembler.
 *          It includes functions for creating, destroying, and identifying tokens,
 *          as well as identifying tokens from context of the full line.s
 */


#include "./lexer.h"
#include "./string_view.h"
#include "../main/assembler.h"
#include "./isa.h"
#include "./util.h"
#include "./error.h"

#include <ctype.h>
#include <string.h>
#include <stdio.h>

/* Token types LUT */
char *token_type_str[] = {
    "Invalid",
    "Unknown",
    "Comma",
    "Dot",
    "Colon",
    "Amper",
    "Hash",
    "Quote",
    "Instruction",
    "Register",
    "StrLit",
    "Immediate",
    "Identifier",
    "Label",
    "Directive",
    "Data",
    "String",
    "Entry",
    "Extern"  
};  

char *get_type_str(TokenType type) 
{
    if (type < 0 || type >= sizeof(token_type_str) / sizeof(token_type_str[0]))
        return "Invalid";
    
    return token_type_str[type];
}

Token *token_create(StringView sv, size_t line_number) 
{
    Token *token = NULL;
    
    if (!sv.str)
        return NULL;
    
    token = (Token *)MALLOC(sizeof(Token));
    if (!token)
        return NULL;
    
    token->sv = sv;
    token->type = TOKEN_UNKNOWN;
    token->line_number = line_number;
    token_identify(token);

    return token;
}

void token_destroy(void *token) 
{
    Token *t = (Token *)token;
    
    if (!t)
        return;
    
    FREE(t);
}

int is_identifier(StringView sv) 
{
    size_t i = 0;
    
    if (!sv.str)
        return 0;
    
    /* Check if the first character is a letter */
    if (!isalpha(sv.str[0]))
        return 0;
    
    /* Check if the rest of the string contains only letters, digits, or underscores */
    for (i = 1; i < sv.length; i++) 
        if (!isalnum(sv.str[i]) && sv.str[i] != '_')
            return 0;
    
    return 1;
}

void token_identify(Token *token) 
{
    Directive dir = 0;

    if (!token)
        return;
    
    if (token->sv.length == 0)
        return;

    if (token->type != TOKEN_UNKNOWN)
        return;

    /* Check for special characters */
    if (is_special_char(token->sv)) 
    {
        switch (token->sv.str[0]) 
        {
            case ',':
                token->type = TOKEN_COMMA;
                break;
            case '.':
                token->type = TOKEN_DOT;
                break;
            case ':':
                token->type = TOKEN_COLON;
                break;
            case '&':
                token->type = TOKEN_AMPER;
                break;
            case '#':
                token->type = TOKEN_HASH;
                break;
            case '"':
                token->type = TOKEN_QUOTE;
                break;
        }
    }

    /* Check for keywords */
    else if (is_instruction(token->sv))
        token->type = TOKEN_INSTRUCTION;
    
    else if (is_register(token->sv))
        token->type = TOKEN_REGISTER;

    else if ((dir = is_directive(token->sv)))
    {
        switch (dir - 1)
        {

        case DIR_DATA:
            token->type = TOKEN_DIR_DATA;
            break;

        case DIR_STRING:
            token->type = TOKEN_DIR_STRING;
            break;

        case DIR_ENTRY:
            token->type = TOKEN_DIR_ENTRY;
            break;

        case DIR_EXTERN:
            token->type = TOKEN_DIR_EXTERN;
            break;
            
        }
    }

    else if (is_identifier(token->sv))
        token->type = TOKEN_IDENTIFIER;
            
}

void identify_context(ArrayList *tokens, AssemblerContext *ctx) 
{
    size_t i = 0;
    Token *token = NULL, *prev = NULL, *next = NULL;

    if (!tokens)
        return;

    /* Iterate through the tokens */
    while (i < array_list_size(tokens))
    {
        /* Get the current token */
        token = (Token *)array_list_get(tokens, i);

        /* If possible to get prev and next tokens */
        prev = (i) ? (Token *)array_list_get(tokens, i - 1) : NULL;
        next = (i < array_list_size(tokens) - 1) ? (Token *)array_list_get(tokens, i + 1) : NULL;

        /* Identify Labels */
        if (prev && token->type == TOKEN_COLON)
            prev->type = TOKEN_LABEL;

        /* Check that a label's colon is separated from a directive's dot */
        if (prev && prev->type == TOKEN_COLON && token->type == TOKEN_DOT)
        {
            StringView sv = {0};
            sv = prev->sv;

            if (sv.str[sv.length] == '.')
                error_report(ctx->errors, ERR_LABEL_MISSING_SPACE, "%s:%lu: Invalid label name - Missing whitespace between colon and directive",
                             ctx->ir_filename, prev->line_number);
        }            

        /* Identify Immediate values */
        else if (next && token->type == TOKEN_HASH)
            next->type = TOKEN_IMM;

        else if (prev && token->type == TOKEN_COMMA && prev->type == TOKEN_IMM)
            token->type = TOKEN_IMM;

        /* Identify String Literals */
        else if (prev && token->type == TOKEN_COMMA && prev->type == TOKEN_STR_LIT)
            token->type = TOKEN_STR_LIT;
        
        else if (prev && next && prev->type == TOKEN_QUOTE && next->type == TOKEN_QUOTE) 
            token->type = TOKEN_STR_LIT;

        /* Identify Data Directives */
        else if (token->type == TOKEN_DIR_DATA)
        {
            size_t j;
            
            for (j = i + 1; j < array_list_size(tokens); j++)
            {
                token = (Token *)array_list_get(tokens, j);
                
                if (token->type == TOKEN_COMMA)
                    continue;
            
                token->type = TOKEN_IMM;
            }
            break;
        }        
        i++;
    }
}


void lexer_init(Lexer *lexer) 
{
    if (!lexer)
        return;
    
    lexer->cursor = 0; 
    lexer->line_number = 0;
}

int lexer_next_line(Lexer *lexer, AssemblerContext *ctx) 
{
    if (!lexer || !ctx)
        return 0;

    /* EOF */
    if (lexer->line_number >= array_list_size(ctx->preprocessed_lines))
        return 0;

    /* Read the next line and update the cursor and line number */
    lexer->current_line = sv_from_str(array_list_get(ctx->preprocessed_lines, lexer->line_number));
    lexer->cursor = 0;
    lexer->line_number++;
    
    return 1;
}

void lexer_tokenize_line(Lexer *lexer, AssemblerContext *ctx, ArrayList *tokens)
{
    Token *token = NULL;
    StringView sv = {0};
    size_t i = 0;
    size_t start = 0;

    if (!lexer)
        return;
    
    while (i < lexer->current_line.length) 
    {
        /* Skip whitespace */
        while (i < lexer->current_line.length && (lexer->current_line.str[i] == ' ' || lexer->current_line.str[i] == '\t'))
            i++;

        /* Check for end of line */
        if (i >= lexer->current_line.length) 
            break;

        /* Check for single char tokens */
        if (is_special_char(sv_from_parts(lexer->current_line.str + i, 1)))
        {
            sv = sv_from_str(lexer->current_line.str + i);
            sv.length = 1;
            
            token = token_create(sv, lexer->line_number);

            if (!token)
                return;
                
            array_list_append(tokens, token);               
            array_list_append(ctx->tokens, token);
            i++;
            continue;
        }       
        
        /* Check for multi-char tokens */
        start = i;

        while (i < lexer->current_line.length && 
               !is_special_char(sv_from_parts(lexer->current_line.str + i, 1)) &&
               lexer->current_line.str[i] != ' ' && lexer->current_line.str[i] != '\t')
            i++;


        /* Set the token */
        sv = sv_from_str(lexer->current_line.str + start);
        sv.length = i - start;

        token = token_create(sv, lexer->line_number);

        /* Add the token to the list */
        array_list_append(tokens, token);
        array_list_append(ctx->tokens, token);
    } 

    /* Identify token types from context */
    identify_context(tokens, ctx);

    /* Check for other token errors */
    for (i = 0; i < array_list_size(tokens); i++)
    {
        token = (Token *)array_list_get(tokens, i);
        if (token->type == TOKEN_UNKNOWN)
        {
            token->type = TOKEN_INVALID;
            token->sv.str = NULL;
            token->sv.length = 0;
            
            error_report(ctx->errors, ERR_INVALID_TOKEN, "%s:%lu: Invalid token '%.*s'", 
                        ctx->ir_filename, token->line_number, (int)token->sv.length, token->sv.str);
        }
    }
}
