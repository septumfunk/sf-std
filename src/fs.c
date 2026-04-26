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
    if (fread(out, (size_t)size, 1, f) < 1) {
        fclose(f);
        return sf_fs_ex_err(SF_READ_FAILURE);
    }
    fclose(f);

    return sf_fs_ex_ok();
}

sf_fsb_ex sf_file_buffer(sf_str path) {
    const long size = sf_file_size(path);
    if (size < 0)
        return sf_fsb_ex_err(SF_FILE_NOT_FOUND);
    FILE *f = fopen(path.c_str, "rb");
    if (!f)
        return sf_fsb_ex_err(SF_OPEN_FAILURE);
    sf_buffer out = sf_buffer_fixed((size_t)size);
    sf_buffer_seek(&out, SF_BUFFER_START, 0);

    // Read into buffer
    if (fread(out.ptr, (size_t)size, 1, f) < 1) {
        fclose(f);
        sf_buffer_clear(&out);
        return sf_fsb_ex_err(SF_READ_FAILURE);
    }
    fclose(f);

    return sf_fsb_ex_ok(out);
}
