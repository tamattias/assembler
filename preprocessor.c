#include "preprocessor.h"
#include "hashtable.h"
#include "dynstr.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

/* Number of characters to pre-allocate for macros. */
#define MACRO_BUFFER_INITIAL_CAPACITY 16

static int is_eol(char c)
{
    return c == '\0' || c == '\n';
}

/* Returns 1 if end of line and 0 otherwise. */
static int read_field(char **line, char *field, int max_length)
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
    /* TODO: Limit max length. */
    while ((c = *(*line)++) != '\0' && !isspace(c))
        *field++ = c;

    /* Add null terminator. */
    field[0] = 0;

    /* If end of line, return 0 else return line pointer. */
    return is_eol(c);
}

static void free_macro(void *macro)
{
    dynstr_free((dynstr_t*)macro);
}

int preprocess(const char *infilename, const char *outfilename)
{
    FILE *in;
    FILE *out;
    hashtable_t *macro_table;
    dynstr_t *macro_buf;
    int in_macro;
    char line[80]; /* TODO: replace with max line length */
    char *head; /* Pointer to current byte in line being processed. */
    char field[80]; /* TODO: Limit to proper value. */
    char macroname[80]; /* TODO: Limit to proper value. */
    dynstr_t *macro;
    int eol;

    in = fopen(infilename, "r");
    if (!in) {
        printf("preprocess: couldn't open file: %s\n", infilename);
        return 1;
    }

    out = fopen(outfilename, "w");
    if (!out) {
        printf("preprocess: couldn't open output file: %s\n", outfilename);
        fclose(in);
        return 1;
    }

    /* TODO: Better way to determine maximum number of slots in macro table. */
    macro_table = hashtable_alloc(1024, free_macro);
    macro_buf = 0;
    in_macro = 0;

    while (fgets(line, sizeof(line), in)) {
        printf("%s", line);

        head = line;

        /* Read first field in line. */
        eol = read_field(&head, field, sizeof(field) - 1);

        printf("First field: %s\n", field);

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
            read_field(&head, macroname, sizeof(macroname) - 1);

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

    fclose(out);

    fclose(in);

    return 0;
}
