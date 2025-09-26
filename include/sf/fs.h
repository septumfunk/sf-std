#ifndef FILES_H
#define FILES_H

#include "sf/result.h"

/// Get the size of a file at the specified path. Returns -1 if the file doesn't exist.
EXPORT long sf_file_size(sf_str path);
#define sf_file_exists(path) (sf_file_size(path) >= 0)
/// Load a file into a byte buffer. Returns an error result on failure.
EXPORT sf_result sf_load_file(uint8_t *out, sf_str path);

#endif // FILES_H
