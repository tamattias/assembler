/**
 * @file dynstr.c
 * @author Tamir Attias
 * @brief Dynamic string implementation.
 */

#include "dynstr.h"

#include <stdlib.h>
#include <string.h>

struct dynstr {
    /** Maximum number of characters excluding null terminator. */
    int capacity;
    /** Number of characters in buf excluding null terminator. */
    int size;
    /** Backing buffer. */
    char *buf;
};

dynstr_t *dynstr_alloc(int capacity)
{
    /* Allocate a string object. */
    dynstr_t *str = (dynstr_t*)malloc(sizeof(dynstr_t));

    /* Check if out of memory. */
    if (!str)
        return 0;

    /* Initial size is 0. */
    str->size = 0;

    /* Set capacity to given capacity. */
    str->capacity = capacity;

    /* Allocate a backing buffer with room for null terminator. */
    str->buf = (char*)malloc(capacity + 1);

    /* Check if out of memory. */
    if (!str->buf) {
        free(str); /* Free the string object. */
        return 0;
    }

    /* Null terminate. */
    str->buf[0] = 0; /* Null terminator. */

    return str;
}

dynstr_t *dynstr_alloc_from_string(const char *init_str)
{
    /* Allocate a string object. */
    dynstr_t *str = (dynstr_t*)malloc(sizeof(dynstr_t));
    
    /* Check if out of memory. */
    if (!str)
        return 0;

    /* Set initial size to size of init_str. */
    str->size = strlen(init_str);

    /* Set capacity to size of init_str. */
    str->capacity = str->size;

    /* Allocate the backing buffer with room for null terminator. */
    str->buf = (char*)malloc(str->capacity + 1);

    /* Check if out of memory. */
    if (!str->buf) {
        free(str); /* Free the string object. */
        return 0;
    }

    /* Copy initial string to backing buffer including null terminator. */
    memcpy(str->buf, init_str, str->size + 1);

    return str;
}

void dynstr_free(dynstr_t *str)
{
    /* Free backing buffer. */
    free(str->buf);

    /* Free string object. */
    free(str);
}

int dynstr_size(const dynstr_t *str)
{
    return str->size;
}

const char *dynstr_pointer(const dynstr_t *str)
{
    return str->buf;
}

int dynstr_append(dynstr_t *str, const char *suffix)
{
    int suffix_len; /* Suffix length. */
    int new_len; /* Expanded string length. */
    int new_capacity; /* New capacity. */
    char *new_buf; /* Rellocated buffer. */

    /* Find length of suffix. */
    suffix_len = strlen(suffix);

    /* Find new length of expanded string. */
    new_len = str->size + suffix_len;

    /* Check if we would overflow the buffer. */
    if (new_len > str->capacity) {
        /* Double capacity. */
        new_capacity = str->capacity * 2;

        /* Expand the string. */
        new_buf = realloc(str->buf, str->capacity + 1);

        /* Check if out of memory. */
        if (!new_buf)
            return 1; /* At this point str->buf is NOT freed. */

        /* Update capacity and replace old buffer with new buffer. */
        str->capacity = new_capacity;
        str->buf = new_buf;
    }

    /* Copy suffix to end of string. */
    memcpy(str->buf + str->size, suffix, suffix_len);
    
    str->size = new_len; /* Update size. */
    str->buf[str->size] = '\0'; /* Add null terminator. */

    return 0;
}

void dynstr_clear(dynstr_t *str)
{
    /* Set size to zero and null terminate. */
    str->size = 0;
    str->buf[0] = 0;
}
