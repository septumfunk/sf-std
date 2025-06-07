#ifndef DYNAMIC_H
#define DYNAMIC_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sf/str.h"
#include "sf/result.h"

#define SF_MAP_DEFAULT_BUCKETS 8

/// A key/value pair that contains size information and a possible reference to a chain of pairs.
typedef struct sf_key_value {
    const sf_str key; /// The key which this pair is indexed by.
    const struct {
        const void *const pointer; /// A read only pointer to the data on the heap.
        const size_t size; /// The size of the memory block pointed to.
    } value; /// Contains information about the value contained by this pair.

    struct sf_key_value *next; /// Can either point to the next pair in a series of conflicts, or `nullptr` if there is none.
} sf_key_value;

/// A fast and simple hashmap indexed by strings which may contain a value of any type.
typedef struct {
    uint64_t bucket_count; /// The amount of currently available buckets. Will expand to reduce conflicts as the map grows.
    uint64_t pair_count; /// The amount of pairs currently held within the map.
    sf_key_value **buckets; /// A pointer to the first bucket in an array of them.
} sf_map;

/// Create a new empty map with the default amount of buckets `SF_MAP_DEFAULT_BUCKETS 8`
[[nodiscard("sf_map leaked")]]
EXPORT sf_map sf_map_new(void);
/// Delete and clean up after a map and all of its pairs.
EXPORT void sf_map_delete(sf_map *self);
/// Clear a map's pairs and reset it to its default state `SF_MAP_DEFAULT_BUCKETS 8`
EXPORT void sf_map_clear(sf_map *self);
/// Insert a key/value pair into the map. The value's size is specified by the `size` parameter.
/// It's recommended to use the convenience macro, `sf_map_insert`.
EXPORT void _sf_map_insert(sf_map *self, const sf_str key, void *value, size_t size);
#define sf_map_insert(self, key, value) _sf_map_insert(self, key, value, sizeof(*value))
/// C-String alternative to _sf_map_insert.
/// It's recommended to use the convenience macro, `sf_map_cinsert`.
static inline void _sf_map_cinsert(sf_map *self, const char *key, void *value, size_t size) { _sf_map_insert(self, sf_lit(key), value, size); }
#define sf_map_cinsert(self, key, value) _sf_map_cinsert(self, key, value, sizeof(*value))
/// Get a value from a map by its key.
/// It's recommended to use the convenience macro, `sf_map_get`.
EXPORT const void *_sf_map_get(sf_map *self, const sf_str key);
#define sf_map_get(self, type, key) (*(type *)_sf_map_get(self, key))
/// C-String alternative to _sf_map_get.
/// It's recommended to use the convenience macro, `sf_map_cget`.
static inline const void *_sf_map_cget(sf_map *self, const char *key) { return _sf_map_get(self, sf_lit(key)); }
#define sf_map_cget(self, type, key) (*(type *)_sf_map_cget(self, key))
/// Check if a key has a corresponding value within the map.
EXPORT bool sf_map_exists(sf_map *self, const sf_str key);
static inline bool sf_map_cexists(sf_map *self, const char *key) { return sf_map_exists(self, sf_lit(key)); }
/// Remove and free a value from a map by its key.
EXPORT void sf_map_remove(sf_map *self, const sf_str key);
static inline void sf_map_cremove(sf_map *self, const char *key) { sf_map_remove(self, sf_lit(key)); }
/// Iterate over a map and call a custom handler function on each key/value pair.
EXPORT void sf_map_foreach(sf_map *self, void (*func)(void *ud, sf_key_value *pair), void *ud);

#define SF_VEC_INITIAL_SIZE 4

/// A generic dynamic vec. Be aware that data may move around the heap,
/// and the size of the vec may not always be equal to the amount of
/// elements in it.
typedef struct {
    size_t element_size; /// The size of an individual element in the vec.
    uint64_t slots; /// The amount of currently available slots.
    uint64_t count; /// The amount of currently used slots.
    uint8_t *data; /// A series of sequential bytes representing vec elements.
    uint8_t *top; /// The byte directly after the very last element of the vector.
} sf_vec;

/// Creates a new vec with a specified element size.
[[nodiscard("sf_vec leaked")]]
EXPORT sf_vec _sf_vec_new(size_t element_size);
#define sf_vec_new(type) _sf_vec_new(sizeof(type))
/// Cleans up after a vec, deleting all elements.
EXPORT void sf_vec_delete(sf_vec *self);
/// Copies an element to the end of a vec.
EXPORT void sf_vec_push(sf_vec *self, void *data);
/// Copies an array of elements to the end of a vec.
EXPORT void sf_vec_append(sf_vec *self, void *data, uint64_t count);
/// Copies and then removes the last vec element, returning a pointer to its copy on the heap.
EXPORT void *sf_vec_pop(sf_vec *self);
/// Copies an element into the vec at a specified index, shifting all elements after it forward.
EXPORT void sf_vec_insert(sf_vec *self, uint64_t index, void *data);
/// Sets the value of an index of the vec directly.
EXPORT void sf_vec_set(sf_vec *self, uint64_t index, void *data);
/// Gets a value from the vec, returning a pointer to it.
/// This pointer may only be valid until the next vec operation is executed,
/// as a vec's data may not always stay in the same place.
EXPORT const void *_sf_vec_get(sf_vec *, uint64_t);
#define sf_vec_get(vec, type, index) (*(const type *const)_sf_vec_get(vec, index))
/// Removes an element from a vec at the specified index.
EXPORT void sf_vec_remove(sf_vec *self, uint64_t index);

/// Flags for determining a buffer's behavior.
typedef enum : uint8_t {
    SF_BUFFER_GROW     = 0b10000000,
    SF_BUFFER_EMPTY    = 0b01000000,
    SF_BUFFER_READONLY = 0b00100000,
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
[[nodiscard("sf_buffer leaked")]]
EXPORT sf_buffer sf_buffer_fixed(size_t size);
/// Allocate a buffer that grows as you insert bytes.
[[nodiscard("sf_buffer leaked")]]
EXPORT sf_buffer sf_buffer_grow();
/// Insert a value into a buffer.
[[nodiscard(SF_RESULT_LEAKED)]]
EXPORT sf_result sf_buffer_insert(sf_buffer *buffer, const void *const ptr, size_t size);
/// Insert a value into a buffer with automatic sizing.
#define sf_buffer_autoins(buffer, value) sf_buffer_insert(buffer, value, sizeof(*value))
/// Seek to a position in the buffer.
EXPORT void sf_buffer_seek(sf_buffer *buffer, sf_buffer_handle handle, int64_t offset);
/// Free a buffer and/or revert it to an empty state.
EXPORT void sf_buffer_clear(sf_buffer *buffer);
/// Copies x bytes from the buffer head to the specified location.
[[nodiscard(SF_RESULT_LEAKED)]]
EXPORT sf_result sf_buffer_read(sf_buffer *buffer, void *dest, size_t bytes);
/// Automatically read a value based on the type of the pointer.
/// `dest` must be a pointer.
#define sf_buffer_autoread(buffer, dest) sf_buffer_read(buffer, dest, sizeof(*dest))

#endif // DYNAMIC_H
