#include "sf/containers/buffer.h"

sf_buffer sf_buffer_fixed(const size_t size) {
    uint8_t *dat = calloc(1, size);
    return (sf_buffer) {
        .size = size,
        .ptr = dat,
        .head = dat,
        .flags = 0,
    };
}

sf_buffer sf_buffer_grow() {
    return (sf_buffer) {
        .size = 0,
        .ptr = nullptr,
        .head = nullptr,
        .flags = SF_BUFFER_GROW | SF_BUFFER_EMPTY,
    };
}

sf_buffer sf_buffer_own(uint8_t *existing, const size_t size) {
    return (sf_buffer) {
        .size = size,
        .ptr = existing,
        .head = existing,
        .flags = 0,
    };
}

sf_result sf_buffer_insert(sf_buffer *buffer, const void *const ptr, const size_t size) {
    const uint64_t offset = (uint64_t)(buffer->ptr + buffer->size - buffer->head);
    if (offset < size) {
        if (buffer->flags & SF_BUFFER_EMPTY) {
            buffer->size = size;
            buffer->ptr = malloc(size);
            buffer->head = buffer->ptr;
            buffer->flags &= ~SF_BUFFER_EMPTY;
        } else if (buffer->flags & SF_BUFFER_GROW) {
            const long long ofs = buffer->head - buffer->ptr;
            buffer->size += size - offset;
            buffer->ptr = realloc(buffer->ptr, buffer->size);
            buffer->head = buffer->ptr + ofs;
        } else return sf_err(sf_lit("Buffer is fixed size and head doesn't have space to write."));
    }

    memcpy(buffer->head, ptr, size);
    buffer->head += size;
    return sf_ok();
}

void sf_buffer_seek(sf_buffer *buffer, const sf_buffer_handle handle, const int64_t offset) {
    switch (handle) {
        case SF_BUFFER_START:
            buffer->head = buffer->ptr + min((size_t)offset, buffer->size);
            break;
        case SF_BUFFER_END:
            buffer->head = buffer->ptr + buffer->size - min((size_t)offset, buffer->size);
            break;
    }
}

void sf_buffer_clear(sf_buffer *buffer) {
    free(buffer->ptr);
    buffer->ptr = buffer->head = nullptr;
    buffer->size = 0;
    buffer->flags |= SF_BUFFER_EMPTY;
}

sf_result sf_buffer_read(sf_buffer *buffer, void *dest, const size_t bytes) {
    if ((uint64_t)(buffer->ptr + buffer->size - buffer->head) < bytes)
        return sf_err(sf_lit("Out of bounds read at buffer write head."));

    memcpy(dest, buffer->head, bytes);
    buffer->head += bytes;
    return sf_ok();
}