/**
 * @file file_io.h
 * @brief Header file for file input/output operations.
 * @details This file contains function prototypes for reading and writing files,
 *          including reading lines from a file, writing encoded words and symbols,
 *          and creating output files with different extensions.
 *          All file handling operations are done from this module, since all the functions here
 *          contain error checking and reporting.
 */

#ifndef FILE_IO_H
#define FILE_IO_H

#include <stdio.h>
#include "../data_structures/array_list.h"
#include "../main/assembler.h"

/* File extensions for different output files */
#define ASM_EXT ".as"
#define IR_EXT ".am"
#define OBJ_EXT ".ob"
#define ENT_EXT ".ent"
#define EXT_EXT ".ext"

/* Function pointer type for writing different types of data to files */
typedef void (*WriteFunction) (FILE *, void *);

/**
 * @brief Reads a file and returns its lines as an ArrayList of char*.
 * @param filename The name of the file to read.
 * @return An ArrayList containing the lines of the file. Each line is a char*.
 * @note The caller is responsible for freeing the ArrayList and its contents.
 */

ArrayList *file_read_lines(const char *filename);

/**
 * @brief Writes an encoded word to a file.
 * @param file The file to write to.
 * @param item The word to write.
 * @note The word is written in the format: <address> <value>.
 */
void write_word_to_file(FILE *file, void *item);

/**
 * @brief Writes a symbol to a file.
 * @param file The file to write to.
 * @param item The symbol to write.
 * @note The symbol is written in the format: <symbol_name> <address>.
 */
void write_symbol_to_file(FILE *file, void *item);

/**
 * @brief Creates an output file with the specified extension and writes data to it.
 * @param ctx The assembler context.
 * @param extension The file extension (e.g., ".am",".ob", ".ent", ".ext").
 * @param data_list The list of data to write to the file.
 * @param write_func The function to write the data to the file.
 * @note The function will create a new file with the specified extension and write the data to it.
 */
void create_output_file(AssemblerContext *ctx, const char *extension, ArrayList *data_list, WriteFunction write_func);

/**
 * @brief Generates output files based on the assembler context.
 * @param ctx The assembler context.
 * @param mode The mode of output generation (0 for preprocessing, 2 for final output).
 */
void generate_output(AssemblerContext *ctx, int mode);

#endif /* FILE_IO_H */