/**
 * @file dynstr.c
 * @author Tamir Attias
 * @brief Dynamic string implementation.
 */

#include "dynstr.h"

#include <stdlib.h>
#include <string.h>

struct dynstr {
    int capacity;
    int size;
    char *buf;
};

dynstr_t *dynstr_alloc(int capacity)
{
    dynstr_t *str;
    str = (dynstr_t*)malloc(sizeof(dynstr_t));
    str->size = 0;
    str->capacity = capacity;
    str->buf = (char*)malloc(capacity + 1);
    str->buf[0] = 0; /* Null terminator. */
    return str;
}

dynstr_t *dynstr_alloc_from_string(char *init_str)
{
    dynstr_t *str;
    str = (dynstr_t*)malloc(sizeof(dynstr_t));
    str->size = strlen(init_str);
    str->capacity = str->size;
    str->buf = (char*)malloc(str->capacity + 1);
    memcpy(str->buf, init_str, str->size + 1);
    return str;
}

void dynstr_free(dynstr_t *str)
{
    free(str->buf);
    free(str);
}

int dynstr_size(dynstr_t *str)
{
    return str->size;
}

const char *dynstr_pointer(dynstr_t *str)
{
    return str->buf;
}

void dynstr_append(dynstr_t *str, char *suffix)
{
    int suffix_len;
    int new_len;

    suffix_len = strlen(suffix);
    new_len = str->size + suffix_len;

    /* Check if we would overflow the bufefr. */
    if (new_len > str->capacity) {
        /* Double capacity. */
        str->capacity *= 2;

        /* Expand the string. */
        str->buf = realloc(str->buf, str->capacity + 1);
    }

    /* Append suffix at end of string and update size. */
    memcpy(str->buf + str->size, suffix, suffix_len);
    str->size  = new_len;
    str->buf[str->size] = '\0'; /* Add null terminator. */
}

void dynstr_clear(dynstr_t *str)
{
    str->size = 0;
    str->buf[0] = 0;
}
