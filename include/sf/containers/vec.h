#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "sanitizers.h"

#pragma GCC diagnostic ignored "-Wunused-function"

/***********************************
 * You should #define VEC_T as a value type,
 * #define VEC_NAME as the desired type name for the vec.
 * Optionally, define
 * - void (*CLEANUP_FN)(MAP_NAME *)
 * - type VSIZE_T
***********************************/

#ifndef VEC_NAME
#error Undefined typename VEC_NAME
#define VEC_NAME sf_vec
#endif
#ifndef VEC_T
#error Undefined type VEC_T
#define VEC_T void *
#endif

#ifndef VSIZE_T
#define VSIZE_T size_t
#endif

#define CAT(a, b) a##b
#define EXPAND_CAT(a, b) CAT(a, b)
#define FUNC(name) EXPAND_CAT(VEC_NAME, _##name)

#define INITIAL_SIZE 4

/// A generic dynamic vec. Be aware that data may move around the heap,
/// and the size of the vec may not always be equal to the amount of
/// elements in it.
typedef struct VEC_NAME {
    VSIZE_T slots; /// The amount of currently available slots.
    VSIZE_T count; /// The amount of currently used slots.
    VEC_T *data;
    VEC_T *top;
} VEC_NAME;

/// Create a new vec.
/// Note that vecs are lazily allocated.
static inline VEC_NAME FUNC(new)(void) {
    return (VEC_NAME) {
        .slots = 0,
        .count = 0,
        .data = NULL,
        .top = NULL,
    };
}
/// Allocate a new vec.
/// Differs from new in that it explicitly allocates `count` elements.
/// Initializes all elements to `def`.
static inline VEC_NAME FUNC(alloc)(VSIZE_T count, VEC_T def) {
    VEC_NAME v = (VEC_NAME) {
        .slots = count,
        .count = count,
        .data = malloc(sizeof(VEC_T) * count),
        .top = NULL,
    };

    for (VSIZE_T i = 0; i < count; ++i)
        memcpy(v.data + i, &def, sizeof(VEC_T));
    v.top = v.data + count - 1;

    return v;
}
/// Clean up after a vec's resources.
static inline void FUNC(free)(VEC_NAME *vec) {
    #ifdef CLEANUP_FN
    CLEANUP_FN(vec);
    #endif
    free(vec->data);
    vec->slots = 0;
    vec->count = 0;
    vec->data = NULL;
    vec->top = NULL;
}
/// Push an element to the end of a vec.
static inline void FUNC(push)(VEC_NAME *vec, const VEC_T value) {
    if (!vec->data || !vec->count || !vec->slots) {
        __lsan_disable();
        vec->data = calloc(INITIAL_SIZE, sizeof(VEC_T));
        __lsan_enable();
        vec->slots = INITIAL_SIZE;
    }

    if (vec->count == vec->slots) // Vector is full, double size.
        vec->data = realloc(vec->data, (vec->slots *= 2) * sizeof(VEC_T));

    memcpy(vec->data + vec->count, &value, sizeof(VEC_T));
    vec->count++;

    vec->top = vec->count == 0 ? vec->data : vec->data + vec->count - 1;
}
/// Append elements to the end of a vec.
static inline void FUNC(append)(VEC_NAME *vec, const VEC_T *values, VSIZE_T size) {
    for (VSIZE_T i = 0; i < size; ++i)
        FUNC(push)(vec, values[i]);
}
/// Pop an element from the end of a vec.
static inline VEC_T FUNC(pop)(VEC_NAME *vec) {
    assert(vec->count > 0 && "Vec is empty.");
    if (vec->count == 0)
        return (VEC_T){0};

    vec->count--;
    VEC_T data = *(vec->data + vec->count);
    if (vec->slots > INITIAL_SIZE && vec->count <= vec->slots / 2) // Reduce size if possible
        vec->data = realloc(vec->data, (vec->slots /= 2) * sizeof(VEC_T));

    vec->top = vec->count == 0 ? vec->data : vec->data + vec->count - 1;
    return data;
}
/// Insert an element at a specified index.
static inline void FUNC(insert)(VEC_NAME *vec, const VSIZE_T index, const VEC_T value) {
    assert(index <= vec->count && "Index out of bounds of vec.");
    if (index > vec->count)
        return;
    if (vec->count == vec->slots) // Vector is full, double size.
        vec->data = realloc(vec->data, (vec->slots *= 2) * sizeof(VEC_T));

    if (index == vec->count) {
        FUNC(push)(vec, value);
        return;
    }
    vec->count++;

    memcpy(vec->data + (index + 1), vec->data + index, sizeof(VEC_T) * (vec->count - index - 1));
    memcpy(vec->data + index, &value, sizeof(VEC_T));

    vec->top = vec->count == 0 ? vec->data : vec->data + vec->count - 1;
}
/// Set the value at a specified index.
static inline void FUNC(set)(const VEC_NAME *vec, const VSIZE_T index, VEC_T data) {
    assert(index < vec->count && "Index out of bounds of vec.");
    if (index >= vec->count)
        return;
    vec->data[index] = data;
}
/// Get the value at a specified index.
static inline VEC_T FUNC(get)(const VEC_NAME *vec, const VSIZE_T index) {
    assert(index < vec->count && "Index out of bounds of vec.");
    if (index >= vec->count)
        return (VEC_T){0};
    return *(vec->data + index);
}
/// Delete the value at the specified index.
static inline void FUNC(delete)(VEC_NAME *vec, const VSIZE_T index) {
    assert(index < vec->count && "Index out of bounds of vec.");
    vec->count--;
    if (vec->count - index > 0)
        memcpy(vec->data + index, vec->data + (index + 1), (vec->count - index) * (sizeof(VEC_T)));
    if (vec->slots > INITIAL_SIZE && vec->count <= vec->slots / 2) // Reduce size if possible
        vec->data = realloc(vec->data, vec->slots /= 2);
    vec->top = vec->count == 0 ? vec->data : vec->data + vec->count - 1;
}

#undef VEC_NAME
#undef VEC_T

#undef CAT
#undef EXPAND_CAT
#undef FUNC
#ifdef CLEANUP_FN
#undef CLEANUP_FN
#endif
#ifdef VSIZE_T
#undef VSIZE_T
#endif
