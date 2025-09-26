#include <assert.h>

#define VEC_NAME sf_vec_int
#define VEC_T int
#include "sf/containers/vec.h"

int main(void) {
    sf_vec_int vec = sf_vec_int_new();

    for (int i = 0; i < 5; ++i)
        sf_vec_int_push(&vec, i);
    for (int i = 0; i < 5; ++i) {
        int v = sf_vec_int_get(&vec, (uint64_t)i);
        assert(v == i);
    }

    sf_vec_int_insert(&vec, 3, 4);
    assert(sf_vec_int_get(&vec, 3) == 4);
    assert(sf_vec_int_get(&vec, vec.count - 1) == sf_vec_int_pop(&vec));

    sf_vec_int_free(&vec);
}