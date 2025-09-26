#include <assert.h>
#include "sf/str.h"

#define MAP_NAME map_ci
#define MAP_K char
#define MAP_V int
#include "sf/containers/map.h"

#define MAP_NAME map_ss
#define MAP_K sf_str
#define MAP_V sf_str
#define HASH_FN sf_str_hash
#define EQUAL_FN sf_str_eq
#include "sf/containers/map.h"

int main(void) {
    map_ci map = map_ci_new();

    int a;
    map_ci_set(&map, 'a', 4);
    assert(map_ci_get(&map, 'a', &a) && a == 4);
    map_ci_set(&map, 'b', 6);
    assert(map_ci_get(&map, 'b', &a) && a == 6);
    map_ci_set(&map, 'c', 8);
    assert(map_ci_get(&map, 'c', &a) && a == 8);

    map_ci_free(&map);

    map_ss map2 = map_ss_new();

    map_ss_set(&map2, sf_lit("test"), sf_lit("80085"));
    sf_str out = SF_STR_EMPTY;
    assert(map_ss_get(&map2, sf_lit("test"), &out) && sf_str_eq(sf_lit("80085"), out));

    map_ss_free(&map2);
}