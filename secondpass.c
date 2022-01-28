/**
 * @file secondpass.h
 * @author Tamir Attias
 * @brief Second pass definitions.
 */

#include "secondpass.h"
#include "constants.h"
#include "shared.h"

#include <stdio.h>
#include <string.h>

/**
 * Internal state for second pass.
 */
typedef struct {
    const char *line_head;
} state_t;

static int process_line(state_t *st, const char *line)
{
    st->line_head = line;
    return 0;
}

static int write_image(FILE *fp, const word_t *image, int base_addr, int len)
{
    int i;
    word_t w;

    for (i = 0; i < len; ++i) {
        /* Get word. */
        w = image[i];

        /* Write address. */
        fprintf(fp, "%04d ", base_addr + i);

        /* Encode it to file. */
        fprintf(fp, "A%x-B%x-C%x-D%x-E%x\n",
            (unsigned)((w >> 16) & 0xF),
            (unsigned)((w >> 12) & 0xF),
            (unsigned)((w >> 8)  & 0xF),
            (unsigned)((w >> 4)  & 0xF),
            (unsigned)((w >> 0)  & 0xF)
        );
    }

    return 0;
}

static int write_object_file(const char *obfilename, struct shared *shared)
{
    FILE *out; /* Output file pointer. */
    int error = 0; /* Return value. */

    /* Try to open file for writing. */
    if ((out = fopen(obfilename, "wb")) == 0) {
        printf("secondpass: could not open object file %s for writing\n", obfilename);
        return 1;
    }

    /* Write header. */
    if (fprintf(out, "%d %d\n", shared->code_image_len, shared->data_image_len) < 0) {
        printf("secondpass: error: could not write header.\n");
        goto done;
    }

    /* Write code image. */
    if ((error = write_image(out, shared->code_image, 100, shared->code_image_len)) != 0) {
        printf("secondpass: error: could not write code image.\n");
        goto done;
    }

    /* Write data image. */
    if ((error = write_image(out, shared->data_image, 100 + shared->code_image_len, shared->data_image_len)) != 0) {
        printf("secondpass: error: could not write data image.\n");
        goto done;
    }

done:
    /* Close output file. */
    fclose(out);

    return error;
}

int secondpass(const char *infilename, const char *obfilename, struct shared *shared)
{
    FILE *in;
    char line[MAX_LINE_LENGTH + 1];
    state_t st;
    int error = 0;

    /* Try to open input file. */
    if ((in = fopen(infilename, "r")) == 0) {
        printf("secondpass: error: could not open %s\n", infilename);
        return 1;
    }

    /* Initialize state to zero. */
    memset(&st, 0, sizeof(st));

    /* Process file line by line. */
    while (fgets(line, sizeof(line), in))
        error |= process_line(&st, line);

    /* Close input file. */
    fclose(in);

    /* Terminate early if an error was encountered. */
    if (error)
        return error;

    /* Write machine code to object file. */
    if ((error = write_object_file(obfilename, shared)) != 0)
        return error;

    return 0;
}

