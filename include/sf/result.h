#ifndef RESULT_H
#define RESULT_H

#include "sf/str.h"

/// The result of a function. `result.ok` indicates success, and the presence of a string in `result.err` indicates
/// failure if `result.ok` is false, otherwise indicating a warning or message.
typedef struct {
    bool ok;
    sf_str err;
} sf_result;
#define SF_RESULT_LEAKED "sf_result leaked. Did you mean to discard it?\nsf_discard(result)"

/// A result indicating success with an empty string.
[[nodiscard(SF_RESULT_LEAKED)]]
static inline sf_result sf_ok() { return (sf_result) { true, SF_STR_EMPTY }; }
/// A result indicating failure with a reason string.
[[nodiscard(SF_RESULT_LEAKED)]]
static inline sf_result sf_err(const sf_str reason) { return (sf_result){false, sf_str_dup(reason)}; }
/// A wrapper around `sf_err` for use with c-strings.
[[nodiscard(SF_RESULT_LEAKED)]]
static inline sf_result sf_cerr(const char *reason) { return (sf_result){false, sf_str_cdup(reason)}; }
/// Discard the result of a function.
static inline void sf_discard(sf_result result) { if (!result.ok) sf_str_free(result.err); }

#endif // RESULT_H
