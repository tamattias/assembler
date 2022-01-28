/**
 * @file firstpass.h
 * @author Tamir Attias
 * @brief First pass declarations.
 */

#ifndef FIRSTPASS_H
#define FIRSTPASS_H

/* Forward declarations. */
struct shared;

/**
 * Execute first pass of the assembler.
 *
 * @param filename Input file to process.
 * @param shared Shared state.
 * @return Non-zero on success, zero on failure.
 */
int firstpass(const char *filename, struct shared *shared);

#endif