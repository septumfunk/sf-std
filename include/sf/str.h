#ifndef STRINGS_H
#define STRINGS_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "export.h"

#define SF_STR_LEAKED "sf_str_leaked"

/// Bitfield of flags to define aspects about a `sf_str`
typedef enum : uint8_t {
    SF_STR_NONE  = 0b00000000,
    SF_STR_EMPTY = 0b10000000,
    SF_STR_CONST = 0b01000000,
    SF_STR_LIT   = 0b00100000,
} sf_str_flag;

/// A simple string wrapper with length.
typedef struct {
    char *c_str;
    size_t len;
    uint8_t flags;
} sf_str;
#define SF_STR_EMPTY (sf_str) { nullptr, 0, SF_STR_EMPTY }

#define sf_lit(literal) ((sf_str) { .c_str = (char *)literal, .len = sizeof(literal) - 1, .flags = SF_STR_CONST | SF_STR_LIT })
#define sf_ref(cstr) ((sf_str) { .c_str = (char *)cstr, .len = strlen(cstr), .flags = SF_STR_CONST })
#define sf_own(cstr) ((sf_str) { .c_str = (char *)cstr, .len = strlen(cstr), .flags = SF_STR_NONE })

#define sf_islit(string) (string.flags & SF_STR_LIT)
#define sf_isempty(string) (string.flags & SF_STR_EMPTY)
/// Create a new string with format specifiers.
[[nodiscard(SF_STR_LEAKED)]]
EXPORT sf_str sf_str_fmt(const char *format, ...);
/// Allocate space for and join two strings together.
[[nodiscard(SF_STR_LEAKED)]]
EXPORT sf_str sf_str_join(const sf_str str1, const sf_str str2);
/// Allocates and duplicates a new string from an existing one.
[[nodiscard(SF_STR_LEAKED)]]
EXPORT sf_str sf_str_dup(const sf_str string);
/// Duplicate a c-string into a sf_str.
[[nodiscard(SF_STR_LEAKED)]]
static inline sf_str sf_str_cdup(const char *string) { return sf_str_dup(sf_ref(string)); }
///

/// Returns 0 if two strings are lexographically equal.
EXPORT int sf_str_cmp(const sf_str str1, const sf_str str2);
/// Returns true if two strings are lexographically equal.
static inline bool sf_str_eq(const sf_str str1, const sf_str str2) { return sf_str_cmp(str1, str2) == 0; }

/// Free a string (static inlined for simplicity)
static inline void sf_str_free(sf_str string) {
    if (!(string.flags & SF_STR_CONST || string.flags & SF_STR_LIT)) {
        free(string.c_str);
        string.len = 0;
    }
}

#endif // STRINGS_H
