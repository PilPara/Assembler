/**
 * @file isa.c
 * @brief Implementation of the Instruction Set Architecture (ISA) for the assembler.
 * @details This file contains the implementation of functions for handling
 *          instructions, registers, directives, and addressing modes. It includes
 *          functions for checking if a string is an instruction, finding an
 *          instruction or register, and getting the string representation of
 *          addressing modes. It also includes the definition of the instruction
 *          set, register set, and directives.
 */

#include "./isa.h"

#include <string.h>
#include "./string_view.h"  

/* Macros for easy size of look up tables */
#define TABLE_SIZE(type, name) \
    (sizeof(name) / sizeof(type))


/* Instruction set LUT */
InstructionInfo instruction_set[] = {
    {"mov", OPCODE_MOV, FUNCT_NONE, 2, ADD_IMMEDIATE | ADD_DIRECT | ADD_REGISTER, ADD_DIRECT | ADD_REGISTER},
    {"cmp", OPCODE_CMP, FUNCT_NONE, 2, ADD_IMMEDIATE | ADD_DIRECT | ADD_REGISTER, ADD_IMMEDIATE | ADD_DIRECT | ADD_REGISTER},
    {"add", OPCODE_ADD, FUNCT_ADD, 2, ADD_IMMEDIATE | ADD_DIRECT | ADD_REGISTER, ADD_DIRECT | ADD_REGISTER},
    {"sub", OPCODE_SUB, FUNCT_SUB, 2, ADD_IMMEDIATE | ADD_DIRECT | ADD_REGISTER, ADD_DIRECT | ADD_REGISTER},
    {"lea", OPCODE_LEA, FUNCT_NONE, 2, ADD_DIRECT, ADD_DIRECT | ADD_REGISTER},
    {"clr", OPCODE_CLR, FUNCT_CLR, 1, 0, ADD_DIRECT | ADD_REGISTER},
    {"not", OPCODE_NOT, FUNCT_NOT, 1, 0, ADD_DIRECT | ADD_REGISTER},
    {"inc", OPCODE_INC, FUNCT_INC, 1, 0, ADD_DIRECT | ADD_REGISTER},
    {"dec", OPCODE_DEC, FUNCT_DEC, 1, 0, ADD_DIRECT | ADD_REGISTER},
    {"jmp", OPCODE_JMP, FUNCT_JMP, 1, 0, ADD_DIRECT | ADD_RELATIVE},
    {"bne", OPCODE_BNE, FUNCT_BNE, 1, 0, ADD_DIRECT | ADD_RELATIVE},
    {"jsr", OPCODE_JSR, FUNCT_JSR, 1, 0, ADD_DIRECT | ADD_RELATIVE},
    {"red", OPCODE_RED, FUNCT_NONE, 1, 0, ADD_DIRECT | ADD_REGISTER},
    {"prn", OPCODE_PRN, FUNCT_NONE, 1, 0, ADD_IMMEDIATE | ADD_DIRECT | ADD_REGISTER},
    {"rts", OPCODE_RTS, FUNCT_NONE, 0, 0, 0},
    {"stop", OPCODE_STOP, FUNCT_NONE, 0, 0, 0}
};

/* Register set LUT */
RegisterInfo register_table[] = {
    {"r0", REG_0}, {"r1", REG_1}, {"r2", REG_2}, {"r3", REG_3},
    {"r4", REG_4}, {"r5", REG_5}, {"r6", REG_6}, {"r7", REG_7}
    };

/* Directive set LUT */
char *directives[] = {
    "data", "string", "entry", "extern"
};

/* Addressing modes string LUT */
char *addressing_modes[] = {
    "None", "immediate", "direct", "relative", "register"
};

/* Special characters LUT */
char *special_chars[] = {
    ",", ".", ":", "&", "#", "\""
};


int is_instruction(StringView sv) 
{
    size_t i;

    if (!sv.str)
        return 0;

    for (i = 0; i < TABLE_SIZE(InstructionInfo, instruction_set); i++) 
        if (strncmp(sv.str, instruction_set[i].name, sv.length) == 0)
            return 1;

    return 0;
}

char *get_addressing_mode_str(AddressingMode mode) 
{
    switch(mode)
    {
        case ADD_MOD_NONE:
            return addressing_modes[0]; 

        case ADD_MOD_IMMEDIATE:
            return addressing_modes[1];

        case ADD_MOD_DIRECT:
            return addressing_modes[2];

        case ADD_MOD_RELATIVE:
            return addressing_modes[3];

        case ADD_MOD_REGISTER:
            return addressing_modes[4];
            
        default:
            return NULL;
    }
}

InstructionInfo *find_instruction(char *name) 
{
    size_t i;
    
    if (!name)
        return NULL;

    for (i = 0; i < TABLE_SIZE(InstructionInfo, instruction_set); i++)
        if (strcmp(name, instruction_set[i].name) == 0)
            return &instruction_set[i];

    return NULL;
}

int is_register(StringView sv) 
{
    size_t i;

    if (!sv.str)
        return 0;

    for (i = 0; i < TABLE_SIZE(RegisterInfo, register_table); i++) 
        if (strncmp(sv.str, register_table[i].name, sv.length) == 0)
            return 1;

    return 0;
}

RegisterInfo *find_register(char *name) 
{
    size_t i;
    
    if (!name)
        return NULL;

    for (i = 0; i < TABLE_SIZE(RegisterInfo, register_table); i++)
        if (strcmp(name, register_table[i].name) == 0)
            return &register_table[i];

    return NULL;
}

Directive is_directive(StringView sv) 
{
    size_t i;

    if (!sv.str)
        return 0;

    for (i = 0; i < TABLE_SIZE(char *, directives); i++) 
        if (strncmp(sv.str, directives[i], sv.length) == 0)
            return i + 1;

    return 0;
}   

int is_special_char(StringView sv) 
{
    size_t i;

    if (!sv.str)
        return 0;

    for (i = 0; i < TABLE_SIZE(char *, special_chars); i++) 
        if (strncmp(sv.str, special_chars[i], sv.length) == 0)
            return 1;

    return 0;
}
        
