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

int parse_operand(const char *tok, addr_mode_t addr_mode, operand_t *op)
{
    char c;

    /* Skip whitespace. */
    while ((c = *tok++) != '\0' && isspace(tok))
        ;

    /* Check if empty string. */
    if (c == '\0')
        return 1;

    /* Unread non-whitespace char. */
    --tok;

    if (addr_mode == ADDR_MODE_IMMEDIATE) {
        /* Read integer value. */
        /* TODO: Only allow decimal numbers. */
        return sscanf(tok, "#%ld", &op->value) == 1 ? 0 : -1;
    } else if (addr_mode == ADDR_MODE_DIRECT) {
        /* Read label. */
        return sscanf(tok, "%s", op->label) == 1 ? 0 : -1;
    } else if (addr_mode == ADDR_MODE_INDEX) {
        /* Read label and register index. */
        /* TODO: Only allow decimal numbers. */
        if (sscanf(tok, "%s[r%ld]", op->label, &op->value) != 2)
            return -1;

        /* Check if register index is valid. */
        if (op->value < 10 || op->value > 15)
            return -1; /* Invalid register index. */
    } else if (addr_mode == ADDR_MODE_REGISTER_DIRECT) {
        /* TODO: Handle other register types. */
        /* TODO: Only allow decimal numbers. */
        return sscanf(tok, "r%ld", &op->value) == 1 ? 0 : -1;
    }

    /* TODO: Keep reading. If non-whitespace or EOL encountered then this is an error.*/

    return 0;
}

