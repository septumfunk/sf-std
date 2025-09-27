#include <stdbool.h>

/***********************************
 * You should #define EXPECTED_O & EXPECTED_E as OK/Err types,
 * #define EXPECTED_NAME as the desired type name for the type.
***********************************/

#ifndef EXPECTED_NAME
#error "Please define a typename with EXPECTED_NAME"
#define EXPECTED_NAME sf_expected
#endif

#ifndef EXPECTED_O
#error "Please define an Ok type with EXPECTED_O"
#define EXPECTED_O void *
#endif

#ifndef EXPECTED_E
#error "Please define an Err type with EXPECTED_E
#define EXPECTED_E void *
#endif

#define CAT(a, b) a##b
#define EXPAND_CAT(a, b) CAT(a, b)
#define FUNC(name) EXPAND_CAT(MAP_NAME, _##name)

/// An expected Ok or Err value, with user defined types.
typedef struct {
    bool is_ok;
    union {
        EXPECTED_O ok;
        EXPECTED_E err;
    } value;
} EXPECTED_NAME;

/// Create an Ok variant of the expected.
static inline EXPECTED_NAME FUNC(ok)(EXPECTED_O ok) { return (EXPECTED_NAME) { .is_ok = true, .value.ok = ok, }; }
/// Create an Err variant of the expected.
static inline EXPECTED_NAME FUNC(err)(EXPECTED_E err) { return (EXPECTED_NAME) { .is_ok = false, .value.err = err, }; }

#undef EXPECTED_NAME
#undef EXPECTED_O
#undef EXPECTED_E
