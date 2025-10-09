#ifndef NUMERICS_H
#define NUMERICS_H

#include <stddef.h>
#include <stdint.h>
#define SF_FNV1A_PRIME 0x01000193
#define SF_FNV1A_SEED 0x811C9DC5

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

/// A single point in 2d space.
typedef struct {
    float x, y;
} sf_vec2;
/// Adds two vec2s together and returns the result.
static inline sf_vec2 sf_vec2_add(const sf_vec2 first, const sf_vec2 second) {
    return (sf_vec2){first.x + second.x, first.y + second.y};
}
/// Subtracts two vec2s and returns the result.
static inline sf_vec2 sf_vec2_sub(const sf_vec2 first, const sf_vec2 second) {
    return (sf_vec2){first.x - second.x, first.y - second.y};
}
/// Multiplies a vec2 by a factor of vec3.
static inline sf_vec2 sf_vec2_multv(const sf_vec2 first, const sf_vec2 factor) {
    return (sf_vec2){first.x * factor.x, first.y * factor.y};
}
/// Multiplies a vec2 by a factor of float.
static inline sf_vec2 sf_vec2_multf(const sf_vec2 first, const float factor) {
    return (sf_vec2){first.x * factor, first.y * factor};
}

/// A single point in 3d space.
typedef struct {
    float x, y, z;
} sf_vec3;
/// Adds two vec3s together and returns the result.
static inline sf_vec3 sf_vec3_add(const sf_vec3 first, const sf_vec3 second) {
    return (sf_vec3){first.x + second.x, first.y + second.y, first.z + second.z};
}
/// Subtracts two vec3s and returns the result.
static inline sf_vec3 sf_vec3_sub(const sf_vec3 first, const sf_vec3 second) {
    return (sf_vec3){first.x - second.x, first.y - second.y, first.z - second.z};
}
/// Multiplies a vec3 by a factor of vec3.
static inline sf_vec3 sf_vec3_multv(const sf_vec3 first, const sf_vec3 factor) {
    return (sf_vec3){first.x * factor.x, first.y * factor.y, first.z * factor.z};
}
/// Multiplies a vec3 by a factor of float.
static inline sf_vec3 sf_vec3_multf(const sf_vec3 first, const float factor) {
    return (sf_vec3){first.x * factor, first.y * factor, first.z * factor};
}

/// A representation of a transformation in 3d space.
typedef struct sf_transform {
    sf_vec3 position;
    sf_vec3 rotation;
    sf_vec3 scale;

    struct sf_transform *parent;
} sf_transform;
#define SF_TRANSFORM_IDENTITY ((sf_transform){{0, 0, 0}, {0, 0, 0}, {1, 1, 1}, NULL})

/// Hash a buffer of `size` bytes with the fnv1a algorithm.
uint32_t sf_fnv1a(const void *data, size_t size);

/// Generates random bytes at the buffer specified.
//TODO: EXPORT void sf_rand(uint8_t *buffer, size_t size);
/// Generates a random float in the specified inclusive range.
//TODO: EXPORT float sf_randf(float min, float max);
/// Generates a random integer in the specified inclusive range.
//TODO: EXPORT int64_t sf_randi(int64_t min, int64_t max);
/// Generates a random unsigned integer in the specified inclusive range.
//TODO: EXPORT uint64_t sf_randu(uint64_t min, uint64_t max);

#endif // NUMERICS_H
