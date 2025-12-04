#include <stdio.h>
#include <sys/stat.h>
#include "sf/fs.h"
#include "sf/containers/buffer.h"

long sf_file_size(const sf_str path) {
    struct stat s;
    if (stat(path.c_str, &s) == -1)
        return -1;
    return s.st_size;
}

sf_fs_ex sf_load_file(uint8_t *out, const sf_str path) {
    const long size = sf_file_size(path);
    if (size < 0)
        return sf_fs_ex_err(SF_FILE_NOT_FOUND);
    FILE *f = fopen(path.c_str, "rb");
    if (!f)
        return sf_fs_ex_err(SF_OPEN_FAILURE);

    // Read into buffer
    if (fread(out, (unsigned long long)size, 1, f) < 0) {
        fclose(f);
        return sf_fs_ex_err(SF_READ_FAILURE);
    }
    fclose(f);

    return sf_fs_ex_ok();
}

sf_fsb_ex sf_file_buffer(sf_str path) {
    const size_t size = (size_t)sf_file_size(path);
    if (size < 0)
        return sf_fsb_ex_err(SF_FILE_NOT_FOUND);
    FILE *f = fopen(path.c_str, "rb");
    if (!f)
        return sf_fsb_ex_err(SF_OPEN_FAILURE);
    sf_buffer out = sf_buffer_fixed(size);

    // Read into buffer
    if (fread(out.ptr, size, 1, f) < 0) {
        fclose(f);
        sf_buffer_clear(&out);
        return sf_fsb_ex_err(SF_READ_FAILURE);
    }
    fclose(f);

    return sf_fsb_ex_ok(out);
}
