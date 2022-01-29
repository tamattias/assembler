/**
 * @file dynstr.h
 * @author Tamir Attias
 * @brief Dynamic string declarations.
 * @details A dynamic string is an expandable string without a size limit. The
 *          backing buffer is reallocated as necessary.
 */

#ifndef DYNSTR_H
#define DYNSTR_H

typedef struct dynstr dynstr_t;

/**
 * Allocates an empty dynamic string.
 *
 * @param capacity Initial capacity in characters.
 * @return Pointer to the dynamic string object or null if out of memory.
 */
dynstr_t *dynstr_alloc(int capacity);

/**
 * Allocates a dynamic string from an initial character string.
 *
 * @param init_str Null terminated string of characters to copy to the dynamic
 *                 string after allocation.
 * @note The initial capacity will be set to the length of the string.
 * @return Pointer to the dynamic string object or null if out of memory.
 */
dynstr_t *dynstr_alloc_from_string(const char *init_str);

/**
 * Frees a dynamic string from memory.
 *
 * @param str Pointer to the dynamic string object. It must not be accessed
 *            after calling this function.
 */
void dynstr_free(dynstr_t *str);

/**
 * Returns the number of characters in the string.
 *
 * @param str Pointer to the string object whose length is to be retrieved.
 * @return Number of characters in the dynamic string, excluding the null
 *         terminator.
 */
int dynstr_size(const dynstr_t *str);

/**
 * Returns a pointer to the internal representation of the string.
 *
 * @return Pointer to the null terminated buffer that backs this dynamic
 *         string.
 */
const char *dynstr_pointer(const dynstr_t *str);

/**
 * Appends characters to the string, reallocating the string if necessary.
 *
 * @param str Pointer to the dynamic string object to append to.
 * @param suffix Null termintaed string of characters to append.
 * @return Zero on success, non-zero if error (e.g., out of memory.)
 * @note When out of memory a non-zero value is returned and the string stays
 *       the same as before.
 */
int dynstr_append(dynstr_t *str, const char *suffix);

/**
 * Empties a previously allocated dynamic string.
 *
 * @param str Pointer to string object to clear.
 */
void dynstr_clear(dynstr_t *str);

#endif