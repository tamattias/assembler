/**
 * @file preprocessor.h
 * @author Tamir Attias
 * @brief Preprocessor declarations.
 */

#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

/**
 * Preprocesses an input file, reading macro definitions and expanding them.
 *
 * @param infilename Path of raw input file.
 * @param outfilename Path of processed output file.
 * @return Zero on success, non-zero on failure.
 */
int preprocess(const char *infilename, const char *outfilename);

#endif
