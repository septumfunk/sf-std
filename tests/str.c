#include <assert.h>
#include "sf/str.h"

int main(void) {
    sf_str twelve = sf_str_fmt("twelve{%d}", 12);
    assert(sf_str_eq(twelve, sf_lit("twelve{12}")));
    sf_str_free(twelve);
}