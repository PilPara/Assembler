/**
 * @file code_gen.h
 * @brief Header file for code generation functions.
 * @details This file contains function prototypes for encoding instructions and directives into machine code.
 *         It includes functions for creating and destroying words, encoding operands, and manipulating fields in words.
 *         The code generation functions are used in the assembler to convert high-level instructions into machine code.
 */

#ifndef CODE_GEN_H
#define CODE_GEN_H

#include "../common/isa.h"
#include "../common/parser.h"

/* Word structure */
/* This structure represents A 24-bit word used in the assembler. */
/* It contains the value of the word and its address in memory. */
/* The value is stored as an unsigned integer which is larger than 24 bits and masked to 24 bits. */
/* 8 upper bits will always be 0. */

typedef struct {
    unsigned int value;
    int address;
} Word;

/**
 * @brief Creates a new word with the specified value.
 * @param value The value of the word.
 * @return Pointer to the newly created word.
 * @note The caller is responsible for freeing the word using word_destroy().
 */
Word *word_create(unsigned int value);

/**
 * @brief Destroys a word and frees its memory.
 * @param word Pointer to the word to destroy.
 * @note Frees the word structure and its contents.
 */
void word_destroy(void *word);

/**
 * @brief Encodes a ParsedInstruction into between 1 and 3 machine words.
 * @param instruction Pointer to the parsed instruction to encode.
 * @param ctx Pointer to the assembler context.
 * @param IC Pointer to the instruction counter.
 * @note This function encodes the instruction and its operands into machine code.
 */

void encode_instruction(ParsedInstruction *instruction, AssemblerContext *ctx, int *IC);

/**
 * @brief Encodes a ParsedDirective into machine words.
 * @param directive Pointer to the parsed directive to encode.
 * @param ctx Pointer to the assembler context.
 * @param IC Pointer to the instruction counter.
 * @param DC Pointer to the data counter. 
 */
void encode_data(ParsedDirective *directive, AssemblerContext *ctx, int *IC, int *DC);

/**
 * @brief Encodes an operand into a word.
 * @param operand Pointer to the operand token to encode.
 * @param word Pointer to the word to modify.
 * @param is_source Flag indicating if the operand is a source operand.
 * @param is_relative Flag indicating if the operand uses relative addressing.
 * @note This function sets the appropriate addressing mode and operand value in the word.
 */

void encode_operand(Token *operand, Word *word, int is_source, int is_relative);

/**
 * @brief Encodes the first word that a parsed instruction will generate.
 * @param instruction Pointer to the parsed instruction to encode.
 * @param ctx Pointer to the assembler context.
 * @param IC Pointer to the instruction counter.
 */
void encode_first_word(ParsedInstruction *instruction, AssemblerContext *ctx, int *IC);


/**
 * @brief Encodes an operand into a word with additional information.
 * @param operand Pointer to the operand token to encode.
 */
void encode_operand_extra(Token *operand, AssemblerContext *ctx, Word *word, AddressingMode add_mode, int current_IC);


/* Field manipulation functions */
/**
 * @brief Sets the opcode field of a word.
 * @param word Pointer to the word to modify.
 * @param opcode The opcode value to set.
 */ 
void word_set_opcode(Word *word, int opcode);

/**
 * @brief Sets the source addressing mode field of a word.
 * @param word Pointer to the word to modify.
 * @param mode The addressing mode value to set.
 */
void word_set_rs_add_mod(Word *word, unsigned int mode);

/**
 * @brief Sets the source operand field of a word.
 * @param word Pointer to the word to modify.
 * @param operand The source operand value to set.
 */
void word_set_rs_operand(Word *word, unsigned int operand);

/**
 * @brief Sets the destination addressing mode field of a word.
 * @param word Pointer to the word to modify.
 * @param mode The addressing mode value to set.
 */
void word_set_rt_add_mod(Word *word, unsigned int mode);

/**
 * @brief Sets the destination operand field of a word.
 * @param word Pointer to the word to modify.
 * @param operand The destination operand value to set.
 */
void word_set_rt_operand(Word *word, unsigned int operand);

/**
 * @brief Sets the function code field of a word.
 * @param word Pointer to the word to modify.
 * @param funct The function code value to set.
 */
void word_set_funct(Word *word, unsigned int funct);

/**
 * @brief Sets the ARE field of a word.
 * @param word Pointer to the word to modify.
 * @param are The ARE value to set.
 * @note The ARE field indicates the addressing mode of the word.
 * @note The ARE field is used to indicate whether the word is absolute, external, or relative.
 */
void word_set_are(Word *word, unsigned int are);

/**
 * @brief Encodes an immediate value from an instruction statement into a word.
 * @param word Pointer to the word to modify.
 * @param immediate The immediate value to encode. 
 */

void word_from_immediate(Word *word, int immediate);

/**
 * @brief Encodes an immediate value from a directive statement into a word.
 * @param word Pointer to the word to modify.
 * @param immediate The immediate value to encode. 
 */

void data_from_immediate(Word *word, int immediate);

#endif /* CODE_GEN_H */