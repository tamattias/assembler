/**
 * @file symtable.h
 * @author Tamir Attias
 * @brief Symbol table declarations.
 */

#ifndef SYMTABLE_H
#define SYMTABLE_H

#include "constants.h"
#include "instruction.h"

/**
 * Computes the base address of a symbol.
 *
 * @details The base address of a symbol is the nearest multiple of 16 smaller
 *          than its address. We perform floor division (since addr is always)
 *          positive. Then we multiply the result by 16 to get the right
 *          address.
 */ 
#define SYMBOL_BASE_ADDR(addr) (((addr) / 16) * 16)

/**
 * Computes a symbol offset from its address.
 *
 * @details The remainder of division by 16 is the offset from the base
 *          address.
 */
#define SYMBOL_OFFSET(addr)    ((addr) % 16)

typedef struct {
    char name[MAX_LABEL_LENGTH + 1]; /** Label. */
    word_t value; /** Value. */
    word_t base_addr; /** Base address. */
    word_t offset;    /** Offset from base address in memory. */
    int ext; /** External flag. */
    int ent; /** Entry flag. */
    int code; /** Code flag. */
    int data; /** Data flag. */
} symbol_t;

typedef struct symtable symtable_t;

/**
 * Allocates a symbol table.
 *
 * @return Pointer to the allocated table.
 */
symtable_t *symtable_alloc();

/**
 * Frees a symbol table and all symbols in it.
 *
 * @param table Pointer to the table to free.
 */
void symtable_free(symtable_t *table);

/**
 * Creates a new symbol in the symbol table with the given label if doesn't
 * already exist in the table.
 *
 * @param table The symbol table into which to insert the symbol.
 * @param label Label that will be copied to the symbol's name field.
 * @return If the symbol was not already present in the table, a pointer to
 *         the new symbol, else a null pointer.
 */
symbol_t *symtable_new(symtable_t *table, const char *label);

/**
 * Performs a symbol lookup.
 *
 * @param table The symbol table in which to perform the lookup.
 * @param label The name of the symbol to look for.
 * @return If the symbol is found, a pointer to the symbol else a null
           pointer.
 */
symbol_t *symtable_find(symtable_t *table, const char *label);

#endif