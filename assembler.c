/**
 * @file assembler.c
 * @author Tamir Attias
 * @brief Assembler entry point.
 */

#include "preprocessor.h"
#include "shared.h"
#include "firstpass.h"
#include "secondpass.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * Print friendly usage instructions.
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
    /* TODO: Handle long paths correctly when using strcat. */

    char as_filename[FILENAME_MAX + 1]; /* Source assembly file path (.as). */
    char am_filename[FILENAME_MAX + 1]; /* Macro expanded file path (.am). */
    char ob_filename[FILENAME_MAX + 1]; /* Object file path (.ob). */
    char ent_filename[FILENAME_MAX + 1]; /* Entry points file path (.ent). */
    char ext_filename[FILENAME_MAX + 1]; /* Externals file path (.ext). */
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
       the memory segments are quite large. */
    shared = shared_alloc();

    /* Run first pass. */
    if (firstpass(am_filename, shared)) {
        printf("fatal error: first pass failed.\n");
        shared_free(shared);
        return 1;
    }

    /* Set object file path by appending .ob extension. */
    strcpy(ob_filename, basename);
    strcat(ob_filename, ".ob");

    /* Set entry points file path by appending .ent. */
    strcpy(ent_filename, basename);
    strcat(ent_filename, ".ent");

    /* Set externals file path by appending .ext. */
    strcpy(ext_filename, basename);
    strcat(ext_filename, ".ext");

    /* Run second pass. */
    if (secondpass(am_filename, ob_filename, ent_filename, ext_filename, shared)) {
        printf("fatal error: second pass failed.\n");
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
