/**
 * @file preprocessor.c
 * @brief Implementation of the preprocessor for the assembler.
 * @details This file contains functions for initializing, destroying,
 *          and using the preprocessor. The preprocessor handles macro definitions,
 *          removes comments and empty lines from the source code,
 *          and expands macros in the source code.
 *          If no errors are found, the preprocessor generates an IR (.am) file.
 */

#include "../common/error.h"
#include "../common/file_io.h"
#include "../common/isa.h"
#include "../common/util.h"
#include "../common/string_view.h"

#include "../data_structures/array_list.h"
#include "../data_structures/hash_map.h"

#include "./preprocessor.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

void preprocessor_init(Preprocessor *pp)
{
    if (!pp)
        return;

    memset(pp, 0, sizeof(Preprocessor));

    pp->current_macro = array_list_create(free_string);
    if (!pp->current_macro) 
        return;

    pp->macros = hash_map_create(free_string);
    if (!pp->macros) 
    {
        array_list_destroy(pp->current_macro);
        pp->current_macro = NULL;
        return;
    }
    
    pp->state = STATE_DEFAULT;
}

void preprocessor_destroy(Preprocessor *pp)
{
    if (!pp)
        return;

    if (pp->raw_lines) 
    {
        array_list_destroy(pp->raw_lines);
        pp->raw_lines = NULL;
    }

    if (pp->current_macro) 
    {
        array_list_destroy(pp->current_macro);
        pp->current_macro = NULL;
    }

    if (pp->macros) 
    {
        hash_map_destroy(pp->macros);
        pp->macros = NULL;
    }
}

int next_line(Preprocessor *pp, AssemblerContext *ctx)
{
    if (!pp || !pp->raw_lines)
        return 0;

    /* Check if reached EOF */
    if (pp->line_number >= array_list_size(pp->raw_lines))
        return 0;

    pp->current_line = sv_from_str(array_list_get(pp->raw_lines, pp->line_number));

    /* Check for valid line length */
    if (pp->current_line.length > MAX_LINE_LEN) 
        error_report(ctx->errors, ERR_MAX_LINE_LEN, "%s:%d: Line exceeds maximum length of %d characters", 
            ctx->filename, pp->line_number + 1, MAX_LINE_LEN);

    /* Update the line number */
    pp->current_line = sv_trim(pp->current_line);
    pp->line_number++;

    return 1;
}

int is_comment(StringView sv)
{
    return sv_starts_with(sv, ";");
}

int is_empty_line(StringView sv)
{
    return sv.length == 0;
}

int is_macro_def(StringView sv)
{
    return sv_starts_with(sv, MACRO_DEF) && !sv_starts_with(sv, MACRO_END);
}

int is_macro_end(StringView sv)
{
    return sv_starts_with(sv, MACRO_END);
}   

int is_macro_call(Preprocessor *pp, const char *macro_name)
{
    if (!pp || !pp->current_line.str)
        return 0;

    return hash_map_contains(pp->macros, macro_name);
}

int validate_macro(Preprocessor *pp, AssemblerContext *ctx, char *macro_def, char *macro_end, char **macro_name_out)
{
    char *macro_name = NULL;
    StringView sv = {0};
    size_t i = 0;
    char *tmp;

    if (!pp || !ctx || !macro_def || !macro_end || !macro_name_out)
        return 0;

    /* Validate macro definition line */
    macro_name = macro_def + strlen(MACRO_DEF);  /* Skip "mcro" */
    
    if (macro_name[0] != ' ') 
        error_report(ctx->errors, ERR_MCRO_SPACE_MISSING, "%s:%d: Missing space between 'mcro' and macro name: %s", ctx->filename, DEF_LINE(pp), macro_def);

    macro_name++;  /* Skip the space */
    *macro_name_out = macro_name;

    /* Check for extra characters after macro name */
    tmp = macro_name;
    while (*tmp != '\0' && *tmp != ' ')
        tmp++;

    SKIP_WHITESPACE(tmp);
    if (*tmp != '\0') 
        error_report(ctx->errors, ERR_MCRO_DEF_EXTRA, "%s:%d: Macro definition contains extra characters: '%s'", ctx->filename, DEF_LINE(pp), macro_def);

    /* Validate macro end line */
    tmp = macro_end + strlen(MACRO_END);  /* Skip "mcroend" */
    SKIP_WHITESPACE(tmp);
    if (*tmp != '\0') 
        error_report(ctx->errors, ERR_MCRO_DEF_EXTRA, "%s:%lu: Macro end contains extra characters: '%s'", ctx->filename, pp->line_number, macro_end);

    /* Validate macro name */
    sv = sv_trim(sv_from_str(*macro_name_out));
    
    /* Check for empty name and length */
    if (sv.length == 0) 
        error_report(ctx->errors, ERR_MCRO_NAME, "%s:%d: Macro name is empty", ctx->filename, DEF_LINE(pp));

    /* Check for maximum length */
    else if (sv.length > MAX_MCRO_NAME_LEN) 
        error_report(ctx->errors, ERR_MCRO_NAME, "%s:%d: Macro name exceeds maximum length of %d characters", ctx->filename, DEF_LINE(pp), MAX_MCRO_NAME_LEN);

    /* Check for duplicate macros */
    if (hash_map_contains(pp->macros, *macro_name_out)) 
        error_report(ctx->errors, ERR_MCRO_NAME, "%s:%d: Macro name already defined: '%s'", ctx->filename, DEF_LINE(pp), *macro_name_out);

    /* Check for invalid starting character */
    if (isdigit(sv.str[0])) 
        error_report(ctx->errors, ERR_MCRO_NAME, "%s:%d: Macro name cannot start with a digit: '%s'", ctx->filename, DEF_LINE(pp), *macro_name_out);

    if (isupper(sv.str[0])) 
        error_report(ctx->errors, ERR_MCRO_NAME, "%s:%d: Macro name cannot start with an uppercase letter: '%s'", ctx->filename, DEF_LINE(pp), *macro_name_out);

    /* Check for invalid characters */
    for (i = 0; i < sv.length && sv.str[i] != ' '; i++) 
        if (!isalnum(sv.str[i]) && sv.str[i] != '_') 
        {
            error_report(ctx->errors, ERR_MCRO_DEF_EXTRA, "%s:%d: Invalid character in macro name: '%s'", ctx->filename, DEF_LINE(pp), *macro_name_out);
            break;
        }

    /* Check if name conflicts with instruction name */
    if (find_instruction(sv.str) != NULL) 
        error_report(ctx->errors, ERR_MCRO_NAME, "%s:%d: Macro name conflicts with instruction name: '%s'", ctx->filename, DEF_LINE(pp), *macro_name_out);

    /* Check if name conflicts with register name */
    else if (find_register(sv.str) != NULL)
        error_report(ctx->errors, ERR_MCRO_NAME, "%s:%d: Macro name conflicts with register name: '%s'", ctx->filename, DEF_LINE(pp), *macro_name_out);

    
    /* Check if name conflicts with directive name */
    else if (is_directive(sv)) 
        error_report(ctx->errors, ERR_MCRO_NAME, "%s:%d: Macro name conflicts with directive name", ctx->filename, DEF_LINE(pp));

    /* Check if name conflicts with label name */
    else if (sv_ends_with(sv, ":")) 
        error_report(ctx->errors, ERR_MCRO_NAME, "%s:%d: Macro name may conflictswith label name: '%s'", ctx->filename, DEF_LINE(pp), *macro_name_out);

    return array_list_size(ctx->errors) == 0 ? 1 : 0;
}

void define_macro(Preprocessor *pp, AssemblerContext *ctx)
{
    char *macro_def = NULL;
    char *macro_name = NULL;
    char *macro_body = NULL;
    char *macro_end = NULL;
    int is_valid = 0;

    if (!pp || !pp->current_macro || !array_list_size(pp->current_macro))
        return;

    /* Extract first and last lines (definition and end) */
    macro_def = (char *)array_list_remove(pp->current_macro, 0);
    macro_end = (char *)array_list_remove(pp->current_macro, array_list_size(pp->current_macro) - 1);
    
    /* Validate the macro */
    is_valid = validate_macro(pp, ctx, macro_def, macro_end, &macro_name);
    
    /* Create and store the macro body if valid */
    macro_body = array_list_to_str(pp->current_macro);
    if (macro_body) 
    {
        if (is_valid) 
            hash_map_put(pp->macros, macro_name, macro_body);
        else 
            FREE(macro_body);
    }

    /* Clean up */
    FREE(macro_def);
    FREE(macro_end);
    array_list_clear(pp->current_macro);
}

void expand_macro(Preprocessor *pp, AssemblerContext *ctx, const char *macro_name)
{
    char *macro_body = NULL;
    char *tmp_body = NULL;
    char *line = NULL;
    char *token = NULL;
    const char newline_delim = '\n';

    if (!pp || !macro_name || !ctx)
        return;

    /* Check if the macro exists */
    macro_body = hash_map_get(pp->macros, macro_name);
    if (!macro_body)
        return;

    /* Create a copy of the macro body */
    tmp_body = STRDUP(macro_body);
    if (!tmp_body)
        return;
    
    /* Split by newlines and add each line to preprocessed_lines */
    line = tmp_body;
    token = strchr(line, newline_delim);
    
    while (token != NULL) 
    {
        *token = '\0';  /* Replace newline with null terminator */
        array_list_append(ctx->preprocessed_lines, STRDUP(line));
        line = token + 1;  /* Move to character after newline */
        token = strchr(line, newline_delim);
    }
    
    /* Add the last line if it exists */
    if (*line != '\0') 
        array_list_append(ctx->preprocessed_lines, STRDUP(line));
    
    FREE(tmp_body);
}

void preprocess(AssemblerContext *ctx) 
{
    Preprocessor pp;

    if (!ctx)
        return;

    /* Initialize the preprocessor */
    preprocessor_init(&pp);

    /* Read the file into raw_lines */
    pp.raw_lines = file_read_lines(ctx->filename);

    if (!pp.raw_lines) 
    {
        error_report(ctx->errors, ERR_FILE_READ, "Failed to read file: %s", ctx->filename);
        preprocessor_destroy(&pp);
        return;
    }

    /* Process each line */
    while (next_line(&pp, ctx))
    {
        /* Skip empty lines and comments */
        if (is_empty_line(pp.current_line) || is_comment(pp.current_line))
            continue;

        switch (pp.state)
        {
            /* Default state - process lines normally */
            case STATE_DEFAULT:
                /* Check for macro definition or call */
                if (is_macro_def(pp.current_line))
                {
                    pp.state = STATE_MACRO;
                    array_list_append(pp.current_macro, STRDUP_NORM(pp.current_line.str));
                    continue;
                }
                
                if (is_macro_call(&pp, pp.current_line.str))
                {
                    expand_macro(&pp, ctx, pp.current_line.str);
                    continue;
                }

                /* Add the line to preprocessed lines */
                array_list_append(ctx->preprocessed_lines, STRDUP_NORM(pp.current_line.str));
                break;
                
            /* Macro state - process macro lines */
            case STATE_MACRO:
                /* Check for macro end or continue adding lines */
                if (is_macro_end(pp.current_line))
                {
                    pp.state = STATE_DEFAULT;
                    array_list_append(pp.current_macro, STRDUP_NORM(pp.current_line.str));
                    define_macro(&pp, ctx);
                    continue;
                }
                array_list_append(pp.current_macro, STRDUP_NORM(pp.current_line.str));
                continue;
        }
    }

    /* If no errors - generate IR file */
    if (array_list_size(ctx->errors) == 0)
        generate_output(ctx, 0);

    /* Clean up */
    preprocessor_destroy(&pp);
}