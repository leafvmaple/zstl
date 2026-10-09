// sys/cstring.hpp — minimal <cstring> replacement.
//
// Self-contained string primitives plus runtime byte ABI.
#pragma once

#include "sys/cstddef.hpp"
// Compiler-generated copies also use these symbols. The kernel/runtime owns them.
#if !defined(ZSTL_RUNTIME_DECLARATIONS_PROVIDED)
extern "C" void* memcpy(void*, const void*, sys::size_t);
extern "C" void* memmove(void*, const void*, sys::size_t);
extern "C" void* memset(void*, int, sys::size_t);
extern "C" int memcmp(const void*, const void*, sys::size_t);
#endif

namespace sys {
using ::memcpy;
using ::memmove;
using ::memset;
using ::memcmp;

inline size_t strlen(const char* s) noexcept {
    size_t n = 0;
    while (s[n]) ++n;
    return n;
}
inline int strcmp(const char* a, const char* b) noexcept {
    while (*a && *a == *b) { ++a; ++b; }
    return static_cast<unsigned char>(*a) - static_cast<unsigned char>(*b);
}
inline int strncmp(const char* a, const char* b, size_t n) noexcept {
    for (size_t i = 0; i < n; ++i) {
        int difference = static_cast<unsigned char>(a[i]) - static_cast<unsigned char>(b[i]);
        if (difference || !a[i]) return difference;
    }
    return 0;
}
inline char* strcpy(char* destination, const char* source) noexcept {
    char* out = destination;
    while ((*out++ = *source++)) {}
    return destination;
}
inline char* strncpy(char* destination, const char* source, size_t n) noexcept {
    size_t i = 0;
    for (; i < n && source[i]; ++i) destination[i] = source[i];
    for (; i < n; ++i) destination[i] = '\0';
    return destination;
}
inline const char* strchr(const char* s, int ch) noexcept {
    do {
        if (*s == static_cast<char>(ch)) return s;
    } while (*s++);
    return nullptr;
}
inline char* strchr(char* s, int ch) noexcept { return const_cast<char*>(sys::strchr(static_cast<const char*>(s), ch)); }
inline const char* strrchr(const char* s, int ch) noexcept {
    const char* found = nullptr;
    do {
        if (*s == static_cast<char>(ch)) found = s;
    } while (*s++);
    return found;
}
inline char* strrchr(char* s, int ch) noexcept { return const_cast<char*>(sys::strrchr(static_cast<const char*>(s), ch)); }
inline const char* strstr(const char* s, const char* needle) noexcept {
    if (!*needle) return s;
    for (; *s; ++s) {
        size_t i = 0;
        while (needle[i] && s[i] && needle[i] == s[i]) ++i;
        if (!needle[i]) return s;
    }
    return nullptr;
}
inline char* strstr(char* s, const char* needle) noexcept { return const_cast<char*>(sys::strstr(static_cast<const char*>(s), needle)); }
inline const void* memchr(const void* memory, int ch, size_t n) noexcept {
    const auto* bytes = static_cast<const unsigned char*>(memory);
    for (size_t i = 0; i < n; ++i) if (bytes[i] == static_cast<unsigned char>(ch)) return bytes + i;
    return nullptr;
}
inline void* memchr(void* memory, int ch, size_t n) noexcept { return const_cast<void*>(sys::memchr(static_cast<const void*>(memory), ch, n)); }
} // namespace sys
