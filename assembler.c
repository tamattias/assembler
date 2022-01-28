/**
 * @file assembler.c
 * @author Tamir Attias
 * @brief Assembler entry point.
 */

#include "preprocessor.h"
#include "shared.h"
#include "firstpass.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * Print friendly usage intstructions.
 */
void print_usage()
{
    puts("usage: assembler <basename> [...basename]");
    puts("example: assembler test/ps test/data");
}

/**
 * Assembles a file.
 *
 * @param basename Path to the source file to process without extension.
 * @return Zero on success, non-zero on failure.
 */
static int process_file(const char *basename)
{
    char as_filename[FILENAME_MAX + 1]; /* Source assembly file name (.as). */
    char am_filename[FILENAME_MAX + 1]; /* Macro expanded file name (.am). */
    shared_t *shared; /* Shared assembly state. */

    /* Set input filename to basename with .as extension. */
    strcpy(as_filename, basename);
    strcat(as_filename, ".as");

    /* Set output filename to base with .am extension. */
    strcpy(am_filename, basename);
    strcat(am_filename, ".am");

    /* Preprocess. */
    if (preprocess(as_filename, am_filename)) {
        printf("error: could not preprocess source file.\n");
        return 1;
    }

    /* Allocate shared assembly state. We don't keep this on the stack because
       the memory image is quite large. */
    shared = shared_alloc();

    /* Run first pass. */
    if (firstpass(am_filename, shared)) {
        printf("error: first pass failed, aborting.\n");
        shared_free(shared);
        return 1;
    }

    /* Free shared assembly state. */
    shared_free(shared);

    return 0;
}

int main(int argc, char *argv[])
{
    int error = 0; /* Did some file fail to process? */

    /* Too few arguments, print correct usage. */
    if (argc < 2) {
        print_usage();
        return 1;
    }

    /* Process all file basenames in the second argument onward and note when 
       a file fails to process for the exit code. */
    while (*++argv)
        error |= process_file(*argv);

    return error;
}
