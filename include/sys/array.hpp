// sys/array.hpp — fixed-size array<T, N>.
#pragma once

#include "sys/cstddef.hpp"

namespace sys {

template <class T, size_t N>
struct array {
    T _data[N];

    using value_type = T;
    using size_type = size_t;
    using iterator = T*;
    using const_iterator = const T*;

    constexpr size_t size() const noexcept { return N; }
    constexpr bool empty() const noexcept { return N == 0; }

    constexpr T& operator[](size_t i) { return _data[i]; }
    constexpr const T& operator[](size_t i) const { return _data[i]; }

    constexpr T& at(size_t i) { return _data[i]; }
    constexpr const T& at(size_t i) const { return _data[i]; }

    constexpr T* data() noexcept { return _data; }
    constexpr const T* data() const noexcept { return _data; }

    constexpr T& front() { return _data[0]; }
    constexpr const T& front() const { return _data[0]; }
    constexpr T& back() { return _data[N - 1]; }
    constexpr const T& back() const { return _data[N - 1]; }

    constexpr iterator begin() noexcept { return _data; }
    constexpr iterator end() noexcept { return _data + N; }
    constexpr const_iterator begin() const noexcept { return _data; }
    constexpr const_iterator end() const noexcept { return _data + N; }
    constexpr const_iterator cbegin() const noexcept { return _data; }
    constexpr const_iterator cend() const noexcept { return _data + N; }

    void fill(const T& v) { for (size_t i = 0; i < N; ++i) _data[i] = v; }
};

}  // namespace sys
