#include <assert.h>

#define MAP_NAME sf_str_map
#define MAP_K char
#define MAP_V int
#include "sf/containers/map.h"

int main() {
    sf_str_map map = sf_str_map_new();

    int a;
    sf_str_map_set(&map, 'a', 4);
    assert(sf_str_map_get(&map, 'a', &a) && a == 4);
    sf_str_map_set(&map, 'b', 6);
    assert(sf_str_map_get(&map, 'b', &a) && a == 6);
    sf_str_map_set(&map, 'c', 8);
    assert(sf_str_map_get(&map, 'c', &a) && a == 8);

    sf_str_map_free(&map);
}