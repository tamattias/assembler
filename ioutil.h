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
 * Try to parse an operand from a token.
 *
 * @details Parses an operand that matches the given addressing mode. If the
 *          operand is invalid or doesn't match the addressing mode, an error
 *          value is returned.
 * @param tok Token to parse.
 * @param addr_mode Which address mode to expect?
 * @return Zero on success, non-zero on failure.
 */
int parse_operand(const char *tok, addr_mode_t addr_mode, operand_t *op);

#endif