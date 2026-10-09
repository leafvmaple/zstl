// Allocation/conversion declarations owned by the embedding runtime.
#pragma once

#include "sys/cstddef.hpp"
#if !defined(ZSTL_RUNTIME_DECLARATIONS_PROVIDED)
extern "C" {
void* malloc(sys::size_t);
void* calloc(sys::size_t, sys::size_t);
void* realloc(void*, sys::size_t);
void free(void*);
long strtol(const char*, char**, int);
[[noreturn]] void abort();
}
#endif

namespace sys {
using ::malloc;
using ::calloc;
using ::realloc;
using ::free;
using ::strtol;
using ::abort;
inline int abs(int x) noexcept { return x < 0 ? -x : x; }
} // namespace sys
