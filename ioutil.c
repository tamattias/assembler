/**
 * @file ioutil.c
 * @author Tamir Attias
 * @brief  Impementation of I/O utilities.
 */
#include "ioutil.h"

#include <ctype.h>

int is_eol(char c)
{
    return c == '\0' || c == '\n';
}

void read_field(char **line, char *field, int *plen)
{
    char c;

    /* Skip whitespace */
    while ((c = *(*line)++) != '\0' && isspace(c))
        ;

    /* Unread non-whitespace or EOL character. */
    --(*line);

    if (is_eol(c))
        return;

    /* Initialize length to 0. */
    if (plen)
        *plen = 0;
    
    /* Copy first field until space or end of line. */
    while ((c = *(*line)++) != '\0' && !isspace(c)) {
        *field++ = c;

        /* Increment length. */
        if (plen)
            ++*plen;
    }

    /* Unread end of line marker. */
    --(*line);

    /* Add null terminator to field. */
    field[0] = 0;
}