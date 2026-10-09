// sys/iterator.hpp — simple iterator helpers.
#pragma once

#include "sys/cstddef.hpp"

namespace sys {

template <class C>
constexpr auto begin(C& c) -> decltype(c.begin()) { return c.begin(); }
template <class C>
constexpr auto begin(const C& c) -> decltype(c.begin()) { return c.begin(); }
template <class C>
constexpr auto end(C& c) -> decltype(c.end()) { return c.end(); }
template <class C>
constexpr auto end(const C& c) -> decltype(c.end()) { return c.end(); }

template <class T, size_t N>
constexpr T* begin(T (&a)[N]) noexcept { return a; }
template <class T, size_t N>
constexpr T* end(T (&a)[N]) noexcept { return a + N; }

template <class C>
constexpr auto size(const C& c) noexcept(noexcept(c.size())) -> decltype(c.size()) { return c.size(); }
template <class T, size_t N>
constexpr size_t size(const T (&)[N]) noexcept { return N; }

// Random-access distance only; sufficient for vector/string/array.
template <class It>
constexpr auto distance(It first, It last) -> decltype(last - first) {
    return last - first;
}

}  // namespace sys
