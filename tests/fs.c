#include <assert.h>
#include <stdlib.h>
#include "sf/fs.h"
#include "sf/str.h"

int main(void) {
    long size = sf_file_size(sf_lit("CMakeLists.txt"));
    assert(size > 0);

    uint8_t *file = malloc((size_t)size);
    sf_result res = sf_load_file(file, sf_lit("CMakeLists.txt"));
    assert(res.ok);

    free(file);
}