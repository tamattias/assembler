/**
 * @file preprocessor.c
 * @author Tamir Attias
 * @brief Preprocessor implementation.
 */

#include "preprocessor.h"
#include "constants.h"
#include "hashtable.h"
#include "dynstr.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

/* Number of characters to pre-allocate for macros. */
#define MACRO_BUFFER_INITIAL_CAPACITY 16

/* Number of buckets in macro hash table. */
#define MACRO_TABLE_BUCKET_COUNT 1024

/* Checks whether a character terminates a line buffer
   i.e., if it is a newline or null terminator. */
static int is_eol(char c)
{
    return c == '\0' || c == '\n';
}

/* Returns 1 if end of line and 0 otherwise. */
static int read_field(char **line, char *field)
{
    char c;

    /* Skip whitespace */
    while ((c = *(*line)++) != '\0' && isspace(c))
        ;

    if (is_eol(c))
        return 0;

    /* Unread whitespace. */
    --(*line);
    
    /* Copy first field until space or end of line. */
    while ((c = *(*line)++) != '\0' && !isspace(c))
        *field++ = c;

    /* Add null terminator. */
    field[0] = 0;

    /* If end of line, return 0 else return line pointer. */
    return is_eol(c);
}

/* Callback for deallocating a macro buffer stored in a hash table. */
static void free_macro(void *macro)
{
    dynstr_free((dynstr_t*)macro);
}

int preprocess(const char *infilename, const char *outfilename)
{
    FILE *in, *out; /* Input, output file pointers. */
    char line[MAX_LINE_LENGTH + 1]; /* Line buffer. */
    char *head; /* Pointer to current byte in line being processed. */
    char field[MAX_LINE_LENGTH + 1]; /* Field buffer. */
    int in_macro; /* Non-zero if within a macro definition. */
    hashtable_t *macro_table; /* Table mapping macro names to their body. */
    char macroname[MAX_LINE_LENGTH + 1]; /* Name of currently defined macro. */
    dynstr_t *macro_buf; /* Buffer for currently defined macro's body. */
    dynstr_t *macro; /* Body of referenced macro. */
    int eol; /* Did we encounter the end of the line when reading a field? */

    /* Open input file. */
    in = fopen(infilename, "r");
    if (!in) {
        printf("preprocess: couldn't open file: %s\n", infilename);
        return 1;
    }

    /* Open output file. */
    out = fopen(outfilename, "w");
    if (!out) {
        printf("preprocess: couldn't open output file: %s\n", outfilename);
        fclose(in);
        return 1;
    }

    /* Initialize macro processing state. */
    macro_table = hashtable_alloc(MACRO_TABLE_BUCKET_COUNT, free_macro);
    macro_buf = 0;
    in_macro = 0;

    /* Read input file line by line. */
    while (fgets(line, sizeof(line), in)) {
        /* Set read head to beginning of line. */
        head = line;

        /* Read first field in line. */
        eol = read_field(&head, field);

        /* Logic when processing a line within a macro. */
        if (in_macro) {
            if (strcmp(field, "endm") == 0) {
                /* End of macro; store in table. */
                hashtable_insert(macro_table, macroname, macro_buf);
                in_macro = 0;
                macro_buf = 0;
            } else {
                /* Not end of macro; append to macro buffer. */
                dynstr_append(macro_buf, line);
            }
            continue;
        }

        /* Check if new macro is being declared. */
        if (strcmp(field, "macro") == 0) {
            /* End of line before macro name specified. */
            if (eol) {
                printf("preprocess: macro missing name.\n");
                break;
            }

            /* Read macro name. */
            read_field(&head, macroname);

            /* TODO: Check for extra fields? Report error? */

            /* Enter macro state. */
            in_macro = 1;

            /* Sanity check. */
            assert(macro_buf == 0);
            
            /* Allocate empty macro buffer string. */
            macro_buf = dynstr_alloc(MACRO_BUFFER_INITIAL_CAPACITY);

            continue;
        }

        /* Macro not declared, check if first field in line is a macro
           reference. */
        if ((macro = (dynstr_t*)hashtable_find(macro_table, field)) != 0) {
            /* Write macro contents to output file. */
            fwrite(dynstr_pointer(macro), 1, dynstr_size(macro), out);
            continue;
        }

        /* Not a macro, copy line as is to output. */
        fputs(line, out);
    }

    /* Free unused macro buffer. */
    if (macro_buf)
        dynstr_free(macro_buf);

    /* Free macro table. */
    hashtable_free(macro_table);

    /* Close output file. */
    fclose(out);

    /* Close input file. */
    fclose(in);

    return 0;
}
