/**
 * @file symtable.c
 * @author Tamir Attias
 * @brief Symbol table implementation.
 */

#include "symtable.h"
#include "hashtable.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

/**
 * Number of slots in hash table. Chosen arbitrarily.
 * Ideally, this should be set to the number of symbols that is expected to be
 * defined on average.
 */
#define SYMTABLE_SLOTS 4096

/* TODO: Use free list to improve performance? */

struct symtable {
    /** Backing hash table. */
    hashtable_t *ht
};

symtable_t *symtable_alloc()
{
    symtable_t *table = (symtable_t*)malloc(sizeof(symtable_t));
    
    /* Allocate the underlying hash table. */
    table->ht = hashtable_alloc(SYMTABLE_SLOTS, free);

    return table;
}

void symtable_free(symtable_t *table)
{
    /* Free underlying hash table. */
    hashtable_free(table->ht);

    free(table);
}

symbol_t *symtable_new(symtable_t *table, const char *label)
{
    symbol_t *sym = (symbol_t*)calloc(1, sizeof(symbol_t));
    
    /* Copy label. */
    strcpy(sym->name, label);

    /* Don't insert empty symbols. */
    assert(label[0] != 0);

    /* Insert into hash table. */
    hashtable_insert(table->ht, label, sym);

    return sym;
}

symbol_t *symtable_find(symtable_t *table, const char *label)
{
    return (symbol_t*)hashtable_find(table->ht, label);
}