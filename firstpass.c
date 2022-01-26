/**
 * @file firstpass.c
 * @author Tamir Attias
 * @brief First pass implementation.
 */

#include "firstpass.h"
#include "constants.h"

#include <stdio.h>

int firstpass(const char *infilename, const char *outfilename)
{
    FILE *in, *out;
    char line[MAX_LINE_LENGTH + 1];

    in = fopen(infilename, "r");
    if (!in) {
        printf("firstpass: could not open input file %s\n", infilename);
        return 1;
    }

    out = fopen(outfilename, "w");
    if (!out) {
        printf("firstpass: could not open output file %s\n", outfilename);
        fclose(in);
        return 1;
    }

    

    while (fgets(line, sizeof(line), in)) {

    }

    fclose(in);
    fclose(out);

    return 0;
}
