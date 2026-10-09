// sys/initializer_list.hpp — compiler-magic header.
//
// std::initializer_list is special: the compiler synthesizes it from
// brace-enclosed initializers and looks it up by name in namespace std.
// Supply the compiler ABI type without any system library headers.
#pragma once

#include "sys/cstddef.hpp"
// The compiler constructs this standard ABI type even when all other library
// facilities live in sys. Match the compiler's representation here.
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
    constexpr const T* begin() const noexcept { return _data; }
#if defined(_MSC_VER)
    // The Microsoft ABI passes begin/end pointers, not pointer/element count.
    constexpr initializer_list() noexcept : _data(nullptr), _end(nullptr) {}
    constexpr initializer_list(const T* first, const T* last) noexcept : _data(first), _end(last) {}
    constexpr size_type size() const noexcept { return _data == _end ? 0 : _end - _data; }
    constexpr const T* end() const noexcept { return _end; }
private:
    const T* _data;
    const T* _end;
#else
    constexpr initializer_list() noexcept : _data(nullptr), _size(0) {}
    constexpr size_type size() const noexcept { return _size; }
    constexpr const T* end() const noexcept { return _size ? _data + _size : _data; }
private:
    constexpr initializer_list(const T* data, size_type size) noexcept : _data(data), _size(size) {}
    const T* _data;
    size_type _size;
#endif
};
template <class T> constexpr const T* begin(initializer_list<T> list) noexcept { return list.begin(); }
template <class T> constexpr const T* end(initializer_list<T> list) noexcept { return list.end(); }
}  // namespace std

namespace sys {

template <class T>
using initializer_list = ::std::initializer_list<T>;

}  // namespace sys
