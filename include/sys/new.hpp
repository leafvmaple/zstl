// sys/new.hpp — placement new + nothrow.
//
// Provides the non-allocating placement forms of operator new,
// plus sys::nothrow_t / sys::nothrow so callers can write
//   new (sys::nothrow) T(...).
//
// Freestanding mode declares allocating operations for the embedding runtime.
// Hosted mode uses the system <new> and forwards sys::nothrow allocation to it.
#pragma once

#include "sys/cstddef.hpp"

// Placement new — required by the language; provided by the compiler/runtime
// in hosted mode. When building freestanding without <new>, supplying these
// inline definitions is safe because the language reserves these signatures.
#if !defined(ZSTL_FREESTANDING)
#include <new>
#else
inline void* operator new(sys::size_t, void* p) noexcept { return p; }
inline void* operator new[](sys::size_t, void* p) noexcept { return p; }
inline void operator delete(void*, void*) noexcept {}
inline void operator delete[](void*, void*) noexcept {}
#endif

namespace sys {

struct nothrow_t {
    explicit nothrow_t() = default;
};
inline constexpr nothrow_t nothrow{};

}  // namespace sys

#if defined(ZSTL_FREESTANDING)
// The embedding runtime defines allocating operations; no system headers needed.
void* operator new(sys::size_t);
void* operator new[](sys::size_t);
void* operator new(sys::size_t, const sys::nothrow_t&) noexcept;
void* operator new[](sys::size_t, const sys::nothrow_t&) noexcept;
void operator delete(void*) noexcept;
void operator delete[](void*) noexcept;
void operator delete(void*, sys::size_t) noexcept;
void operator delete[](void*, sys::size_t) noexcept;
void operator delete(void*, const sys::nothrow_t&) noexcept;
void operator delete[](void*, const sys::nothrow_t&) noexcept;
#else
inline void* operator new(sys::size_t n, const sys::nothrow_t&) noexcept {
    return ::operator new(n, std::nothrow);
}
inline void* operator new[](sys::size_t n, const sys::nothrow_t&) noexcept {
    return ::operator new[](n, std::nothrow);
}
inline void operator delete(void* p, const sys::nothrow_t&) noexcept {
    ::operator delete(p, std::nothrow);
}
inline void operator delete[](void* p, const sys::nothrow_t&) noexcept {
    ::operator delete[](p, std::nothrow);
}
#endif
