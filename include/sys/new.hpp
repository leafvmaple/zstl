// sys/new.hpp — placement new + nothrow.
//
// Provides the non-allocating placement forms of operator new,
// plus sys::nothrow_t / sys::nothrow so callers can write
//   new (sys::nothrow) T(...).
//
// Allocating operations are supplied by the embedding runtime.
#pragma once

#include "sys/cstddef.hpp"

// Placement forms are non-allocating and do not need a runtime.
inline void* operator new(sys::size_t, void* p) noexcept { return p; }
inline void* operator new[](sys::size_t, void* p) noexcept { return p; }
inline void operator delete(void*, void*) noexcept {}
inline void operator delete[](void*, void*) noexcept {}

namespace sys {

struct nothrow_t {
    explicit nothrow_t() = default;
};
inline constexpr nothrow_t nothrow{};

}  // namespace sys

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
