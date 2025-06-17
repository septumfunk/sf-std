#include <stdio.h>
#include <sys/stat.h>
#include "sf/fs.h"

long sf_file_size(const sf_str path) {
    struct stat s;
    if (stat(path.c_str, &s) == -1)
        return -1;
    return s.st_size;
}

sf_result sf_load_file(uint8_t *out, const sf_str path) {
    const long size = sf_file_size(path);
    if (size < 0)
        return sf_err(sf_str_fmt("Requested file '%s' could not be found.", path));
    FILE *f = fopen(path.c_str, "rb");
    if (!f)
        return sf_err(sf_str_fmt("Call to fopen on requested file '%s' failed.", path));

    // Read into buffer
    if (fread(out, (unsigned long long)size, 1, f) < 0) {
        fclose(f);
        return sf_err(sf_str_fmt("Requested file '%s' could not be read.", path));
    }
    fclose(f);

    return sf_ok();
}
