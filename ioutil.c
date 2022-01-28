#include "ioutil.h"

#include <ctype.h>

int is_eol(char c)
{
    return c == '\0' || c == '\n';
}

/**
 * TODO: Adapt this so end of line is unread for is_eol to work.
 */
int read_field(char **line, char *field, int *plen)
{
    char c;

    /* Skip whitespace */
    while ((c = *(*line)++) != '\0' && isspace(c))
        ;

    if (is_eol(c))
        return 0;

    /* Unread whitespace. */
    --(*line);

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

    /* Add null terminator. */
    field[0] = 0;

    /* If end of line, return 0 else return line pointer. */
    return is_eol(c);
}