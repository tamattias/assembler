#include "preprocessor.h"

#include <stdio.h>
#include <string.h>

void print_usage()
{
    puts("usage: assembler <filename>");
}

int main(int argc, char *argv[])
{
    char *basename;
    char infilename[FILENAME_MAX+1];
    char outfilename[FILENAME_MAX+1];

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

    /* Preprocess */
    if (preprocess(infilename, outfilename)) {
        printf("error: could not preprocess source file.\n");
        return 1;
    }

    return 0;
}
