/**
 * @file file_io.c
 * @brief Implementation of file input/output operations.
 * @details This file contains functions for reading lines from a file,
 *         writing encoded words and symbols, and creating output files with
 *        different extensions.
 */

#include <stdio.h>
#include <string.h>

#include "./error.h"
#include "./file_io.h"
#include "./util.h"
#include "./code_gen.h"
#include "../main/assembler.h"
#include "../assembly/first_pass.h"

ArrayList *file_read_lines(const char *filename)
{
    FILE *file = NULL;
    ArrayList *lines = NULL;
    int c;
    long pos;
    char *full_path = NULL;
    size_t full_path_len = 0;

    if (!filename) 
        return NULL;

    /* add extension */
    full_path_len = strlen(filename) + strlen(ASM_EXT) + 1;

    /* Complete the full path with extension */
    full_path = (char *)MALLOC(full_path_len);
    if (!full_path)
        return NULL;
    
    strcpy(full_path, filename);
    strcat(full_path, ASM_EXT);

    file = fopen(full_path, "r");
    if (!file)
    {   
        error_report(NULL, ERR_FILE_OPEN, "Failed to open file: %s", full_path);
        FREE(full_path);
        return NULL;
    } 
    
    lines = array_list_create(free_string);
    
    if (!lines) 
    {
        fclose(file);
        FREE(full_path);
        return NULL;
    }


    for(;;) 
    {
        size_t i = 0;
        size_t len = 0;
        char *line = NULL;
        
        /* Mark the current position */
        pos = ftell(file);

        /* Check if we reached EOF */
        if (pos == -1) 
        {
            array_list_destroy(lines);
            fclose(file);
            FREE(full_path);
            return NULL;
        }
        
        /* Count characters until newline or EOF */
        while ((c = fgetc(file)) != EOF && c != '\n') 
            len++;
        
        /* If reached EOF with no characters - break */
        if (c == EOF && len == 0)
            break;
            
        /* Allocate exactly the right amount of memory (plus null terminator) */
        line = (char *)MALLOC(len + 1);
        if (!line)  
        {
            array_list_destroy(lines);
            fclose(file);
            FREE(full_path);
            return NULL;
        }

        /* Go back to the start of the line */
        if (fseek(file, pos, SEEK_SET) != 0)  
        {
            FREE(line);
            array_list_destroy(lines);
            fclose(file);
            FREE(full_path);
            return NULL;
        }
        
        /* Read the line into the allocated buffer */
        while (i < len && (c = fgetc(file)) != EOF) 
            line[i++] = (char)c;
        
        line[len] = '\0';
        
        /* Skip the newline that was already read */
        if (c != EOF) 
            fgetc(file);
        
        /* Add line to the list */
        array_list_append(lines, line);
    }

    fclose(file);
    FREE(full_path);
    return lines;
}

void write_word_to_file(FILE *file, void *item)
{
    Word *word = (Word *)item;
    fprintf(file, "%07d %06x\n", word->address, word->value);
}

void write_symbol_to_file(FILE *file, void *item)
{
    Symbol *symbol = (Symbol *)item;
    fprintf(file, "%.*s %07u\n", (int)symbol->sv.length, symbol->sv.str, symbol->address);
}

void create_output_file(AssemblerContext *ctx, const char *extension, ArrayList *data_list, void (*write_func)(FILE*, void*))
{
    FILE *file = NULL;
    char *output_filename = NULL;
    char *ext = NULL;
    size_t i = 0;
    size_t ext_len = strlen(extension);

    if (!ctx || !ctx->filename || !data_list || array_list_size(data_list) == 0|| !write_func)
        return;

    /* Allocate memory for filename with extension */
    output_filename = (char *)MALLOC(strlen(ctx->filename) + ext_len + 1);
    if (!output_filename)
        return;

    /* Copy base filename and remove original extension */
    output_filename = strcpy(output_filename, ctx->filename);
    ext = strrchr(output_filename, '.');
    if (ext)
        *ext = '\0';

    /* Add new extension */
    strcat(output_filename, extension);

    /* Open file for writing */
    file = fopen(output_filename, "w");
    if (!file)
    {
        FREE(output_filename);
        error_report(NULL, ERR_FILE_OPEN, "Failed to open file: %s", output_filename);
        return;
    }

    /* Write each item using the provided function */
    for (i = 0; i < array_list_size(data_list); i++)
    {
        void *item = array_list_get(data_list, i);
        write_func(file, item);
    }

    fclose(file);
    FREE(output_filename);
}

void generate_output(AssemblerContext *ctx, int mode)
{
    if (!ctx || !ctx->filename)
        return;

    /* Generate IR files (.am) */
    if (mode == 0)
    {
        size_t i;
        char *output_filename = NULL;
        char *ext = NULL;
        FILE *file = NULL;

        /* Create IR file */
        output_filename = (char *)MALLOC(strlen(ctx->filename) + 4);
        if (!output_filename)
            return;

        output_filename = strcpy(output_filename, ctx->filename);
        ext = strrchr(output_filename, '.');
        if (ext)
            *ext = '\0';

        strcat(output_filename, IR_EXT);

        file = fopen(output_filename, "w");
        if (!file)
        {
            error_report(NULL, ERR_FILE_OPEN, "Failed to open file: %s", output_filename);
            FREE(output_filename);
            return;
        }

        for (i = 0; i < array_list_size(ctx->preprocessed_lines); i++)
        {
            char *line = (char *)array_list_get(ctx->preprocessed_lines, i);
            fprintf(file, "%s\n", line);
        }

        fclose(file);
        ctx->ir_filename = output_filename;
    }

    /* Generate object file (.ob) with code and data */
    if (array_list_size(ctx->code_img))
    {
        size_t i;
        FILE *file = NULL;
        char *output_filename = NULL;
        char *ext = NULL;

        /* Create object file */
        output_filename = (char *)MALLOC(strlen(ctx->filename) + 4);
        if (!output_filename)
            return;

        output_filename = strcpy(output_filename, ctx->filename);
        ext = strrchr(output_filename, '.');
        if (ext)
            *ext = '\0';

        strcat(output_filename, OBJ_EXT);

        file = fopen(output_filename, "w");
        if (!file)
        {
            error_report(NULL, ERR_FILE_OPEN, "Failed to open file: %s", output_filename);
            FREE(output_filename);
            return;
        }

        fprintf(file, "     %d %d\n", ctx->IC - INITIAL_IC - ctx->DC, ctx->DC);
        /* For object files, write both code and data sections */
        for (i = 0; i < array_list_size(ctx->code_img); i++)
        {
            Word *word = (Word *)array_list_get(ctx->code_img, i);
            write_word_to_file(file, word);
        }

        for (i = 0; i < array_list_size(ctx->data_img); i++)
        {
            Word *word = (Word *)array_list_get(ctx->data_img, i);
            write_word_to_file(file, word);
        }

        fclose(file);
        FREE(output_filename);
    }

    /* Generate entries file (.ent) */
    if (array_list_size(ctx->entries))
        create_output_file(ctx, ENT_EXT, ctx->entries, write_symbol_to_file);
    

    /* Generate externals file (.ext) */
    if (array_list_size(ctx->externals))
        create_output_file(ctx, EXT_EXT, ctx->externals, write_symbol_to_file);
}
