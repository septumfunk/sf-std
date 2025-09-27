#include <assert.h>
#include <string.h>
#include "sf/containers/buffer.h"
#include "sf/result.h"

int main(void) {
    sf_buffer buff = sf_buffer_fixed(1024);

    unsigned char bytes[1024] = { 4, 2, 0, 6, 9 };
    sf_result res = sf_buffer_insert(&buff, bytes, sizeof(bytes));
    assert(res.is_ok);
    sf_discard(res);
    assert(memcmp(buff.ptr, bytes, sizeof(bytes)) == 0);

    sf_buffer_clear(&buff);


    buff = sf_buffer_fixed(512);

    res = sf_buffer_insert(&buff, bytes, sizeof(bytes));
    assert(!res.is_ok);
    sf_discard(res);

    sf_buffer_clear(&buff);


    buff = sf_buffer_grow();

    res = sf_buffer_insert(&buff, bytes, sizeof(bytes));
    assert(res.is_ok);
    sf_discard(res);
    assert(buff.size == sizeof(bytes));

    sf_buffer_clear(&buff);


    buff = sf_buffer_own(bytes, sizeof(bytes));
    assert(memcmp(buff.ptr, bytes, sizeof(bytes)) == 0);
}