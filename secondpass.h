/**
 * @file secondpass.h
 * @author Tamir Attias
 * @brief Second pass declarations.
 */

#ifndef SECONDPASS_H
#define SECONDPASS_H

/* Forward declaration. */
struct shared;

/**
 * Executes second pass.
 *
 * @param infilename Input filename (.am file).
 * @param obfilename Object filename (.ob)
 * @param shared Shared assembly state.
 * @return Zero on success, non-zero on failure.
 */
int secondpass(const char *infilename, const char *obfilename, struct shared *shared);

#endif