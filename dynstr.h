/**
 * @file dynstr.h
 * @author Tamir Attias
 * @brief Dynamic string declarations.
 */

#ifndef DYNSTR_H
#define DYNSTR_H

typedef struct dynstr dynstr_t;

dynstr_t *dynstr_alloc(int capacity);

dynstr_t *dynstr_alloc_from_string(const char *init_str);

void dynstr_free(dynstr_t *str);

int dynstr_size(const dynstr_t *str);

const char *dynstr_pointer(const dynstr_t *str);

void dynstr_append(dynstr_t *str, const char *suffix);

void dynstr_clear(dynstr_t *str);

#endif