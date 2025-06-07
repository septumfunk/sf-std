#include <stdlib.h>
#include <time.h>
#include "sf/numerics.h"

sf_str sf_transform_str(const sf_transform transform) {
    sf_str pos_str = sf_vec3_str(transform.position);
    sf_str rot_str = sf_vec3_str(transform.rotation);
    sf_str sca_str = sf_vec3_str(transform.scale);
    sf_str tsf_str = sf_str_fmt("XYZ %s ROT %s SCALE %s", pos_str.c_str, rot_str.c_str, sca_str.c_str);

    sf_str_free(pos_str);
    sf_str_free(rot_str);
    sf_str_free(sca_str);
    return tsf_str;
}

static bool _is_rand_initialized = false;
void sf_rand(uint8_t *buffer, size_t size) {
    if (!_is_rand_initialized) {
        srand((unsigned long)time(nullptr));
        _is_rand_initialized = true;
    }
    for (size_t i = 0; i < size; ++i)
        buffer[i] = (uint8_t)(rand() & 0xFF);
}

float sf_randf(float min, float max) {
    float rand;
    sf_rand((uint8_t *)&rand, sizeof(rand));
    return min + rand * (max - min);
}

int64_t sf_randi(int64_t min, int64_t max) {
    int64_t rand;
    sf_rand((uint8_t *)&rand, sizeof(rand));
    return min + (rand % max);
}

uint64_t sf_randu(uint64_t min, uint64_t max) {
    uint64_t rand;
    sf_rand((uint8_t *)&rand, sizeof(rand));
    return min + (rand % max);
}
