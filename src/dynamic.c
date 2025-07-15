#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "sf/dynamic.h"
#include "sf/numerics.h"

sf_key_value *sf_push_kv(sf_key_value *list, sf_key_value *new) {
    new->next = list;
    return new;
}

sf_map sf_map_new(void) {
    return (sf_map) {
        .bucket_count = 0,
        .pair_count = 0,
        .buckets = nullptr,
    };
}

void sf_map_delete(sf_map *map) {
    sf_map_clear(map);
    free(map->buckets);
}

void sf_map_clear(sf_map *map) {
    for (uint64_t i = 0; i < map->bucket_count; ++i) {
        const sf_key_value *pair = map->buckets[i];
        map->buckets[i] = nullptr;

        while (pair) {
            const sf_key_value *next = pair->next;
            free(pair->key.buffer);
            free((void *)pair->value.pointer);
            pair = next;
        }
    }

    if (map->bucket_count > SF_MAP_DEFAULT_BUCKETS) {
        map->bucket_count = SF_MAP_DEFAULT_BUCKETS;
        free(map->buckets);
        map->buckets = sf_calloc(map->bucket_count, sizeof(sf_key_value *));
    }
}

double sf_map_load(const sf_map *map, const uint64_t bucket_count) {
    return (double)map->pair_count / (double)bucket_count;
}

void sf_map_rehash(sf_map *map, const uint64_t new_bucket_count) {
    sf_key_value *pairs = nullptr;
    for (uint64_t i = 0; i < map->bucket_count; ++i) {
        sf_key_value *pair = map->buckets[i];
        sf_key_value *p_last = nullptr;
        map->buckets[i] = nullptr;

        while (pair) {
            if (!pairs)
                pairs = pair;
            if (p_last)
                p_last->next = pair;

            p_last = pair;
            pair = pair->next;
        }
        if (p_last) p_last->next = nullptr;
    }

    sf_key_value **new_buffer = sf_realloc(map->buckets, new_bucket_count * sizeof(sf_key_value *));
    map->bucket_count = new_bucket_count;
    map->buckets = new_buffer;

    while (pairs) {
        sf_key_value *next = pairs->next;
        const uint32_t hash = sf_fnv1a(pairs->key.buffer, pairs->key.size) & map->bucket_count - 1;
        map->buckets[hash] = sf_push_kv(map->buckets[hash], pairs);
        pairs = next;
    }
}

void sf_map_insert(sf_map *map, const sf_map_key key, const void *value, const size_t size) {
    if (!map->buckets || !map->bucket_count) {
        map->buckets = sf_calloc(SF_MAP_DEFAULT_BUCKETS, sizeof(sf_key_value *));
        map->bucket_count = SF_MAP_DEFAULT_BUCKETS;
    }

    if (sf_map_exists(map, key))
        sf_map_remove(map, key);

    void *value_copy = sf_malloc(size);
    memcpy(value_copy, value, size);

    const uint32_t hash = sf_fnv1a(key.buffer, key.size) & map->bucket_count - 1;
    sf_key_value *pair = sf_malloc(sizeof(sf_key_value));
    memcpy(pair, &(sf_key_value) {
        .key = {
            .buffer = sf_memdup(key.buffer, key.size),
            .size = key.size,
        },
        .value = {
            .pointer = value_copy,
            .size = size
        },
    }, sizeof(sf_key_value));
    map->buckets[hash] = sf_push_kv(map->buckets[hash], pair);
    map->pair_count++;
}

void *sf_map_get(const sf_map *map, const sf_map_key key) {
    assert(map->buckets && "Map is not initialized.");
    const uint32_t hash = sf_fnv1a(key.buffer, key.size) & map->bucket_count - 1;

    const sf_key_value *seek = map->buckets[hash];
    while (seek) {
        if (key.size == seek->key.size && memcmp(key.buffer, seek->key.buffer, key.size) == 0)
            break;
        seek = seek->next;
    }

    assert(seek != nullptr && "Map entry does not exist.");
    if (!seek)
        return nullptr;

    return seek->value.pointer;
}

void sf_map_remove(sf_map *map, const sf_map_key key) {
    const uint32_t hash = sf_fnv1a(key.buffer, key.size) & map->bucket_count - 1;

    sf_key_value *seek = map->buckets[hash];
    sf_key_value *seek_p = nullptr;
    while (seek) {
        if (key.size == seek->key.size && memcmp(key.buffer, seek->key.buffer, key.size) == 0) {
            if (seek_p)
                seek_p->next = seek->next;
            free(seek->key.buffer);
            free(seek->value.pointer);
            free(seek);
            break;
        }
        seek_p = seek;
        seek = seek->next;
    }
    map->pair_count--;
}

bool sf_map_exists(const sf_map *map, const sf_map_key key) {
    if (map->bucket_count == 0)
        return false;

    const uint32_t hash = sf_fnv1a(key.buffer, key.size) & map->bucket_count - 1;
    const sf_key_value *seek = map->buckets[hash];
    while (seek) {
        if (key.size == seek->key.size && memcmp(key.buffer, seek->key.buffer, key.size) == 0)
            break;
        seek = seek->next;
    }

    return seek ? true : false;
}

void sf_map_foreach(const sf_map *map, void (*func)(void *ud, sf_key_value *pair), void *ud) {
    for (uint64_t i = 0; i < map->bucket_count; ++i) {
        sf_key_value *p = map->buckets[i];
        while (p) {
            func(ud, p);
            p = p->next;
        }
    }
}

sf_vec _sf_vec_new(const size_t element_size) {
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

void sf_vec_push(sf_vec *vec, const void *data) {
    if (!vec->data || !vec->count || !vec->slots) {
        vec->data = sf_calloc(SF_VEC_INITIAL_SIZE, vec->element_size);
        vec->slots = SF_VEC_INITIAL_SIZE;
    }

    if (vec->count == vec->slots) // Vector is full, double size.
        vec->data = sf_realloc(vec->data, (vec->slots *= 2) * vec->element_size);

    memcpy(vec->data + vec->count * vec->element_size, data, vec->element_size);
    vec->count++;

    vec->top = vec->data + vec->count;
}

void sf_vec_append(sf_vec *vec, const void *data, const uint64_t count) {
    const size_t new_size = vec->count + count + 7 & ~(uint64_t)7; // Round to nearest greater multiple of 8.
    if (new_size > vec->slots || !vec->data) {
        vec->slots = new_size;
        vec->data = vec->data ? sf_realloc(vec->data, new_size * vec->element_size) : sf_calloc(new_size, vec->element_size);
    }

    memcpy(vec->data + vec->count *  vec->element_size, data, count * vec->element_size);
    vec->count += count;
    vec->top = vec->data + vec->count *  vec->element_size;
}

void *sf_vec_pop(sf_vec *vec) {
    assert(vec->count > 0 && "Vec is empty.");
    vec->count--;

    uint8_t *new_data = sf_malloc(vec->element_size);
    memcpy(new_data, vec->data + vec->count * vec->element_size, vec->element_size);

    if (vec->slots > SF_VEC_INITIAL_SIZE && vec->count <= vec->slots / 2) // Reduce size if possible
        vec->data = sf_realloc(vec->data, (vec->slots /= 2) * vec->element_size);

    vec->top = vec->data + vec->count;
    return new_data;
}

void sf_vec_insert(sf_vec *vec, const uint64_t index, const void *data) {
    assert(index <= vec->count && "Index out of bounds of vec.");
    if (vec->count == vec->slots) // Vector is full, double size.
        vec->data = sf_realloc(vec->data, (vec->slots *= 2) * vec->element_size);

    if (index == vec->count) {
        sf_vec_push(vec, data);
        return;
    }
    vec->count++;

    memcpy(vec->data + (index + 1) * vec->element_size, vec->data + index * vec->element_size, vec->element_size * (vec->count - index - 1));
    memcpy(vec->data + index * vec->element_size, data, vec->element_size);

    vec->top = vec->data + vec->count;
}

void sf_vec_set(const sf_vec *vec, const uint64_t index, const void *data) {
    assert(index < vec->count && "Index out of bounds of vec.");
    memcpy(vec->data + index * vec->element_size, data, vec->element_size);
}

const void *_sf_vec_get(const sf_vec *vec, const uint64_t index) {
    assert(index < vec->count && "Index out of bounds of vec.");
    return vec->data + index * vec->element_size;
}

void sf_vec_remove(sf_vec *vec, const uint64_t index) {
    assert(index < vec->count && "Index out of bounds of vec.");
    vec->count--;
    if (vec->count - index > 0)
        memcpy(vec->data + index * vec->element_size, vec->data + (index + 1) * vec->element_size, (vec->count - index) * vec->element_size);
    if (vec->slots > SF_VEC_INITIAL_SIZE && vec->count <= vec->slots / 2) // Reduce size if possible
        vec->data = sf_realloc(vec->data, vec->slots /= 2);
    vec->top = vec->data + vec->count;
}

sf_buffer sf_buffer_fixed(const size_t size) {
    uint8_t *dat = sf_calloc(1, size);
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
            buffer->ptr = sf_malloc(size);
            buffer->head = buffer->ptr;
            buffer->flags &= ~SF_BUFFER_EMPTY;
        } else if (buffer->flags & SF_BUFFER_GROW) {
            const long long ofs = buffer->head - buffer->ptr;
            buffer->size += size - offset;
            buffer->ptr = sf_realloc(buffer->ptr, buffer->size);
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
