#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "dynamic.h"
#include "numerics.h"

#define PRIME 0x01000193
#define SEED 0x811C9DC5

sf_key_value *sf_push_kv(sf_key_value *list, sf_key_value *new) {
    new->next = list;
    return new;
}

static uint32_t sf_fnv1a(const void *data, size_t size, uint32_t hash) {
    assert(data);
    const unsigned char *head = data;
    while (size--) {
        auto cc = *head++;
        hash = (cc ^ hash) * PRIME;
    }
    return hash;
}

sf_map sf_map_new(void) {
    return (sf_map) {
        .bucket_count = 0,
        .pair_count = 0,
        .buckets = nullptr,
    };
}

void sf_map_delete(sf_map *self) {
    sf_map_clear(self);
    free(self->buckets);
}

void sf_map_clear(sf_map *self) {
    for (uint64_t i = 0; i < self->bucket_count; ++i) {
        sf_key_value *pair = self->buckets[i];
        sf_key_value *next = nullptr;
        self->buckets[i] = nullptr;

        while (pair) {
            next = pair->next;
            sf_str_free(pair->key);
            free((void *)pair->value.pointer);
            pair = next;
        }
    }

    if (self->bucket_count > SF_MAP_DEFAULT_BUCKETS) {
        self->bucket_count = SF_MAP_DEFAULT_BUCKETS;
        self->buckets = realloc(self->buckets, self->bucket_count);
    }
}

double sf_map_load(sf_map *self, uint64_t bucket_count) {
    return (double)self->pair_count / (double)bucket_count;
}

void sf_map_rehash(sf_map *self, uint64_t new_bucket_count) {
    sf_key_value *pairs = nullptr;
    for (uint64_t i = 0; i < self->bucket_count; ++i) {
        sf_key_value *pair = self->buckets[i];
        self->buckets[i] = nullptr;

        while (pair) {
            if (pairs)
                pairs->next = pair;

            pairs = pair;
            pair = pair->next;
        }
    }

    self->bucket_count = new_bucket_count;
    self->buckets = realloc(self->buckets, self->bucket_count);
    while (pairs) {
        uint32_t hash = sf_fnv1a(pairs->key.c_str, pairs->key.len, SEED) & (self->bucket_count - 1);
        self->buckets[hash] = sf_push_kv(self->buckets[hash], pairs);
    }
}

void _sf_map_insert(sf_map *self, const sf_str key, void *value, size_t size) {
    if (!self->buckets || !self->bucket_count) {
        self->buckets = calloc(SF_MAP_DEFAULT_BUCKETS, sizeof(sf_key_value *));
        self->bucket_count = SF_MAP_DEFAULT_BUCKETS;
    }

    if (sf_map_exists(self, key))
        sf_map_remove(self, key);

    void *value_copy = malloc(size);
    memcpy(value_copy, value, size);

    uint32_t hash = sf_fnv1a(key.c_str, key.len, SEED) & (self->bucket_count - 1);
    sf_key_value *pair = malloc(sizeof(sf_key_value));
    memcpy(pair, &(sf_key_value) {
        .key = sf_str_dup(key),
        .value = {
            .pointer = value_copy,
            .size = size
        },
    }, sizeof(sf_key_value));
    self->buckets[hash] = sf_push_kv(self->buckets[hash], pair);
    self->pair_count++;
}

const void *_sf_map_get(sf_map *self, const sf_str key) {
    assert(self->buckets && "Map is not initialized.");
    uint32_t hash = sf_fnv1a(key.c_str, key.len, SEED) & (self->bucket_count - 1);

    sf_key_value *seek = self->buckets[hash];
    while (seek) {
        if (sf_str_eq(key, seek->key))
            break;
        seek = seek->next;
    }

    assert(seek != nullptr && "Map entry does not exist.");

    return seek->value.pointer;
}

void sf_map_remove(sf_map *self, const sf_str key) {
    uint32_t hash = sf_fnv1a(key.c_str, key.len, SEED) & (self->bucket_count - 1);

    sf_key_value *seek = self->buckets[hash];
    sf_key_value *seek_p = nullptr;
    while (seek) {
        if (sf_str_eq(key, seek->key)) {
            if (seek_p)
                seek_p->next = seek->next;
            free(seek);
        }
        seek_p = seek;
        seek = seek->next;
    }
    self->pair_count--;
}

bool sf_map_exists(sf_map *self, const sf_str key) {
    if (self->bucket_count == 0)
        return false;

    uint32_t hash = sf_fnv1a(key.c_str, key.len, SEED) & (self->bucket_count - 1);
    sf_key_value *seek = self->buckets[hash];
    while (seek) {
        if (sf_str_eq(key, seek->key))
            break;
        seek = seek->next;
    }

    return seek ? true : false;
}

void sf_map_foreach(sf_map *self, void (*func)(void *ud, sf_key_value *pair), void *ud) {
    for (uint64_t i = 0; i < self->bucket_count; ++i) {
        sf_key_value *p = self->buckets[i];
        while (p) {
            func(ud, p);
            p = p->next;
        }
    }
}

sf_vec _sf_vec_new(size_t element_size) {
    return (sf_vec) {
        .element_size = element_size,
        .slots = 0,
        .count = 0,
        .data = nullptr,
    };
}

void sf_vec_delete(sf_vec *vec) {
    free(vec->data);
    vec->slots = 0;
    vec->count = 0;
    vec->data = nullptr;
}

void sf_vec_push(sf_vec *vec, void *data) {
    if (!vec->data || !vec->count || !vec->slots) {
        vec->data = calloc(SF_VEC_INITIAL_SIZE, vec->element_size);
        vec->slots = SF_VEC_INITIAL_SIZE;
    }

    if (vec->count == vec->slots) // Vector is full, double size.
        vec->data = realloc(vec->data, (vec->slots *= 2) * vec->element_size);

    memcpy(vec->data + vec->count * vec->element_size, data, vec->element_size);
    vec->count++;

    vec->top = vec->data + vec->count;
}

void sf_vec_append(sf_vec *vec, void *data, uint64_t count) {
    size_t new_size = (vec->count + count + 7) & ~(uint64_t)7; // Round to nearest greater multiple of 8.
    if (new_size > vec->slots) {
        vec->slots = new_size;
        vec->data = vec->data ? realloc(vec->data, new_size) : calloc(1, new_size);
    }

    memcpy(vec->data + vec->count *  vec->element_size, data, count * vec->element_size);
    vec->count += count;
    vec->top = vec->data + vec->count *  vec->element_size;
}

void *sf_vec_pop(sf_vec *vec) {
    assert(vec->count > 0 && "Vec is empty.");
    vec->count--;

    uint8_t *new_data = malloc(vec->element_size);
    memcpy(new_data, vec->data + vec->count * vec->element_size, vec->element_size);

    if (vec->slots > SF_VEC_INITIAL_SIZE && vec->count <= vec->slots / 2) // Reduce size if possible
        vec->data = realloc(vec->data, vec->slots /= 2);

    vec->top = vec->data + vec->count;
    return new_data;
}

void sf_vec_insert(sf_vec *vec, uint64_t index, void *data) {
    assert(index <= vec->count && "Index out of bounds of vec.");
    if (vec->count == vec->slots) // Vector is full, double size.
        vec->data = realloc(vec->data, (vec->slots *= 2) * vec->element_size);
    vec->count++;

    if (index == vec->count - 1) {
        sf_vec_push(vec, data);
        return;
    }

    memcpy(vec->data + (index + 1) * vec->element_size, vec->data + index * vec->element_size, vec->element_size * (vec->count - index - 1));
    memcpy(vec->data + index * vec->element_size, data, vec->element_size);

    vec->top = vec->data + vec->count;
}

void sf_vec_set(sf_vec *vec, uint64_t index, void *data) {
    assert(index < vec->count && "Index out of bounds of vec.");
    memcpy(vec->data + index * vec->element_size, data, vec->element_size);
}

const void *_sf_vec_get(sf_vec *vec, uint64_t index) {
    assert(index < vec->count && "Index out of bounds of vec.");
    return vec->data + index * vec->element_size;
}

void sf_vec_remove(sf_vec *vec, uint64_t index) {
    assert(index < vec->count && "Index out of bounds of vec.");
    vec->count--;
    if (vec->count - index > 0)
        memcpy(vec->data + index * vec->element_size, vec->data + (index + 1) * vec->element_size, vec->count - index);
    if (vec->slots > SF_VEC_INITIAL_SIZE && vec->count <= vec->slots / 2) // Reduce size if possible
        vec->data = realloc(vec->data, vec->slots /= 2);
    vec->top = vec->data + vec->count;
}

sf_buffer sf_buffer_fixed(size_t size) {
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

sf_result sf_buffer_insert(sf_buffer *buffer, const void *const ptr, size_t bytes) {
    uint64_t offset = (uint64_t)(buffer->ptr + buffer->size - buffer->head);
    if (offset < bytes) {
        if (buffer->flags & SF_BUFFER_EMPTY) {
            buffer->size = bytes;
            buffer->ptr = malloc(bytes);
            buffer->head = buffer->ptr;
            buffer->flags &= ~SF_BUFFER_EMPTY;
        } else if (buffer->flags & SF_BUFFER_GROW) {
            long long ofs = buffer->head - buffer->ptr;
            buffer->size += (bytes - offset);
            buffer->ptr = realloc(buffer->ptr, buffer->size);
            buffer->head = buffer->ptr + ofs;
        } else return sf_err(sf_lit("Buffer is fixed size and head doesn't have space to write."));
    }

    memcpy(buffer->head, ptr, bytes);
    buffer->head += bytes;
    return sf_ok();
}

void sf_buffer_seek(sf_buffer *buffer, sf_buffer_handle handle, int64_t offset) {
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

sf_result sf_buffer_read(sf_buffer *buffer, void *dest, size_t bytes) {
    if ((uint64_t)(buffer->ptr + buffer->size - buffer->head) < bytes)
        return sf_err(sf_lit("Out of bounds read at buffer write head."));

    memcpy(dest, buffer->head, bytes);
    buffer->head += bytes;
    return sf_ok();
}
