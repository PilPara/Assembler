/**
 * @file assembler.c
 * @brief Main file for the assembler program.
 * @details This file contains the entry point to the program and the assembler context initialization and destruction functions.
 *          It also includes the assembly process function which connects all the assembly phases.
 */


#include <stdio.h>
#include <string.h>

#include "./assembler.h"
#include "../assembly/preprocessor.h"   
#include "../assembly/first_pass.h"   
#include "../assembly/second_pass.h"   
#include "../common/error.h"
#include "../common/file_io.h"
#include "../common/util.h"
#include "../common/code_gen.h"

/* Macros for easy initialization and destruction of array lists for the assembler context */
#define INIT_LIST(field, free_func, error_msg)              \
    ctx->field = array_list_create(free_func);              \
    if (!ctx->field)                                        \
    {                                                       \
        fprintf(stderr, error_msg);                         \
        asm_ctx_destroy(ctx);                               \
        return;                                             \
    }

#define DESTROY_IF_EXISTS(field)                            \
        if (ctx->field)                                     \
        {                                                   \
            array_list_destroy(ctx->field);                 \
            ctx->field = NULL;                              \
        }


int main(int argc, char **argv) 
{
    /* Check Command line arguments */
    if (argc < 2) 
    {
        fprintf(stderr, "Usage <%s> - At least one file name must be provided as a command line argument\n", argv[0]);  
        return 1;
    }

    /* Assemble the files */
    assemble(argv + 1, argc - 1);

    return 0;
}


void assemble(char **files, int file_count)
{
    int i;

    for (i = 0; i < file_count; i++)
    {
        AssemblerContext ctx = {0};
        asm_ctx_init(&ctx, files[i]);

        /* Preprocess the file */
        preprocess(&ctx);
        
        /* Check for errors and if there are any, report them and move to next file if exists */
        if (array_list_size(ctx.errors) > 0) 
        {
            error_report_all(ctx.errors);
            asm_ctx_destroy(&ctx);
            continue;
        }
        
        /* First pass - build symbol table and calculate IC and DC */
        first_pass(&ctx);

        if (array_list_size(ctx.errors) > 0) 
        {
            error_report_all(ctx.errors);
            asm_ctx_destroy(&ctx);
            continue;
        }

        /* Second pass - encode instructions and directives */
        second_pass(&ctx);

        if (array_list_size(ctx.errors) > 0) 
        {
            error_report_all(ctx.errors);
            asm_ctx_destroy(&ctx);
            continue;
        }
        
        asm_ctx_destroy(&ctx);
    }
}

void asm_ctx_init(AssemblerContext *ctx, const char *filename)
{
    if (!ctx)
        return;
    
    /* Initialize all properties to NULL/0 first */
    memset(ctx, 0, sizeof(AssemblerContext));
    ctx->filename = filename;
    ctx->ir_filename = NULL;
    ctx->IC = INITIAL_IC;
    
    /* Initialize all the array lists */
    INIT_LIST(errors, error_destroy, "Failed to create error list\n");
    INIT_LIST(preprocessed_lines, free_string, "Failed to create preprocessed lines list\n");
    INIT_LIST(tokens, token_destroy, "Failed to create tokens list\n");
    INIT_LIST(code_img, word_destroy, "Failed to create code image list\n");
    INIT_LIST(data_img, word_destroy, "Failed to create data image list\n");
    INIT_LIST(entries, symbol_destroy, "Failed to create entries list\n");
    INIT_LIST(externals, symbol_destroy, "Failed to create externals list\n");
    INIT_LIST(entry_names, symbol_destroy, "Failed to create entry names list\n");
    INIT_LIST(extern_names, NULL, "Failed to create extern names list\n");
    
    /* Initialize the symbol table */
    ctx->symbol_table = hash_map_create(free_string);
    if (!ctx->symbol_table) 
    {
        fprintf(stderr, "Failed to create symbol table\n");
        asm_ctx_destroy(ctx);
        return;
    }
}

void asm_ctx_destroy(AssemblerContext *ctx)
{
    if (!ctx)
        return;

    /* Destroy all the array lists */
    DESTROY_IF_EXISTS(errors);
    DESTROY_IF_EXISTS(preprocessed_lines);
    DESTROY_IF_EXISTS(tokens);
    DESTROY_IF_EXISTS(code_img);
    DESTROY_IF_EXISTS(data_img);
    DESTROY_IF_EXISTS(entries);
    DESTROY_IF_EXISTS(externals);
    DESTROY_IF_EXISTS(entry_names);
    DESTROY_IF_EXISTS(extern_names);
    
    /* Destroy the symbol table */
    if (ctx->symbol_table) 
    {
        hash_map_destroy(ctx->symbol_table);
        ctx->symbol_table = NULL;
    }

    /* Free IR filename */
    if (ctx->ir_filename) 
        FREE(ctx->ir_filename);
    
}