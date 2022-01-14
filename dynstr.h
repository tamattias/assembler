/**
 * @file dynstr.h
 * @author Tamir Attias
 * @brief Dynamic string declarations.
 */

#ifndef DYNSTRING_H
#define DYNSTRING_H

typedef struct dynstr dynstr_t;

dynstr_t *dynstr_alloc(int capacity);

dynstr_t *dynstr_alloc_from_string(char *init_str);

void dynstr_free(dynstr_t *str);

int dynstr_size(dynstr_t *str);

const char *dynstr_pointer(dynstr_t *str);

void dynstr_append(dynstr_t *str, char *suffix);

void dynstr_clear(dynstr_t *str);

#endif