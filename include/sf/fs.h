#ifndef FILES_H
#define FILES_H

#include "sf/containers/buffer.h"
#include "sf/str.h"
#include "export.h"

typedef enum {
    SF_FILE_NOT_FOUND,
    SF_OPEN_FAILURE,
    SF_READ_FAILURE,
} sf_fs_err;

#define EXPECTED_NAME sf_fs_ex
#define EXPECTED_E sf_fs_err
#include "sf/containers/expected.h"

#define EXPECTED_NAME sf_fsb_ex
#define EXPECTED_O sf_buffer
#define EXPECTED_E sf_fs_err
#include "sf/containers/expected.h"

/// Get the size of a file at the specified path. Returns -1 if the file doesn't exist.
EXPORT long sf_file_size(sf_str path);
#define sf_file_exists(path) (sf_file_size(path) >= 0)
/// Load a file into a preallocated buffer. Returns an error result on failure.
EXPORT sf_fs_ex sf_load_file(uint8_t *out, sf_str path);
/// Load a file as an sf_buffer.
EXPORT sf_fsb_ex sf_file_buffer(sf_str path);

#endif // FILES_H
