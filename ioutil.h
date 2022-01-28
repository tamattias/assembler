/**
 * @file ioutil.h
 * @author Tamir Attias
 * @brief Input/output utilities.
 */

#ifndef IOUTIL_H
#define IOUTIL_H

#include "instruction.h"

/**
 * Checks whether a character terminates a line buffer i.e., if it is a newline
 * or null terminator.
 */
int is_eol(char c);

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
 * @return Zero on success, non-zero on failure.
 */
int parse_number(const char *tok, word_t *w);

#endif