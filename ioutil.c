#include "ioutil.h"

int is_eol(char c)
{
    return c == '\0' || c == '\n';
}

int read_field(char **line, char *field)
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
    while ((c = *(*line)++) != '\0' && !isspace(c))
        *field++ = c;

    /* Add null terminator. */
    field[0] = 0;

    /* If end of line, return 0 else return line pointer. */
    return is_eol(c);
}