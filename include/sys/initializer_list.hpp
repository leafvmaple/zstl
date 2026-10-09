// sys/initializer_list.hpp — compiler-magic header.
//
// std::initializer_list is special: the compiler synthesizes it from
// brace-enclosed initializers and looks it up by name in namespace std.
// Hosted mode uses the system header; freestanding mode supplies the compiler
// ABI type here. Both modes re-export the type under sys.
#pragma once

#if !defined(ZSTL_FREESTANDING)
#include <initializer_list>
#else
#include "sys/cstddef.hpp"
// The compiler constructs this standard ABI type even when all other library
// facilities live in sys. Keep the pointer/length representation shared here.
namespace std {
template <class T>
class initializer_list {
public:
    using value_type = T;
    using reference = const T&;
    using const_reference = const T&;
    using size_type = sys::size_t;
    using iterator = const T*;
    using const_iterator = const T*;
    constexpr initializer_list() noexcept : _data(nullptr), _size(0) {}
    constexpr size_type size() const noexcept { return _size; }
    constexpr const T* begin() const noexcept { return _data; }
    constexpr const T* end() const noexcept { return _size ? _data + _size : _data; }
private:
    constexpr initializer_list(const T* data, size_type size) noexcept : _data(data), _size(size) {}
    const T* _data;
    size_type _size;
};
template <class T> constexpr const T* begin(initializer_list<T> list) noexcept { return list.begin(); }
template <class T> constexpr const T* end(initializer_list<T> list) noexcept { return list.end(); }
}  // namespace std
#endif

namespace sys {

template <class T>
using initializer_list = ::std::initializer_list<T>;

}  // namespace sys
