#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#pragma GCC diagnostic ignored "-Wunused-function"

/***********************************
 * You should #define VEC_T as a value type,
 * #define VEC_NAME as the desired type name for the vec.
***********************************/

#ifndef VEC_NAME
#error "Please define typename VEC_NAME"
#define VEC_NAME sf_vec
#endif
#ifndef VEC_T
#error "Please define type VEC_T"
#define VEC_T void *
#endif

#define CAT(a, b) a##b
#define EXPAND_CAT(a, b) CAT(a, b)
#define FUNC(name) EXPAND_CAT(VEC_NAME, _##name)

#define INITIAL_SIZE 4

/// A generic dynamic vec. Be aware that data may move around the heap,
/// and the size of the vec may not always be equal to the amount of
/// elements in it.
typedef struct {
    size_t slots; /// The amount of currently available slots.
    size_t count; /// The amount of currently used slots.
    VEC_T *data;
    VEC_T *top;
} VEC_NAME;

/// Create a new vec.
/// Note that vecs are lazily allocated.
static VEC_NAME FUNC(new)(void) {
    return (VEC_NAME) {
        .slots = 0,
        .count = 0,
        .data = NULL,
        .top = NULL,
    };
}
/// Clean up after a vec's resources.
static void FUNC(free)(VEC_NAME *vec) {
    free(vec->data);
    vec->slots = 0;
    vec->count = 0;
    vec->data = NULL;
}
/// Push an element to the end of a vec.
static void FUNC(push)(VEC_NAME *vec, const VEC_T value) {
    if (!vec->data || !vec->count || !vec->slots) {
        vec->data = calloc(INITIAL_SIZE, sizeof(VEC_T));
        vec->slots = INITIAL_SIZE;
    }

    if (vec->count == vec->slots) // Vector is full, double size.
        vec->data = realloc(vec->data, (vec->slots *= 2) * sizeof(VEC_T));

    memcpy(vec->data + vec->count, &value, sizeof(VEC_T));
    vec->count++;

    vec->top = vec->data + vec->count - 1;
}
/// Pop an element from the end of a vec.
static VEC_T FUNC(pop)(VEC_NAME *vec) {
    assert(vec->count > 0 && "Vec is empty.");
    if (vec->count == 0)
        return (VEC_T){0};

    vec->count--;
    VEC_T data = *(vec->data + vec->count);
    if (vec->slots > INITIAL_SIZE && vec->count <= vec->slots / 2) // Reduce size if possible
        vec->data = realloc(vec->data, (vec->slots /= 2) * sizeof(VEC_T));

    vec->top = vec->data + vec->count - 1;
    return data;
}
/// Insert an element at a specified index.
static void FUNC(insert)(VEC_NAME *vec, const size_t index, const VEC_T value) {
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

    vec->top = vec->data + vec->count - 1;
}
/// Set the value at a specified index.
static void FUNC(set)(const VEC_NAME *vec, const size_t index, const void *data) {
    assert(index < vec->count && "Index out of bounds of vec.");
    if (index >= vec->count)
        return;
    memcpy(vec->data + index, data, sizeof(VEC_T));
}
/// Get the value at a specified index.
static VEC_T FUNC(get)(const VEC_NAME *vec, const size_t index) {
    assert(index < vec->count && "Index out of bounds of vec.");
    if (index >= vec->count)
        return (VEC_T){0};
    return *(vec->data + index);
}
/// Delete the value at the specified index.
static void FUNC(delete)(VEC_NAME *vec, const size_t index) {
    assert(index < vec->count && "Index out of bounds of vec.");
    vec->count--;
    if (vec->count - index > 0)
        memcpy(vec->data + index, vec->data + (index + 1), (vec->count - index) * (sizeof(VEC_T)));
    if (vec->slots > INITIAL_SIZE && vec->count <= vec->slots / 2) // Reduce size if possible
        vec->data = realloc(vec->data, vec->slots /= 2);
    vec->top = vec->data + vec->count - 1;
}

#undef VEC_NAME
#undef VEC_T