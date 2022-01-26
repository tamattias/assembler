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
 * @return 1 if end of line and 0 otherwise.
 */
int read_field(char **line, char *field);

#endif