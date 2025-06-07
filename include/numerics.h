#ifndef NUMERICS_H
#define NUMERICS_H

#include "str.h"

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
/// Constructs a string representation of a vec2.
[[nodiscard(SF_STR_LEAKED)]]
inline sf_str sf_vec2_str(const sf_vec2 vec2) { return sf_str_fmt("{ %f, %f }", (double)vec2.x, (double)vec2.y); }

/// A single point in 3d space.
typedef struct {
    float x, y, z;
} sf_vec3;
/// String representation of a vec3.
[[nodiscard(SF_STR_LEAKED)]]
inline sf_str sf_vec3_str(const sf_vec3 vec3) { return sf_str_fmt("{ %f, %f, %f }", (double)vec3.x, (double)vec3.y, (double)vec3.z); }

/// A representation of a transformation in 3d space.
typedef struct {
    sf_vec3 position;
    sf_vec3 rotation;
    sf_vec3 scale;
} sf_transform;
/// String representation of a transform.
[[nodiscard(SF_STR_LEAKED)]]
EXPORT sf_str sf_transform_str(const sf_transform transform);

/// Generates random bytes at the buffer specified.
EXPORT void sf_rand(uint8_t *buffer, size_t size);
/// Generates a random float in the specified inclusive range.
EXPORT float sf_randf(float min, float max);
/// Generates a random integer in the specified inclusive range.
EXPORT int64_t sf_randi(int64_t min, int64_t max);
/// Generates a random unsigned integer in the specified inclusive range.
EXPORT uint64_t sf_randu(uint64_t min, uint64_t max);

#endif // NUMERICS_H
