/**
 * @file shared.h
 * @author Tamir Attias
 * @brief Shared assembler state definitions.
 */

#include "shared.h"
#include "symtable.h"

#include <stdlib.h>

shared_t *shared_alloc()
{
    shared_t *shared = (shared_t*)calloc(1, sizeof(shared_t));
    
    /* Allocate symbol table. */
    shared->symtable = symtable_alloc();

    return shared;
}

void shared_free(shared_t *shared)
{
    /* Free symbol table. */
    symtable_free(shared->symtable);

    free(shared);
}
