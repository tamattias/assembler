/**
 * @file shared.h
 * @author Tamir Attias
 * @brief Shared assembler state.
 */

#ifndef SHARED_H
#define SHARED_H

#include "instruction.h"
#include "constants.h"

/**
 * State shared between assembly passes.
 */
typedef struct shared {
    word_t data_image[MAX_DATA_IMAGE_SIZE]; /** Data image. */
    int data_image_len; /** Length of data image in words. */
    word_t code_image[MAX_CODE_IMAGE_SIZE]; /** Machine code image. */
    int code_image_len; /** Length of code image in words. */
} shared_t;

#endif
