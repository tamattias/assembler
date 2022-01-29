/**
 * @file shared.h
 * @author Tamir Attias
 * @brief Shared assembler state declarations.
 */

#ifndef SHARED_H
#define SHARED_H

#include "instset.h"
#include "constants.h"

/* Forward declaration. */
struct symtable;

/**
 * Data about an instruction encoded in the code segment.
 */ 
typedef struct {
    /** Address relative to beginning of object file. */
    int address;
    /** Symbol referenced by each operand (may be empty). */
    char operand_symbols[MAX_OPERANDS][MAX_LABEL_LENGTH + 1];
    /** Number of operands. */
    int num_operands;
} inst_data_t;

/**
 * State shared between assembly passes.
 */
typedef struct shared {
    /** Data segment. */
    word_t data_seg[MAX_DATA_SEGMENT_LEN];
    /** Length of data segment in words. */
    int data_seg_len;
    /** Machine code segment. */
    word_t code_seg[MAX_CODE_SEGMENT_LEN];
    /** Length of code segment in words. */
    int code_seg_len;
    /** Data about instructions in code segment. */
    inst_data_t instructions[MAX_CODE_SEGMENT_LEN];
    /** Number of instructions in code segment. */
    int instruction_count;
    /** Symbol table. */
    struct symtable *symtable;
} shared_t;

/**
 * Allocate shared state.
 *
 * @return Pointer to allocated and initialized shared state.
 */
shared_t *shared_alloc();

/**
 * Free shared state.
 *
 * @param shared Shared state to free.
 */
void shared_free(shared_t *shared);

#endif
