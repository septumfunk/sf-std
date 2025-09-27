#ifndef RESULT_H
#define RESULT_H

#include "sf/str.h"

#ifndef RESULT_NAME
#define RESULT_NAME sf_result
#endif

#ifndef RESULT_E
#define RESULT_E sf_str
#endif

/// The result of a function. `result.ok` indicates success, and the presence of a string in `result.err` indicates
/// failure if `result.ok` is false, otherwise indicating a warning or message.
typedef struct {
    bool is_ok;
    #ifdef RESULT_O
    RESULT_O ok;
    #endif
    RESULT_E err;
} sf_result;

/// A result indicating success with an empty string.
static inline sf_result sf_ok(void) { return (sf_result) { true, SF_STR_EMPTY }; }
/// A result indicating failure with a reason string.
static inline sf_result sf_err(const sf_str reason) { return (sf_result){false, sf_str_dup(reason)}; }
/// A wrapper around `sf_err` for use with c-strings.
static inline sf_result sf_cerr(const char *reason) { return (sf_result){false, sf_str_cdup(reason)}; }
/// Discard the result of a function.
static inline void sf_discard(sf_result result) { if (!result.is_ok) sf_str_free(result.err); }

#endif // RESULT_H
