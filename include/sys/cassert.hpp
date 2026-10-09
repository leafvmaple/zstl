// Assertions do not require a hosted diagnostic/output runtime.
// Deliberately no include guard: NDEBUG may change between inclusions.
#undef assert
#ifndef ZSTL_ASSERT_FAILURE
#if defined(_MSC_VER) && !defined(__clang__)
#include "sys/cstdlib.hpp"
#define ZSTL_ASSERT_FAILURE(expression, file, line) ::sys::abort()
#else
#define ZSTL_ASSERT_FAILURE(expression, file, line) __builtin_trap()
#endif
#endif
#if defined(NDEBUG)
#define assert(expression) static_cast<void>(0)
#else
#define assert(expression) ((expression) ? static_cast<void>(0) : \
    ZSTL_ASSERT_FAILURE(#expression, __FILE__, __LINE__))
#endif
