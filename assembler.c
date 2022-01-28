/**
 * @file assembler.c
 * @author Tamir Attias
 * @brief Assembler entry point.
 */

#include "shared.h"
#include "preprocessor.h"
#include "firstpass.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void print_usage()
{
    puts("usage: assembler <filename>");
}

int main(int argc, char *argv[])
{
    char *basename;
    char infilename[FILENAME_MAX + 1];
    char outfilename[FILENAME_MAX + 1];
    shared_t *shared = 0;

    if (argc < 2) {
        print_usage();
        return 1;
    }

    basename = argv[1];

    /* Set input filename to basename with .as extension. */
    strcpy(infilename, basename);
    strcat(infilename, ".as");

    /* Set output filename to base with .am extension. */
    strcpy(outfilename, basename);
    strcat(outfilename, ".am");

    /* Preprocess. */
    if (preprocess(infilename, outfilename)) {
        printf("error: could not preprocess source file.\n");
        return 1;
    }

    /* Allocate shared assembly state. We don't keep this on the stack because
       the memory image is quite large. */
    shared = (shared_t*)calloc(1, sizeof(shared_t));

    /* Run first pass. */
    if (firstpass(outfilename, shared)) {
        printf("error: first pass failed, aborting.\n");
        free(shared); /* Free shared state. */
        return 1;
    }

    /* Free shared state as it's no longer needed. */
    free(shared);

    return 0;
}
