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
 * Data about an instruction stored in the code segment.
 */ 
typedef struct {
    int ic; /** Address in code segment. */
    int l;  /** Number of words occupied by the instruction. */
    char operand_symbols[MAX_OPERANDS][MAX_LABEL_LENGTH + 1]; /** Symbol referenced by each operand (may be empty). */
    int num_operands; /** Number of operands. */
} inst_data_t;

/**
 * State shared between assembly passes.
 */
typedef struct shared {
    word_t data_seg[MAX_DATA_SEGMENT_SIZE]; /** Data image. */
    int data_seg_len; /** Length of data image in words. */
    word_t code_seg[MAX_CODE_SEGMENT_SIZE]; /** Machine code segment. */
    int code_seg_len; /** Length of code segment in words. */
    inst_data_t instructions[MAX_CODE_SEGMENT_SIZE]; /** Data about instructions in code segment. */
    int instruction_count; /** Number of instructions in code segment. */
    struct symtable *symtable; /** Symbol table. */
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
