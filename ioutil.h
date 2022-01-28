/**
 * @file ioutil.h
 * @author Tamir Attias
 * @brief Input/output utilities.
 */

#ifndef IOUTIL_H
#define IOUTIL_H

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
 * @return 1 if end of line was reached, 0 otherwise.
 */
int read_field(char **line, char *field, int *plen);

#endif