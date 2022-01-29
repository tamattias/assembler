/**
 * @file preprocessor.c
 * @author Tamir Attias
 * @brief Preprocessor implementation.
 */

#include "preprocessor.h"
#include "constants.h"
#include "hashtable.h"
#include "dynstr.h"
#include "ioutil.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

/* Number of characters to pre-allocate for macros. */
#define MACRO_BUFFER_INITIAL_CAPACITY 256

/* Number of buckets in macro hash table. */
#define MACRO_TABLE_BUCKET_COUNT 1024

/* Callback for deallocating a macro buffer stored in a hash table. */
static void free_macro(void *macro)
{
    dynstr_free((dynstr_t*)macro);
}

int preprocess(const char *infilename, const char *outfilename)
{
    FILE *in, *out; /* Input, output file pointers. */
    char line[MAX_LINE_LENGTH + 1]; /* Line buffer. */
    int line_no = 0; /* Line number. */
    char *head; /* Pointer to current byte in line being processed. */
    char field[MAX_LINE_LENGTH + 1]; /* Field buffer. */
    int in_macro; /* Non-zero if within a macro definition. */
    hashtable_t *macro_table; /* Table mapping macro names to their body. */
    char macroname[MAX_LINE_LENGTH + 1]; /* Name of currently defined macro. */
    dynstr_t *macro_buf; /* Buffer for currently defined macro's body. */
    dynstr_t *macro; /* Body of referenced macro. */

    /* Open input file. */
    in = fopen(infilename, "r");
    if (!in) {
        printf("preprocess: couldn't open input file: %s\n", infilename);
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
        /* Increment line counter. */
        ++line_no;

        /* Set read head to beginning of line. */
        head = line;

        /* Read first field in line. */
        read_field(&head, field, 0);

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
            if (is_eol(*head)) {
                printf("preprocess: line %d: macro missing name, ignoring line.\n", line_no);
                continue;
            }

            /* Read macro name. */
            read_field(&head, macroname, 0);

            /* Check for extraneous text. */
            if (!is_whitespace_string(head)) {
                printf("preprocess: line %d: extraneous text after macro name, ignoring line.\n", line_no);
                continue;
            }

            /* Enter macro state. */
            in_macro = 1;

            /* Sanity check. */
            assert(macro_buf == 0);
            
            /* Allocate empty macro buffer string. */
            macro_buf = dynstr_alloc(MACRO_BUFFER_INITIAL_CAPACITY);

            continue;
        }

        /* Not a macro declaration. */

        /* Check if first field in line is a macro reference. */
        if ((macro = (dynstr_t*)hashtable_find(macro_table, field)) != 0) {
            /* Write macro contents to output file. */
            fwrite(dynstr_pointer(macro), 1, dynstr_size(macro), out);
            continue;
        }

        /* Not a macro reference. Copy line as is to output. */
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
