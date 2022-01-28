/**
 * @file shared.h
 * @author Tamir Attias
 * @brief Shared assembler state declarations.
 */

#ifndef SHARED_H
#define SHARED_H

#include "instruction.h"
#include "constants.h"

/* Forward declaration. */
struct symtable;

/**
 * State shared between assembly passes.
 */
typedef struct shared {
    word_t data_image[MAX_DATA_IMAGE_SIZE]; /** Data image. */
    int data_image_len; /** Length of data image in words. */
    word_t code_image[MAX_CODE_IMAGE_SIZE]; /** Machine code image. */
    int code_image_len; /** Length of code image in words. */
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
