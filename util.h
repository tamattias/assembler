/**
 * @file util.h
 * @author Tamir Attias
 * @brief General utility function declarations.
 */

#ifndef UTIL_H
#define UTIL_H

#include "instset.h"

/**
 * Checks whether a character terminates a line buffer i.e., if it is a newline
 * or null terminator.
 */
int is_eol(char c);

/**
 * Checks whether a string has only whitespace characters.
 *
 * @param str Null terminated string to check.
 * @return Non-zero if the string has only whitespace characters or if the
 *         string is empty, else zero.
 */
int is_whitespace_string(const char *str);

/**
 * Reads the next field from a line.
 * 
 * @param line Pointer to line buffer, will be incremented by the amount of
 *             characters read.
 * @param field Pointer to a buffer to which the field will be copied.
 * @param plen Pointer to an integer that will receive the length of the field.
 */
void read_field(char **line, char *field, int *plen);

/**
 * Try to parse a number from a token.
 *
 * @param tok Token to parse.
 * @param w Output pointer to the number.
 * @return Zero on success, non-zero on error.
 * @note Extraneous (non-whitespace) characters after the number are treated
 *       as an error.
 */
int parse_number(const char *tok, word_t *w);

#endif