/**
 * @file ioutil.c
 * @author Tamir Attias
 * @brief  Impementation of I/O utilities.
 */
#include "ioutil.h"

#include <ctype.h>
#include <string.h>
#include <stdio.h>

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

int parse_number(const char *tok, word_t *w)
{
    char c; /* Current character. */
    word_t sign = 1; /* Number sign. */

    /* Skip whitespace. */
    while (!is_eol(c = *tok++) && isspace(c))
        ;
    
    /* Check if empty string. */
    if (is_eol(c)) {
        printf("parse_number: empty string.\n");
        return 1;
    }


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

    /* Apply sign. */
    *w *= sign;
    
    /* Check if we have any extraneous characters. */
    while (!is_eol(c = *tok++) && isspace(c))
        ;
    if (!is_eol(c)) {
        printf("parse_number: extraneous character: %c.\n", c);
        return -1; /* Got extraneous characters. TODO: Indicate in return value? */
    }

    return 0;
}

