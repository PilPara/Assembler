/**
 * @file code_gen.c
 * @brief Implementation of code generation functions.
 * @details This file contains functions for encoding instructions and directives into machine code.
 *         It includes functions for creating and destroying words, encoding operands, and manipulating fields in words,
 *         mostyly via bitwise operations.
 */

#include "./code_gen.h"
#include "../assembly/first_pass.h"
#include "../common/isa.h"
#include "../common/util.h"
#include "../common/error.h"

#include <stdlib.h>
#include <string.h>

/* Macro for setting bits in a word */

#define SET_BITS(word, value, mask, pos) \
    (word) = ((word) & ~(mask)) | (((value) << (pos)) & (mask))

Word *word_create(unsigned int value)
{
    Word *word = NULL;

    word = (Word *)MALLOC(sizeof(Word));

    if (!word)
        return NULL;

    word->value = value;
    word->address = 0;

    return word;
}

void word_destroy(void *word)
{
    Word *w = (Word *)word;

    if (!word)
        return;

    FREE(w);
}

void encode_operand(Token *operand, Word *word, int is_source, int is_relative)
{
    RegisterInfo *reg_info = NULL;
    char *reg_name = NULL;

    if (!operand || !word)
        return;

    /* Register Handling */
    if (operand->type == TOKEN_REGISTER)
    {
        /* Get the register name and find its info */
        reg_name = operand->sv.str;
        reg_name[operand->sv.length] = '\0';
        reg_info = find_register(reg_name);

        if (reg_info)
        {
            if (is_source)
            {
                word_set_rs_add_mod(word, ADD_MOD_REGISTER);
                word_set_rs_operand(word, reg_info->reg);
            }
            else
            {
                word_set_rt_add_mod(word, ADD_MOD_REGISTER);
                word_set_rt_operand(word, reg_info->reg);
            }
        }
    }   

    /* Immediate Handling */
    else if (operand->type == TOKEN_IMM)
    {
        if (is_source)
            word_set_rs_add_mod(word, ADD_MOD_IMMEDIATE);
        else
            word_set_rt_add_mod(word, ADD_MOD_IMMEDIATE);
    }

    /* Identifier Handling */
    else if (operand->type == TOKEN_IDENTIFIER)
    {
        if (is_relative)
        {
            if (is_source)
                word_set_rs_add_mod(word, ADD_MOD_RELATIVE);
            else
                word_set_rt_add_mod(word, ADD_MOD_RELATIVE);
        }
        else 
        {
            if (is_source)
                word_set_rs_add_mod(word, ADD_MOD_DIRECT);
            else
                word_set_rt_add_mod(word, ADD_MOD_DIRECT);
        }
    }
}

void encode_first_word(ParsedInstruction *instruction, AssemblerContext *ctx, int *IC)
{
    InstructionInfo *info = NULL;
    Word *word = NULL;
    char *instruction_name = NULL;

    if (!instruction || !ctx)
        return;

    /* Get instruction info from table */
    instruction_name = instruction->instruction->sv.str;
    instruction_name[instruction->instruction->sv.length] = '\0';
    info = find_instruction(instruction_name);

    if (!info)
        return;

    /* Create an empty word */
    word = word_create(0);

    /* Set the opcode and funct fields */
    word_set_are(word, ARE_ABSOLUTE);

    if (instruction->instruction)
    {
        word_set_opcode(word, info->opcode);
        word_set_funct(word, info->funct);
    }

    /* Encode operands - might generate multiple words */
    if (instruction->rs)
        encode_operand(instruction->rs, word, 1, instruction->rs_add_mode == ADD_MOD_RELATIVE);
    

    if (instruction->rt)
        encode_operand(instruction->rt, word, 0, instruction->rt_add_mode == ADD_MOD_RELATIVE);
    
    /* Set the address */
    word->address = *IC;
    array_list_append(ctx->code_img, word);
}

void encode_operand_extra(Token *operand, AssemblerContext *ctx, Word *word, AddressingMode add_mode, int current_IC)
{
    unsigned int imm = 0;
    size_t address = 0;
    Symbol *symbol = NULL;
    char *symbol_name = NULL;
    long value = 0;

    if (!operand || !ctx || !word)
        return;

    /* Handle extra word for immediate */
    if (operand->type == TOKEN_IMM)
    {
        value = atoi(operand->sv.str);

        /* Validate immediate value is within 21-bit signed range */
        if (value > INT21_MAX || value < INT21_MIN) 
        {
            error_report(ctx->errors, ERR_IMM_OUT_OF_BOUNDS, 
               "%s:%lu: Immediate value %ld exceeds allowed range (%d to %d)", 
               ctx->ir_filename, operand->line_number, value, INT21_MAX, INT21_MIN);
        }

        imm = value;
        word_from_immediate(word, imm);
        word_set_are(word, ARE_ABSOLUTE);
    }
    /* Handle extra word for identifier */

    else if (operand->type == TOKEN_IDENTIFIER)
    {   
        /* Get the symbol from the symbol table */
        symbol_name = operand->sv.str;
        symbol_name[operand->sv.length] = '\0';
        symbol = hash_map_get(ctx->symbol_table, symbol_name);

        if (!symbol)
        {
            error_report(ctx->errors, ERR_SYMBOL_NOT_FOUND, "%s:%d:Symbol '%s' not found in symbol table", 
                ctx->ir_filename, ctx->line_number, symbol_name);
            return;
        }

        if (add_mode == ADD_MOD_DIRECT)
        {
            address = symbol->address;

            /*  Validate the address is within range */
            if (address > UINT24_MAX) 
            {
                error_report(ctx->errors, ERR_ADD_OUT_OF_BOUNDS, 
                   "%s:%lu: Symbol address %lu exceeds maximum allowed value of %lu", 
                   ctx->ir_filename, operand->line_number, address, UINT24_MAX);
            }

            /* Set the address in the word */
            word_from_immediate(word, address);

            if (symbol->external)
                word_set_are(word, ARE_EXTERNAL);
            else
                word_set_are(word, ARE_RELOCATABLE);
        }
        else if (add_mode == ADD_MOD_RELATIVE)
        {
            /*  Calculate relative address */
            address = symbol->address - current_IC + 1;

            /*  Validate the offset is within 21-bit signed range */
            if ((long)address > INT21_MAX || (long)address < INT21_MIN) 
            {
                error_report(ctx->errors, ERR_ADD_OUT_OF_BOUNDS, 
                   "%s:%lu: Relative address offset %ld exceeds allowed range (%d to %d)", 
                   ctx->ir_filename, operand->line_number, (long)address, INT21_MIN, INT21_MAX);
            }

            word_from_immediate(word, address);
            word_set_are(word, ARE_ABSOLUTE);
        }
    }

    /* Append the word to the code image */
    word->address = current_IC;
    array_list_append(ctx->code_img, word);
}

void encode_instruction(ParsedInstruction *instruction, AssemblerContext *ctx, int *IC)
{
    Word *word = NULL;

    if (!instruction || !ctx)
        return;
    
    /* Encode the first word and update the instruction counter */
    encode_first_word(instruction, ctx, IC);
    (*IC)++;

    /* Handle RS operand if present */
    if (instruction->rs)
    {
        if (instruction->rs->type == TOKEN_IMM || instruction->rs->type == TOKEN_IDENTIFIER)
        {
            word = word_create(0);
            encode_operand_extra(instruction->rs, ctx, word, instruction->rs_add_mode, *IC);
            (*IC)++;
        }
    }
    
    /* Handle RT operand if present */
    if (instruction->rt)
    {
        if (instruction->rt->type == TOKEN_IMM || instruction->rt->type == TOKEN_IDENTIFIER)
        {
            word = word_create(0);
            encode_operand_extra(instruction->rt, ctx, word, instruction->rt_add_mode, *IC);
            (*IC)++;
        }
    }
}

void encode_data(ParsedDirective *directive, AssemblerContext *ctx, int *IC, int *DC)
{
    Word *word = NULL;
    size_t i = 0;
    Token *token = NULL;
    long value = 0;

    if (!directive || !ctx)
        return;

    /* Check if there is a label */
    token = (Token *)array_list_get(directive->tokens, i);

    /* Skip label, colon and dot before directive */
    if (token->type == TOKEN_LABEL)
        i += 3;
    /* No label, skip only the dot) */
    else
        i++;        
    
    token = (Token *)array_list_get(directive->tokens, i);

    /* Data directive */
    if (token->type == TOKEN_DIR_DATA)
    {
        for (; i < array_list_size(directive->tokens); i++)
        {
            token = (Token *)array_list_get(directive->tokens, i);

            if (token->type == TOKEN_IMM)
            {
                value = atoi(token->sv.str);
                
                if (value > INT21_MAX || value < INT21_MIN) {
                    error_report(ctx->errors, ERR_IMM_OUT_OF_BOUNDS, 
                                "%s:%lu: Data value %ld exceeds 24-bit range (%d to %d)", 
                                ctx->ir_filename, token->line_number, value, INT21_MIN, INT21_MAX);
                }
                
                /* A new word per immediate value */
                word = word_create(0);
                data_from_immediate(word, value);
                array_list_append(ctx->data_img, word);
                word->address = *IC;
                (*DC)++;
                (*IC)++;
            }
        }
    }

    /* String directive */
    else
    if (token->type == TOKEN_DIR_STRING)
    {
        char *str = NULL;
        size_t length = 0;

        /* Skip the .string token and " */
        i += 2;

        /* Get the string literal */
        token = (Token *)array_list_get(directive->tokens, i);
        str = token->sv.str;
        str[token->sv.length] = '\0';

        length = strlen(str);
        
        for (i = 0; i < length; i++)
        {
            /* A word per character */
            word = word_create(0);
            data_from_immediate(word, str[i]);
            array_list_append(ctx->data_img, word);
            word->address = *IC;
            (*DC)++;
            (*IC)++;
        }

        /* Add a null terminator */
        word = word_create(0);
        word->address = *IC;
        (*DC)++;
        (*IC)++;
        array_list_append(ctx->data_img, word);
    }
}

void word_set_opcode(Word *word, int opcode)
{
    if (!word)
        return;

    SET_BITS(word->value, opcode, OPCODE_MASK, OPCODE_POS);
}

void word_set_rs_add_mod(Word *word, unsigned int mode)
{
    if (!word)
        return;

    SET_BITS(word->value, mode, SRC_ADD_MODE_MASK, SRC_ADD_MODE_POS);
}

void word_set_rs_operand(Word *word, unsigned int operand)
{
    if (!word)
        return;

    SET_BITS(word->value, operand, SRC_OPERAND_MASK, SRC_OPERAND_POS);
}

void word_set_rt_add_mod(Word *word, unsigned int mode)
{
    if (!word)
        return;

    SET_BITS(word->value, mode, DST_ADD_MODE_MASK, DST_ADD_MODE_POS);
}

void word_set_rt_operand(Word *word, unsigned int operand)
{
    if (!word)
        return;

    SET_BITS(word->value, operand, DST_OPERAND_MASK, DST_OPERAND_POS);
}

void word_set_funct(Word *word, unsigned int funct)
{
    if (!word)
        return;

    SET_BITS(word->value, funct, FUNCT_MASK, FUNCT_POS);
}

void word_set_are(Word *word, unsigned int are)
{
    if (!word)
        return;

    SET_BITS(word->value, are, ARE_MASK, ARE_POS);
}

void word_from_immediate(Word *word, int immediate)
{
    if (!word)
        return;

    word->value = (immediate << IMM_SHIFT) & IMM_MASK;
}

void data_from_immediate(Word *word, int immediate)
{
    if (!word)
        return;

    word->value = immediate & WORD_MASK;
}