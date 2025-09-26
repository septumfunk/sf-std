#include <assert.h>
#include "sf/math.h"

uint32_t sf_fnv1a(const void *data, size_t size) {
    const unsigned char *head = data;
    uint32_t hash = SF_FNV1A_SEED;
    while (size--) {
        const unsigned char cc = *head++;
        hash = (cc ^ hash) * SF_FNV1A_PRIME;
    }
    return hash;
}
