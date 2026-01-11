/**
 * @file assembler.h
 * @brief Header file for the assembler module.
 * @details This file contains the definition of the AssemblerContext structure and function prototypes for
 *          initializing, destroying, and using the assembler context.
 */

#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "../data_structures/array_list.h"
#include "../data_structures/hash_map.h"


#define INITIAL_IC 100

/* Assembler context structure */

typedef struct {
    ArrayList *errors;                                  /* List of errors encountered during assembly */
    const char *filename;                               /* Name of the source file being assembled */
    const char *ir_filename;                            /* Name of the intermediate representation file (.am)*/
    size_t line_number;                                 /* Current line number in the source file */
    ArrayList *preprocessed_lines;                      /* List of preprocessed lines */
    ArrayList *tokens;                                  /* List of tokens generated from the source file */
    HashMap *symbol_table;                              /* Symbol table for storing labels and their addresses */
    ArrayList *code_img;                                /* List of code image words */
    ArrayList *data_img;                                /* List of data image words */
    ArrayList *entries;                                 /* List of entry references */
    ArrayList *externals;                               /* List of external references */
    ArrayList *entry_names;                             /* List of entry names */
    ArrayList *extern_names;                            /* List of external names */
    int IC;                                             /* Instruction Counter */
    int DC;                                             /* Data Counter */                    
} AssemblerContext;


/**
 * @brief Assembles the given files.
 * @param files Array of file names to assemble.
 * @param file_count Number of files to assemble.
 * @note This function processes each file, performing preprocessing, first pass,
 *       and second pass assembly. It generates the output files (.ob, .ent, .ext) if assembly is successful.
 */
void assemble(char **files, int file_count);


/**
 * @brief Initializes the assembler context.
 * @param ctx Pointer to the AssemblerContext to initialize.
 * @param filename Name of the source file to assemble.
 * @note This function allocates memory for the context and initializes its fields.
 */

void asm_ctx_init(AssemblerContext *ctx, const char *filename);

/**
 * @brief Destroys the assembler context and frees allocated memory.
 * @param ctx Pointer to the AssemblerContext to destroy.
 * @note This function frees all the resources associated with the context.
 */
void asm_ctx_destroy(AssemblerContext *ctx);


#endif /* ASSEMBLER_H */