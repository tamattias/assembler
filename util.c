/**
 * @file util.c
 * @author Tamir Attias
 * @brief General utility function implementations.
 */

#include "util.h"

#include <ctype.h>
#include <string.h>

int skip_line(FILE *fp)
{
    int c;

    /* Keep reading until end of file or newline. */
    while ((c = getc(fp)) != EOF && c != '\n')
        ;

    return c == EOF ? EOF : 0;
}

int is_eol(char c)
{
    return c == '\0' || c == '\r' ||c == '\n';
}

int is_whitespace_string(const char *str)
{
    /* Go through every whitespace character and return 1 if
       end of string is reached without encountering a non-whitespace
       character. */
    char c;
    while ((c = *str++) != '\0' && isspace(c))
        ;
    return c == '\0';
}

void read_field(char **line, char *field, int *plen)
{
    char c;

    /* Skip whitespace */
    while ((c = *(*line)++) != '\0' && isspace(c))
        ;

    /* Unread non-whitespace or EOL character. */
    --(*line);

    /* Check if empty field. */
    if (is_eol(c)) {
        field[0] = '\0';
        return;
    }

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
    field[0] = '\0';
}

int parse_number(const char *tok, word_t *w)
{
    char c; /* Current character. */
    word_t sign = 1; /* Number sign. */

    /* Skip whitespace. */
    while (!is_eol(c = *tok++) && isspace(c))
        ;
    
    /* Check if empty string. */
    if (is_eol(c))
        return 1;

    /* Check if number sign. */
    if (c == '+' || c == '-') {
        sign *= (c == '-') ? -1 : 1;
    } else {
        /* Unread last character. */
        --tok;
    }

    /* Initialize to 0. */
    *w = 0;
    
    /* Read digits. */
    while (!is_eol(c = *tok++) && isdigit(c)) {
        *w *= 10;
        *w += c - '0';
    }

    /* Unread last non-digit character. */
    --tok;

    /* Check if we have any extraneous characters. */
    if (!is_whitespace_string(tok))
        return -1;

    /* Apply sign. */
    *w *= sign;

    return 0;
}
