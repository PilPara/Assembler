/**
 * @file parser.h   
 * @brief Header file for the parser module.
 * @details This file contains the definitions of structures and function prototypes for parsing instructions and directives.
 *          It includes functions for parsing, validating, and managing parsed instructions and directives.
 *          The parser is responsible for converting a list of tokens into a structured representation that can be used for further processing.
 *          The parser also includes functions for validating operands and directives, as well as checking the types of statements.
 */

#ifndef PARSER_H
#define PARSER_H

#include "./lexer.h"
#include "./isa.h"

/* Parsed Instruction structure */
typedef struct {
    Token *label;                               /* Label of the instruction - optional */
    Token *instruction;                         /* Instruction token */
    Token *rs;                                  /* Source operand token */  
    AddressingMode rs_add_mode;                 /* Source operand addressing mode */
    Token *rt;                                  /* Destination operand token */
    AddressingMode rt_add_mode;                 /* Destination operand addressing mode */
    int operand_count;                          /* Number of operands */
    int code_word_count;                        /* Number of code words generated when encoding */
    ArrayList *tokens;                          /* Original tokens for the instruction */
} ParsedInstruction;

/* Parsed Directive structure */
typedef struct {    
    Token *label;                               /* Label of the directive - optional */
    Token *directive;                           /* Directive token */
    ArrayList *tokens;                          /* Original tokens for the directive */
    int code_word_count;                        /* Number of code words generated when encoding */
} ParsedDirective;

/* Function prototypes */
/* Instruction functions */
/**
 * @brief Initializes a parsed instruction structure.
 * @param instruction Pointer to the ParsedInstruction to initialize.
 * @note This function sets the default values for the fields in the structure.
 * @note No need for a destroy function as the structure is not dynamically allocated, nor does it own any resources.
 */
void parsed_instruction_init(ParsedInstruction *instruction);

/**
 * @brief Parses an instruction from a list of tokens to its parsed representation.
 * @param instruction Pointer to the ParsedInstruction to fill.
 * @param tokens Pointer to the list of tokens to parse.
 * @param ctx Pointer to the assembler context.
 * @param mode The mode of parsing (1 for first pass, 2 for second pass (first pass contains error handling)).
 */

void parse_instruction(ParsedInstruction *instruction, ArrayList *tokens, AssemblerContext *ctx, int mode);


/* Directive functions */
/**
 * @brief Initializes a parsed directive structure.
 * @param directive Pointer to the ParsedDirective to initialize.
 * @note This function sets the default values for the fields in the structure.
 */
void parsed_directive_init(ParsedDirective *directive);

/**
 * @brief Destroys a parsed directive structure and frees its memory.
 * @param directive Pointer to the ParsedDirective to destroy.
 * @note This function frees the directive structure and its contents.
 */
void parsed_directive_destroy(void *directive);

/**
 * @brief Parses a directive from a list of tokens to its parsed representation.
 * @param directive Pointer to the ParsedDirective to fill.
 * @param tokens Pointer to the list of tokens to parse.
 * @param ctx Pointer to the assembler context.
 */
void parse_directive(ParsedDirective *directive, ArrayList *tokens, AssemblerContext *ctx);


/* Helper functions */
/**
 * @brief Checks if to token is an operand.
 * @param token Pointer to the token to check.
 * @return 1 if the token is an operand, 0 otherwise.
 */
int is_operand(Token *token);

/**
 * @brief Calculates the number of operands in an instruction.
 * @param token Pointer to the token to check.
 * @return The number of operands in the instruction.
 */
int count_operands(ArrayList *tokens);

/**
 * @brief Calculates the number of code words a given instruction will generate.
 * @param token Pointer to the token to check.
 */
int calculate_code_word(TokenType type);



/**
 * @brief Returns the addressing mode of a given token.
 * @param token Pointer to the token to check.
 * @param is_amper Flag indicating if the token is preceded by an ampersand (&).
 * @return The addressing mode of the token.
 */

AddressingMode get_addressing_mode(Token *token, int is_amper);


/**
 * @brief Checks if an array list of tokens is an instruction statement.
 * @param tokens Pointer to the list of tokens to check.
 * @return 1 if the tokens represent an instruction statement, 0 otherwise.
 */
int is_instruction_statement(ArrayList *tokens);

 /**
 * @brief Checks if an array list of tokens is a directive statement.
 * @param tokens Pointer to the list of tokens to check.
 * @return 1 if the tokens represent a directive statement, 0 otherwise.
 */
int is_directive_statement(ArrayList *tokens);

int is_extern_statement(ArrayList *tokens);
/**
 * @brief Checks if an array list of tokens is an entry statement.
 * @param tokens Pointer to the list of tokens to check.
 * @return 1 if the tokens represent an entry statement, 0 otherwise.
 */
int is_entry_statement(ArrayList *tokens);

/**
 * @brief Checks if an array list of tokens is a label statement.
 * @param tokens Pointer to the list of tokens to check.
 * @return 1 if the tokens represent a label statement, 0 otherwise.
 */
int is_label_statement(ArrayList *tokens);

/* Validation functions */
/**
 * @brief Validates an immediate value token.
 * @param token Pointer to the token to validate.
 * @param ctx Pointer to the assembler context.
 * @return 1 if the token is a valid immediate value, 0 otherwise.
 */ 

int validate_immediate(Token *token, AssemblerContext *ctx);

/**
 * @brief Validates a data value token.
 * @param token Pointer to the token to validate.
 * @param ctx Pointer to the assembler context.
 * @return 1 if the token is a valid data value, 0 otherwise.
 */
int validate_data_value(Token *token, AssemblerContext *ctx);

/**
 * @brief Validates an instruction token.
 * @param instruction Pointer to the parsed instruction to validate.
 * @param ctx Pointer to the assembler context.
 * @return 1 if the instruction is valid, 0 otherwise.
 * @note This function checks the number of operands, the addressing modes, and the instruction name.
 */
int validate_instruction(ParsedInstruction *instruction, AssemblerContext *ctx);

#endif /* PARSER_H */