/**
 * @file isa.h
 * @brief Header file for the instruction set architecture (ISA) of the assembler.
 * @details This file contains definitions, enums, and function prototypes related to the ISA of the assembler.
 *          It includes definitions for opcodes, addressing modes, function codes, Masks, information about the machine,
 *          and basically all the information that need to be stored.
 */
#ifndef ISA_H
#define ISA_H

#include "./string_view.h"

#define INT21_MIN   -1.048576e6
#define INT21_MAX    1.048575e6
#define UINT24_MAX  1.6777215e7
#define DEC_BASE    10

/* Word bit setup */
#define WORD_BITS 24
#define OPCODE_POS 18
#define SRC_ADD_MODE_POS 16
#define SRC_OPERAND_POS 13
#define DST_ADD_MODE_POS 11
#define DST_OPERAND_POS 8
#define FUNCT_POS 3
#define ARE_POS 0

/* Masks for extracting fields from the instruction word */
#define OPCODE_MASK (0x3F << OPCODE_POS)
#define SRC_ADD_MODE_MASK (0x3 << SRC_ADD_MODE_POS)
#define SRC_OPERAND_MASK (0x7 << SRC_OPERAND_POS)
#define DST_ADD_MODE_MASK (0x3 << DST_ADD_MODE_POS)
#define DST_OPERAND_MASK (0x7 << DST_OPERAND_POS)
#define FUNCT_MASK (0x1F << FUNCT_POS)
#define ARE_MASK (0x7 << ARE_POS)
#define WORD_MASK (0xFFFFFF)
#define IMM_MASK (0xFFFFF8) 
#define IMM_SHIFT 3

/* A R E */
#define ARE_ABSOLUTE  4
#define ARE_EXTERNAL  1
#define ARE_RELOCATABLE 2

/* Opcpdes */
typedef enum {
    OPCODE_MOV = 0,
    OPCODE_CMP = 1,
    OPCODE_ADD = 2,
    OPCODE_SUB = 2,
    OPCODE_LEA = 4,
    OPCODE_CLR = 5,
    OPCODE_NOT = 5,
    OPCODE_INC = 5,
    OPCODE_DEC = 5,
    OPCODE_JMP = 9,
    OPCODE_BNE = 9,
    OPCODE_JSR = 9,
    OPCODE_RED = 12,
    OPCODE_PRN = 13,
    OPCODE_RTS = 14,
    OPCODE_STOP = 15
} Opcode;

/* Function codes */
typedef enum {
    FUNCT_NONE = 0,
    FUNCT_ADD = 1,
    FUNCT_SUB = 2,
    FUNCT_CLR = 1,
    FUNCT_NOT = 2,
    FUNCT_INC = 3,
    FUNCT_DEC = 4,
    FUNCT_JMP = 1,
    FUNCT_BNE = 2,
    FUNCT_JSR = 3
} Funct;

/* Addressing modes */
typedef enum {
    /* Addressing modes for validation */
    ADD_IMMEDIATE   = 1 << 0,
    ADD_DIRECT      = 1 << 1,
    ADD_RELATIVE    = 1 << 2,
    ADD_REGISTER    = 1 << 3,

    /* Addressing modes for instruction encoding */
    ADD_MOD_NONE      = -1,
    ADD_MOD_IMMEDIATE = 0,
    ADD_MOD_DIRECT    = 1,
    ADD_MOD_RELATIVE  = 2,
    ADD_MOD_REGISTER  = 3
} AddressingMode;

/* Operand types */
typedef enum {
    OPERAND_RS,
    OPERAND_RT
} OperandType;

/* Register numbers */
typedef enum {
    REG_0,
    REG_1,
    REG_2,
    REG_3,
    REG_4,
    REG_5,
    REG_6,
    REG_7
} Register;

/* Directives */
typedef enum {
    DIR_DATA,
    DIR_STRING,
    DIR_ENTRY,
    DIR_EXTERN
} Directive;

/* Instruction information structure */
typedef struct {
    char *name;                             /* Instruction name */
    Opcode opcode;                          /* Opcode value */
    Funct funct;                            /* Function code */
    int num_operands;                       /* Number of operands */
    AddressingMode allowd_src_add_mode;     /* Allowed addressing modes for source operand */   
    AddressingMode allowd_dst_add_mode;     /* Allowed addressing modes for destination operand */
} InstructionInfo;

/* Register information structure */
typedef struct {
    char *name;                             /* Register name */
    Register reg;                           /* Register number */
} RegisterInfo;

/* Directive information structure */
typedef struct {
    char *name;                             /* Directive name */
    Directive dir;                          /* Directive type */
} DirectiveInfo;

/* Global variables */
/* LUT's for all the above structures */
extern InstructionInfo instruction_set[];
extern RegisterInfo register_table[];
extern DirectiveInfo directive_table[];
extern char *AddressingMode_names[];
extern char *special_chars[];

/**
 * @brief Checks if the given string is a valid instruction.
 * @param sv The string to check.
 * @return 1 if the string is a valid instruction, i.e exists in the instruction_set[] and 0 otherwise.
 */
int is_instruction(StringView sv);

/**
 * @brief Checks if the given string is a valid register.
 * @param sv The string to check.
 * @return 1 if the string is a valid register, i.e exists in the register_table[] and 0 otherwise.
 */
int is_register(StringView sv);

/**
 * @brief Checks if the given string is a valid directive.
 * @param sv The string to check.
 * @return 1 if the string is a valid directive, i.e exists in the directive_table[] and 0 otherwise.
 */
Directive is_directive(StringView sv);

/**
 * @brief Checks if the given string is a special character.
 * @param sv The string to check.
 * @return 1 if the string is a special character, i.e exists in the special_chars[] and 0 otherwise.
 */
int is_special_char(StringView sv);

/**
 * @brief Checks if the given string is the name of an instruction.
 * @param sv The string to check.
 * @return The instruction information if the string is a valid instruction, NULL otherwise.
 */
InstructionInfo *find_instruction(char *name);

/**
 * @brief Checks if the given string is the name of a register.
 * @param sv The string to check.
 * @return The register information if the string is a valid register, NULL otherwise.
 */
RegisterInfo *find_register(char *name);

/**
 * @brief Returns the string representation of the given addressing mode.
 * @param mode The addressing mode to convert.
 * @return A string representation of the addressing mode.
 */
char *get_addressing_mode_str(AddressingMode mode);

#endif /* ISA_H */