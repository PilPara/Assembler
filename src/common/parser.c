/**
 * @file parser.c
 * @brief Implementation of the parser for the assembler.
 * @details This file contains the implementation of functions for parsing instructions and directives.
 *          It includes functions for initializing, parsing, and validating parsed instructions and directives.
 */

#include "./parser.h"
#include "../data_structures/array_list.h"
#include "./lexer.h"
#include "../common/error.h"
#include "../common/util.h"
#include "../common/isa.h"

#include <string.h>
#include <stdlib.h>
#include <limits.h>

void parsed_instruction_init(ParsedInstruction *instruction)
{
    if (!instruction)
        return;

    instruction->label = NULL;
    instruction->instruction = NULL;
    instruction->rs = NULL;
    instruction->rs_add_mode = ADD_MOD_NONE;
    instruction->rt = NULL;
    instruction->rt_add_mode = ADD_MOD_NONE;
    instruction->operand_count = 0;
    instruction->code_word_count = 0;
    instruction->tokens = NULL;
}

void parse_instruction(ParsedInstruction *instruction, ArrayList *tokens, AssemblerContext *ctx, int mode)
{
    size_t i = 0;
    Token *token = NULL;
    int is_amper = 0;
    int comma_count = 0;

    if (!tokens || array_list_size(tokens) == 0 || !instruction || !ctx)
        return;

    /* Initialize and add one default code word */
    parsed_instruction_init(instruction);
    instruction->tokens = tokens;
    instruction->code_word_count += 1;
    
    /* Check if the statement has a label */
    token = (Token *)array_list_get(tokens, i);
    /* If so skip label token and : token */
    if (token->type == TOKEN_LABEL)
    {
        instruction->label = token;
        i += 2;
    }

    /* No Label */
    else if (token->type == TOKEN_IDENTIFIER)
    {
        /* Validate colon */
        token = (Token *)array_list_get(tokens, i + 1);
        if (token && token->type != TOKEN_COLON)
        {
            error_report(ctx->errors, ERR_LABEL_MISSING_COLON, "%s:%lu: Missing colon after label",
                     ctx->ir_filename, ctx->line_number);
            return;
        }
    }

    /* Check for instruction token */
    instruction->operand_count = count_operands(tokens);
    instruction->instruction = (Token *)array_list_get(tokens, i);
    i++;

    /* Iterate through the tokens */
    for (; i < array_list_size(tokens); i++)
    {
        token = (Token *)array_list_get(tokens, i);

        /* For relative addressing */
        if (token->type == TOKEN_AMPER)
            is_amper = 1;

        if (is_operand(token))
        {
            /* Src */
            if (instruction->rs == NULL)
            {
                instruction->rs = token;
                instruction->rs_add_mode = get_addressing_mode(token, is_amper);
                
                /* Validate immediate operand if in mode 1 */
                if (mode == 1 && token->type == TOKEN_IMM) 
                {
                    if (!validate_immediate(token, ctx)) 
                        return;
                }

            }
            /* Dst */
            
            else if (instruction->rt == NULL)
            {
                instruction->rt = token;
                instruction->rt_add_mode = get_addressing_mode(token, is_amper);
                
                /* Validate immediate operand if in mode 1 */
                if (mode == 1 && token->type == TOKEN_IMM) {
                    if (!validate_immediate(token, ctx)) 
                        return;
                }
            }
        }
        /* Illegal Comma check */
        else if (token->type == TOKEN_COMMA)
            comma_count++;
    }

    /* If only one operand is present, set rt to rs */
    if (instruction->operand_count == 1)
    {
        instruction->rt = instruction->rs;
        instruction->rs = NULL;
        instruction->rt_add_mode = instruction->rs_add_mode;
        instruction->rs_add_mode = ADD_MOD_NONE;
    }

    /* Calculate code word count */
    if (instruction->rs)
        instruction->code_word_count += calculate_code_word(instruction->rs->type);

    if (instruction->rt)
        instruction->code_word_count += calculate_code_word(instruction->rt->type);

    /* Validate instruction */
    if (mode == 1)
    {
        validate_instruction(instruction, ctx);

        if (instruction->operand_count == 2 && comma_count != 1)
            error_report(ctx->errors, ERR_INST_ILLEGAL_NUM_COMMA, "%s:%lu: Invalid number of commas in instruction '%.*s'",
                     ctx->ir_filename, ctx->line_number,
                     (int)instruction->instruction->sv.length, instruction->instruction->sv.str);
    }
}

void parsed_directive_init(ParsedDirective *directive)
{
    if (!directive)
        return;

    directive->label = NULL;
    directive->directive = NULL;
    directive->tokens = NULL;
    directive->code_word_count = 0;
}

void parsed_directive_destroy(void *directive)
{
    ParsedDirective *dir = (ParsedDirective *)directive;
    dir->label = NULL;
    dir->directive = NULL;
    dir->tokens = NULL;
    dir->code_word_count = 0;
}

void parse_directive(ParsedDirective *directive, ArrayList *tokens, AssemblerContext *ctx)
{
    size_t i = 0;
    Token *token = NULL;

    if (!tokens || array_list_size(tokens) == 0)
        return;

    /* Initialize */
    parsed_directive_init(directive);
    directive->tokens = tokens;

    /* Check if the statement has a label */
    token = (Token *)array_list_get(tokens, i);
    if (token->type == TOKEN_LABEL)
    {
        directive->label = token;
        i+= 2;
    }

    /* Check for .directive format */
    token = (Token *)array_list_get(tokens, i);
    if (token->type == TOKEN_DOT)   
        i++;

    /* Missing dot */
    else 
    {
        error_report(ctx->errors, ERR_DIR_DOT_MISSING, "%s:%lu: Invalid directive statement - a dot is missing before the directive",
                     ctx->ir_filename, token->line_number);
        return;
                     
    }

    /* Check for directive token */ 
    directive->directive = (Token *)array_list_get(tokens, i);
    i++;

    /* String directive */
    if (directive->directive->type == TOKEN_DIR_STRING)
    {
        Token *next;
        next = (Token *)array_list_get(tokens, i);
        
        /* Check for illegal commas and quotes at the start */
        if (next)
        {
            if (next->type == TOKEN_COMMA)
            {
                error_report(ctx->errors, ERR_DIR_STR_ILLEGAL_COMMA, "%s:%lu: Illegal comma in string directive - string directive cannot start with a comma",
                             ctx->ir_filename, token->line_number);
                return;
            }
            
            else if (next->type != TOKEN_QUOTE)
            {
                error_report(ctx->errors, ERR_DIR_STR_MISSING_QUOTE, "%s:%lu: Invalid string directive - expected a quote at the beginning of the string",
                             ctx->ir_filename, token->line_number);
                return;
            }


        }
        
        /* Updated DC */
        for (i = 0; i < array_list_size(tokens); i++)
        {
            token = (Token *)array_list_get(tokens, i);
            if (token->type == TOKEN_STR_LIT)
                directive->code_word_count += strlen(token->sv.str);
        }

        token = (Token *)array_list_get(tokens, array_list_size(tokens) - 1);

        /* Check for illegal commas and quotes at the end */
        if (token->type == TOKEN_COMMA)
        {
            error_report(ctx->errors, ERR_DIR_STR_ILLEGAL_COMMA, "%s:%lu: Illegal comma in string directive - string directive cannot end with a comma",
                         ctx->ir_filename, token->line_number);
            return;
        }

        else if (token->type != TOKEN_QUOTE)
        {
            error_report(ctx->errors, ERR_DIR_STR_MISSING_QUOTE, "%s:%lu: Illegal token in string directive - expected a quote at the end of the string",
                         ctx->ir_filename, token->line_number);
            return;
        }
                         
    }

    /* Data directive */
    else if (directive->directive->type == TOKEN_DIR_DATA)
    {
        for (i = 0; i < array_list_size(tokens); i++)
        { 
            Token *next = NULL;

            token = (Token *)array_list_get(tokens, i);
            /* Check for illegal commas */
            if (token->type == TOKEN_IMM) 
            {
                next = (Token *)array_list_get(tokens, i + 1);
                
                if (next && i != array_list_size(tokens)&& next->type != TOKEN_COMMA)
                {
                    error_report(ctx->errors, ERR_DIR_DATA_ILLEGAL_COMMA, "%s:%lu: Missining comma between elements in data directive - expected comma after '%.*s', instead got '%.*s'",
                                 ctx->ir_filename, token->line_number, 
                                 (int)token->sv.length, token->sv.str,
                                    (int)next->sv.length, next->sv.str);
                    return;
                }

                /* Validate data immediate value */
                if (!validate_data_value(token, ctx)) 
                    return;

                directive->code_word_count += 1;
            }

            /* Check for illegal commas */
            if (token->type == TOKEN_COMMA)
            {
                next = (Token *)array_list_get(tokens, i + 1);
                if (next && next->type == TOKEN_COMMA)
                {
                    error_report(ctx->errors, ERR_DIR_MULTY_COMMAS, "%s:%lu: Multiple consecutive commas in data directive",
                                 ctx->ir_filename, token->line_number);
                    return;
                }
            }
            
            if (token->type == TOKEN_DIR_DATA)
            {
                next = (Token *)array_list_get(tokens, i + 1);
                if (next->type == TOKEN_COMMA)
                {
                    error_report(ctx->errors, ERR_DIR_DATA_ILLEGAL_COMMA, "%s:%lu: Illegal comma in data directive - integer list cannot start with a comma",
                                 ctx->ir_filename, token->line_number);
                    return;                              

                }
            }

            if (i == array_list_size(tokens) - 1)
            {
                if (token->type == TOKEN_COMMA)
                    error_report(ctx->errors, ERR_DIR_DATA_ILLEGAL_COMMA, "%s:%lu: Illegal comma in data directive - integer list cannot end with a comma",
                                 ctx->ir_filename, token->line_number);
                return;
            }
        }
    }
        
}
    
int is_operand(Token *token)
{
    if (!token)
        return 0;

    return token->type == TOKEN_REGISTER || token->type == TOKEN_IMM || token->type == TOKEN_IDENTIFIER;
}

int count_operands(ArrayList *tokens)
{
    size_t i = 0;
    Token *token = NULL;
    int count = 0;

    if (!tokens || array_list_size(tokens) == 0)
        return 0;

    for (i = 0; i < array_list_size(tokens); i++)
    {
        token = (Token *)array_list_get(tokens, i);
        if (is_operand(token))
            count++;
    }

    return count;
}

int calculate_code_word(TokenType type) 
{
    return type == TOKEN_IMM || type == TOKEN_IDENTIFIER;
}

AddressingMode get_addressing_mode(Token *token, int is_amper)
{
    if (is_amper)
        return ADD_MOD_RELATIVE;

    if (!token)
        return ADD_MOD_NONE;

    switch (token->type)
    {
        case TOKEN_IMM:
            return ADD_MOD_IMMEDIATE;

        case TOKEN_REGISTER:
            return ADD_MOD_REGISTER;

        case TOKEN_IDENTIFIER:
            return ADD_MOD_DIRECT;

        default:
            return ADD_MOD_NONE;
    }
}

int is_label_statement(ArrayList *tokens)
{
    Token *token = NULL;

    if (!tokens || array_list_size(tokens) == 0)
        return 0;

    token = (Token *)array_list_get(tokens, 0);
    
    return token->type == TOKEN_LABEL;
}

int is_instruction_statement(ArrayList *tokens)
{
    size_t i = 0;
    Token *token = NULL;

    if (!tokens || array_list_size(tokens) == 0)
        return 0;

    /* Skip label and colon tokens */
    if (is_label_statement(tokens))
        i += 2;

    for (; i < array_list_size(tokens); i++)
    {
        token = (Token *)array_list_get(tokens, i);
        if (token->type == TOKEN_INSTRUCTION)
            return 1;
    }

    return 0;
}

int is_entry_statement(ArrayList *tokens)
{
    size_t i = 0;
    Token *token = NULL;

    if (!tokens || array_list_size(tokens) == 0)
        return 0;

    for (; i < array_list_size(tokens); i++)
    {
        token = (Token *)array_list_get(tokens, i);
        if (token->type == TOKEN_DIR_ENTRY)
            return 1;
    }

    return 0;
}

int is_extern_statement(ArrayList *tokens)
{
    size_t i = 0;
    Token *token = NULL;

    if (!tokens || array_list_size(tokens) == 0)
        return 0;

    for (; i < array_list_size(tokens); i++)
    {
        token = (Token *)array_list_get(tokens, i);
        if (token->type == TOKEN_DIR_EXTERN)
            return 1;
    }

    return 0;
}

int is_directive_statement(ArrayList *tokens)
{
    size_t i = 0;
    Token *token = NULL;

    if (!tokens || array_list_size(tokens) == 0)
        return 0;

    if (is_label_statement(tokens))
        i += 2;

    for (;i < array_list_size(tokens); i++)
    {
        token = (Token *)array_list_get(tokens, i);
        if (token->type == TOKEN_DIR_STRING || token->type == TOKEN_DIR_DATA || 
            token->type == TOKEN_DIR_ENTRY || token->type == TOKEN_DIR_EXTERN)
            return 1;
    }

    return 0;
}

int validate_immediate(Token *token, AssemblerContext *ctx) 
{
    char *str = NULL;
    char *endptr = NULL;
    long value;
    
    if (!token || !ctx || token->type != TOKEN_IMM)
        return 0;
    
    str = STRDUP(token->sv.str);
    if (!str) 
        return 0;
    
    /* Check if it's a valid number (can have leading +/- sign) */
    value = strtol(str, &endptr, DEC_BASE);
    
    if (*endptr != '\0') 
    {
        error_report(ctx->errors, ERR_INVALID_IMM, 
                 "%s:%lu: Invalid immediate value '%s'", 
                 ctx->ir_filename, token->line_number, str);
        FREE(str);
        return 0;
    }
    
    /* Check if value is within range for 21 bits (signed) */
    if (value < INT21_MIN || value > INT21_MAX) 
    {
        error_report(ctx->errors, ERR_IMM_OUT_OF_BOUNDS, 
                 "%s:%lu: Immediate value %ld is out of range (-2^20 to 2^20-1)", 
                 ctx->ir_filename, token->line_number, value);
        FREE(str);
        return 0;
    }
    
    FREE(str);
    return 1;
}

/* Validates a numeric value in a data directive */
int validate_data_value(Token *token, AssemblerContext *ctx) 
{
    char *str;
    long value = 0;
    
    if (!token || !ctx || token->type != TOKEN_IMM)
        return 0;
    
    str = token->sv.str;
    str[token->sv.length] = '\0';

    value = atoi(str);

    if (value == 0 && strcmp(str, "0") != 0) 
    {
        error_report(ctx->errors, ERR_INVALID_DATA, 
                 "%s:%lu: Invalid data value '%s'", 
                 ctx->ir_filename, token->line_number, str);
        return 0;
    }

    if (value < INT21_MIN|| value > INT21_MAX - 1) 
    {
        error_report(ctx->errors, ERR_IMM_OUT_OF_BOUNDS, 
                 "%s:%lu: Data value %ld is out of range (-2^20 to 2^20-1)", 
                 ctx->ir_filename, token->line_number, value);
        return 0;
    }
    
    return 1;
}

int validate_instruction(ParsedInstruction *instruction, AssemblerContext *ctx)
{
    InstructionInfo *info = NULL;
    char *instruction_name = NULL;
    int is_valid = 1;
    
    if (!instruction || !ctx || !instruction->instruction)
        return 0;
    
    /* Get instruction info from table */
    instruction_name = instruction->instruction->sv.str;
    instruction_name[instruction->instruction->sv.length] = '\0';
    info = find_instruction(instruction_name);
    
    if (!info)
        return 0;
    
    /* Check operand count */
    if (instruction->operand_count != info->num_operands) 
    {
        error_report(ctx->errors, ERR_SYNTAX_NUM_OPERANDS, "%s:%lu: Invalid number of operands for instruction '%s'. Expected %d, got %d",
            ctx->ir_filename, instruction->instruction->line_number,
            instruction_name, info->num_operands, instruction->operand_count);
        is_valid = 0;
    }
    
    /* Check source operand addressing mode if present */
    if (instruction->rs) 
    {
        /* Check if the instruction supports the addressing mode */
        unsigned int rs_mode_mask = 1 << instruction->rs_add_mode;
        
        if (!(rs_mode_mask & info->allowd_src_add_mode)) 
        {
            error_report(ctx->errors, ERR_SYNTAX_ADD_MOD, "%s:%lu: Invalid addressing mode '%s' for source operand in '%s'",
                   ctx->ir_filename, instruction->instruction->line_number,
                   get_addressing_mode_str(instruction->rs_add_mode),
                   instruction_name);
            is_valid = 0;
        }
    }
    
    /* Check destination operand addressing mode if present */
    if (instruction->rt) 
    {
        /* Check if the instruction supports the addressing mode */
        unsigned int rt_mode_mask = 1 << instruction->rt_add_mode;
        
        if (!(rt_mode_mask & info->allowd_dst_add_mode)) 
        {
           error_report(ctx->errors, ERR_SYNTAX_ADD_MOD, "%s:%lu: Invalid addressing mode '%s' for destination operand in '%s'",
                   ctx->ir_filename, instruction->instruction->line_number,
                   get_addressing_mode_str(instruction->rt_add_mode),
                   instruction_name);
            is_valid = 0;
        }
    }

    return is_valid;
}
