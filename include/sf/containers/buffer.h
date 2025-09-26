#ifndef SF_BUFFER_H
#define SF_BUFFER_H

#include <stdint.h>
#include "export.h"
#include "sf/result.h"

/// Flags for determining a buffer's behavior.
typedef enum {
    SF_BUFFER_GROW     = (1 << 1),
    SF_BUFFER_EMPTY    = (1 << 2),
    SF_BUFFER_READONLY = (1 << 3),
} sf_buffer_flag;

/// Locations to offset from for use with buffer seeking.
typedef enum {
    SF_BUFFER_START,
    SF_BUFFER_END,
} sf_buffer_handle;

/// A dynamic buffer object which can be written to and optionally expand.
typedef struct {
    size_t size;
    uint8_t *ptr;
    uint8_t *head;
    uint8_t flags;
} sf_buffer;

/// Allocate a fixed buffer at a specified size.
EXPORT sf_buffer sf_buffer_fixed(size_t size);
/// Allocate a buffer that grows as you insert bytes.
EXPORT sf_buffer sf_buffer_grow(void);
/// Wrap an existing buffer with an sf_buffer.
EXPORT sf_buffer sf_buffer_own(uint8_t *existing, size_t size);
/// Insert a value into a buffer.
EXPORT sf_result sf_buffer_insert(sf_buffer *buffer, const void *const ptr, size_t size);
/// Insert a value into a buffer with automatic sizing.
#define sf_buffer_autoins(buffer, value) sf_buffer_insert(buffer, value, sizeof(*value))
/// Seek to a position in the buffer.
EXPORT void sf_buffer_seek(sf_buffer *buffer, sf_buffer_handle handle, int64_t offset);
/// Free a buffer and/or revert it to an empty state.
EXPORT void sf_buffer_clear(sf_buffer *buffer);
/// Copies x bytes from the buffer head to the specified location.
EXPORT sf_result sf_buffer_read(sf_buffer *buffer, void *dest, size_t bytes);
/// Automatically read a value based on the type of the pointer.
/// `dest` must be a pointer.
#define sf_buffer_autoread(buffer, dest) sf_buffer_read(buffer, dest, sizeof(*dest))

#endif // SF_BUFFER