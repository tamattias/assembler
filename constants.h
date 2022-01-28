/**
 * @file constants.h
 * @author Tamir Attias
 * @brief Common constants.
 */

#ifndef CONSTANTS_H
#define CONSTANTS_H

/**
 * Maximum line length for all source files, in characters.
 */
#define MAX_LINE_LENGTH 80

/**
 * Maximum length of a label. 
 */
#define MAX_LABEL_LENGTH 31

/**
 * Whitespace delimiters to pass to strtok.
 */
#define WHITESPACE_DELIM "\n\t "

/**
 * Maximum number of items in data directive integer list.
 */
#define MAX_DATA_LENGTH 31

/**
 * Maximum size of data image in words.
 */
#define MAX_DATA_IMAGE_SIZE 8192

/**
 * Maximum size of code image in words.
 */
#define MAX_CODE_IMAGE_SIZE 8192

#endif