#include <stdarg.h>
#include <stdio.h>
#include "sf/str.h"

#pragma clang diagnostic ignored "-Wformat-nonliteral"

sf_str sf_str_fmt(const char *format, ...) {
    va_list arglist;

    va_start(arglist, format);
    unsigned long long size =
        (unsigned long long)vsnprintf(NULL, 0, format, arglist);
    va_end(arglist);

    char *fmt = calloc(1, size + 1);
    va_start(arglist, format);
    vsnprintf(fmt, size + 1, format, arglist);
    va_end(arglist);

    return (sf_str) {
        .c_str = fmt,
        .len = size,
        .flags = SF_STR_NONE,
    };
}

sf_str sf_str_join(const sf_str str1, const sf_str str2) {
    size_t s = str1.len + str2.len;
    sf_str new_str = {
        .c_str = malloc(s + 1),
        .len = s,
    };
    memcpy(new_str.c_str, str1.c_str, str1.len);
    memcpy(new_str.c_str + str1.len, str2.c_str, str2.len);
    new_str.c_str[s] = '\0';

    return new_str;
}

sf_str sf_str_dup(const sf_str string) {
    sf_str new_str = { .c_str = calloc(1, string.len + 1), .len = string.len };
    memcpy(new_str.c_str, string.c_str, string.len);
    return new_str;
}

int sf_str_cmp(const sf_str str1, const sf_str str2) {
    char *a = str1.c_str, *b = str2.c_str;
    for (; *a && *b && *a == *b; ++a, ++b);
    return *b - *a;
}
