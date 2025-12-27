#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#pragma GCC diagnostic ignored "-Wunused-function"

/***********************************
 * You should #define MAP_K & MAP_V as key/value types,
 * #define MAP_NAME as the desired type name for the map.
 * Optionally, #define:
 * - uint32_t (*HASH_FN)(const MAP_K)
 * - bool (*EQUAL_FN)(const MAP_K, const MAP_K)
 * - void (*CLEANUP_FN)(MAP_NAME *)
***********************************/

#ifndef MAP_NAME
#error Undefined typename MAP_NAME
#define MAP_NAME sf_map
#endif
#ifndef MAP_K
#error Undefined type MAP_K
#define MAP_K void *
#endif
#ifndef MAP_V
#error Undefined type MAP_V
#define MAP_V void *
#endif

#define EXPECTED_NAME EXPAND_CAT(MAP_NAME, _ex)
#define EXPECTED_O MAP_V
#include "sf/containers/expected.h"

#define CAT(a, b) a##b
#define EXPAND_CAT(a, b) CAT(a, b)
#define FUNC(name) EXPAND_CAT(MAP_NAME, _##name)

#define DEFAULT_BUCKETS 8
#define SF_FNV1A_PRIME 0x01000193
#define SF_FNV1A_SEED 0x811C9DC5

#ifndef HASH_FN
/// Default fnv1a hashing function for keys.
static inline uint32_t FUNC(hash)(const MAP_K key) {
    const unsigned char *head = (unsigned char *)&key;
    size_t size = sizeof(MAP_K);
    uint32_t hash = SF_FNV1A_SEED;
    while (size--) {
        const unsigned char cc = *head++;
        hash = (cc ^ hash) * SF_FNV1A_PRIME;
    }
    return hash;
}
#define HASH_FN FUNC(hash)
#endif

/// A key/value pair with user defined types.
#define BUCKET EXPAND_CAT(MAP_NAME, _bucket)
typedef struct BUCKET {
    MAP_K key;
    MAP_V value;
    struct BUCKET *next;
} BUCKET;
/// Helper function for pushing buckets.
static inline BUCKET *FUNC(push_kv)(BUCKET *list, BUCKET *new) {
    new->next = list;
    return new;
}

/// A map that uses user defined types for keys/values.
typedef struct MAP_NAME {
    size_t bucket_count; /// Expands to reduce conflicts as the map grows.
    size_t pair_count; /// The amount of key/value pairs currently held within the map.
    BUCKET **buckets;
} MAP_NAME;

/// Creates the map with the specified type and name.
static inline MAP_NAME FUNC(new)(void) {
    return (MAP_NAME) {
        .bucket_count = DEFAULT_BUCKETS,
        .pair_count = 0,
        .buckets = calloc(DEFAULT_BUCKETS, sizeof(BUCKET *)),
    };
}
/// Clear a map, resetting it to the default state.
static inline void FUNC(clear)(MAP_NAME *map) {
    if (!map->buckets || !map->bucket_count)
        return;

    for (size_t i = 0; i < map->bucket_count; ++i) {
        BUCKET *pair = map->buckets[i];
        map->buckets[i] = NULL;

        while (pair) {
            BUCKET *next = pair->next;
            free(pair);
            pair = next;
        }
    }

    if (map->bucket_count > DEFAULT_BUCKETS) {
        map->bucket_count = DEFAULT_BUCKETS;
        free(map->buckets);
        map->buckets = calloc(map->bucket_count, sizeof(BUCKET *));
    }
}
/// Free all of a map's resources.
static inline void FUNC(free)(MAP_NAME *map) {
    #ifdef CLEANUP_FN
    CLEANUP_FN(map);
    #endif
    FUNC(clear)(map);
    free(map->buckets);
    map->buckets = NULL;
    map->bucket_count = 0;
    map->pair_count = 0;
}
/// Calculate the load of a map.
static inline double FUNC(load)(const MAP_NAME *map, const size_t bucket_count) {
    return (double)map->pair_count / (double)bucket_count;
}
/// Rehash a map when the load gets too high.
static inline void FUNC(rehash)(MAP_NAME *map, const size_t new_bucket_count) {
    if (!map->buckets || !map->bucket_count)
        return;
    BUCKET *pairs = NULL;
    BUCKET *p_last = NULL;
    for (size_t i = 0; i < map->bucket_count; ++i) {
        BUCKET *pair = map->buckets[i];
        map->buckets[i] = NULL;

        while (pair) {
            if (!pairs)
                pairs = pair;
            if (p_last)
                p_last->next = pair;

            p_last = pair;
            pair = pair->next;
        }
    }

    BUCKET **new_buffer = realloc(map->buckets, new_bucket_count * sizeof(BUCKET *));
    map->bucket_count = new_bucket_count;
    map->buckets = new_buffer;

    while (pairs) {
        BUCKET *next = pairs->next;
        const uint32_t hash = HASH_FN(pairs->key) & map->bucket_count - 1;
        map->buckets[hash] = FUNC(push_kv)(map->buckets[hash], pairs);
        pairs = next;
    }
}

#define EX EXPAND_CAT(MAP_NAME, _ex)
/// Returns whether the key exists or not and writes to `out` on success.
/// If `out` is null, simply return whether the key exists.
static inline EX FUNC(get)(const MAP_NAME *map, MAP_K key) {
    if (!map->buckets || !map->bucket_count)
        return EXPAND_CAT(EX, _err)();
    const uint32_t hash = HASH_FN(key) % map->bucket_count;

    const BUCKET *seek = map->buckets[hash];
    while (seek) {
        #ifndef EQUAL_FN
        if (key == seek->key)
            break;
        #else
        if (EQUAL_FN(key, seek->key))
            break;
        #endif
        seek = seek->next;
    }

    if (!seek)
        return EXPAND_CAT(EX, _err)();

    return EXPAND_CAT(EX, _ok)(seek->value);
}
/// Delete a value from a map by its key.
static inline void FUNC(delete)(MAP_NAME *map, MAP_K key) {
    if (!map->buckets || !map->bucket_count)
        return;

    const uint32_t hash = HASH_FN(key) % map->bucket_count;
    BUCKET *seek = map->buckets[hash];
    BUCKET *seek_p = NULL;
    while (seek) {
        #ifndef EQUAL_FN
        if (key == seek->key) {
        #else
        if (EQUAL_FN(key, seek->key)) {
        #endif
            if (seek_p)
                seek_p->next = seek->next;
            free(seek);
            map->pair_count--;
            break;
        }
        seek_p = seek;
        seek = seek->next;
    }
}
/// Set the value at the requested key, overriding any existing value.
static inline void FUNC(set)(MAP_NAME *map, MAP_K key, MAP_V value) {
    if (!map->buckets || !map->bucket_count)
        return;
    if (FUNC(get)(map, key).is_ok)
        FUNC(delete)(map, key);

    const uint32_t hash = HASH_FN(key) % map->bucket_count;
    BUCKET *pair = malloc(sizeof(BUCKET));
    memcpy(pair, &(BUCKET) {
        key,
        value,
        NULL
    }, sizeof(BUCKET));
    map->buckets[hash] = FUNC(push_kv)(map->buckets[hash], pair);
    map->pair_count++;

    if (FUNC(load)(map, map->bucket_count) > 0.75)
        FUNC(rehash)(map, map->bucket_count * 2);
}
/// Loop over a map's key/value pairs and execute custom code with them.
static inline void FUNC(foreach)(const MAP_NAME *map, void (*func)(void *ud, MAP_K key, MAP_V value), void *ud) {
    if (!map->buckets || !map->bucket_count)
        return;
    for (size_t i = 0; i < map->bucket_count; ++i) {
        BUCKET *p = map->buckets[i];
        while (p) {
            func(ud, p->key, p->value);
            p = p->next;
        }
    }
}

#undef MAP_NAME
#undef MAP_K
#undef MAP_V
#undef HASH_FN
#undef EQUAL_FN
#ifdef CLEANUP_FN
#undef CLEANUP_FN
#endif

#undef CAT
#undef EXPAND_CAT
#undef FUNC
