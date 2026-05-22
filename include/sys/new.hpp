// sys/new.hpp — placement new + nothrow.
//
// Provides the placement form of operator new (a constexpr, non-allocating
// overload), plus sys::nothrow_t / sys::nothrow so callers can write
//   new (sys::nothrow) T(...).
//
// We do NOT redeclare the global allocating operator new here; this header is
// freestanding-friendly. In hosted mode the system <new> already provides it
// (and is harmless to include for placement new), so we include it for safety.
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

// nothrow operator new — accepts sys::nothrow_t. In hosted mode std::nothrow_t
// has the same shape and the host runtime provides the corresponding
// operator new(size_t, std::nothrow_t&). We provide an overload taking
// sys::nothrow_t that forwards.
#if !defined(ZSTL_FREESTANDING)
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
