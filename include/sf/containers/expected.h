#include <stdbool.h>

/***********************************
 * You should #define EXPECTED_O & EXPECTED_E as OK/Err types,
 * #define EXPECTED_NAME as the desired type name for the type.
 * EXPECTED_O is optional.
***********************************/

#ifndef EXPECTED_NAME
#error Undefined typename EXPECTED_NAME
#define EXPECTED_NAME sf_expected
#endif

#ifndef EXPECTED_O
#   ifndef EXPECTED_E
#       error Undefined type EXPECTED_E
#       define EXPECTED_E void *
#   endif
#endif

#define CAT(a, b) a##b
#define EXPAND_CAT(a, b) CAT(a, b)
#define FUNC(name) EXPAND_CAT(EXPECTED_NAME, _##name)

/// An expected Ok or Err value, with user defined types.
typedef struct {
    bool is_ok;
    union {
        #ifdef EXPECTED_O
        EXPECTED_O ok;
        #endif
        #ifdef EXPECTED_E
        EXPECTED_E err;
        #endif
    } value;
} EXPECTED_NAME;

/// Create an Ok variant of the expected.
#ifdef EXPECTED_O
static inline EXPECTED_NAME FUNC(ok)(EXPECTED_O ok) { return (EXPECTED_NAME) { .is_ok = true, .value.ok = ok, }; }
#else
static inline EXPECTED_NAME FUNC(ok)(void) { return (EXPECTED_NAME) { .is_ok = true }; }
#endif
/// Create an Err variant of the expected.
#ifdef EXPECTED_E
static inline EXPECTED_NAME FUNC(err)(EXPECTED_E err) { return (EXPECTED_NAME) { .is_ok = false, .value.err = err, }; }
#else
static inline EXPECTED_NAME FUNC(err)(void) { return (EXPECTED_NAME) { .is_ok = false }; }
#endif

#undef EXPECTED_NAME
#undef EXPECTED_O
#undef EXPECTED_E

#undef CAT
#undef EXPAND_CAT
#undef FUNC
