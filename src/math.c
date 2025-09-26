#include "sf/math.h"

#include <assert.h>

sf_str sf_transform_str(const sf_transform transform) {
    const sf_str pos_str = sf_vec3_str(transform.position);
    const sf_str rot_str = sf_vec3_str(transform.rotation);
    const sf_str sca_str = sf_vec3_str(transform.scale);
    const sf_str tsf_str = sf_str_fmt("XYZ %s ROT %s SCALE %s", pos_str.c_str, rot_str.c_str, sca_str.c_str);

    sf_str_free(pos_str);
    sf_str_free(rot_str);
    sf_str_free(sca_str);
    return tsf_str;
}

uint32_t sf_fnv1a(const void *data, size_t size) {
    const unsigned char *head = data;
    uint32_t hash = SF_FNV1A_SEED;
    while (size--) {
        const auto cc = *head++;
        hash = (cc ^ hash) * SF_FNV1A_PRIME;
    }
    return hash;
}

// static bool is_rand_initialized = false;
// void sf_rand([[maybe_unused]] uint8_t *buffer, [[maybe_unused]] const size_t size) {
//     if (!is_rand_initialized) {
//         //TODO: init
//         is_rand_initialized = true;
//     }
// }
//
// float sf_randf(float min, float max) {
//     float rand;
//     sf_rand((uint8_t *)&rand, sizeof(rand));
//     return min + rand * (max - min);
// }
//
// int64_t sf_randi(int64_t min, int64_t max) {
//     int64_t rand;
//     sf_rand((uint8_t *)&rand, sizeof(rand));
//     return min + (rand % max);
// }
//
// uint64_t sf_randu(uint64_t min, uint64_t max) {
//     uint64_t rand;
//     sf_rand((uint8_t *)&rand, sizeof(rand));
//     return min + (rand % max);
// }
