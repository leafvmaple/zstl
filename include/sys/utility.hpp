// sys/utility.hpp — move/forward/swap/exchange/pair.
#pragma once

#include "sys/type_traits.hpp"

namespace sys {

template <class T>
constexpr typename remove_reference<T>::type&& move(T&& v) noexcept {
    return static_cast<typename remove_reference<T>::type&&>(v);
}

template <class T>
constexpr T&& forward(typename remove_reference<T>::type& v) noexcept {
    return static_cast<T&&>(v);
}
template <class T>
constexpr T&& forward(typename remove_reference<T>::type&& v) noexcept {
    static_assert(!is_lvalue_reference<T>::value, "bad forward of rvalue as lvalue");
    return static_cast<T&&>(v);
}

template <class T>
void swap(T& a, T& b) noexcept {
    T tmp(sys::move(a));
    a = sys::move(b);
    b = sys::move(tmp);
}

template <class T, class U = T>
T exchange(T& obj, U&& new_value) {
    T old = sys::move(obj);
    obj = sys::forward<U>(new_value);
    return old;
}

template <class T1, class T2>
struct pair {
    using first_type = T1;
    using second_type = T2;
    T1 first{};
    T2 second{};

    constexpr pair() = default;
    constexpr pair(const T1& a, const T2& b) : first(a), second(b) {}
    template <class U1, class U2>
    constexpr pair(U1&& a, U2&& b) : first(sys::forward<U1>(a)), second(sys::forward<U2>(b)) {}

    pair(const pair&) = default;
    pair(pair&&) noexcept = default;
    pair& operator=(const pair&) = default;
    pair& operator=(pair&&) noexcept = default;
};

template <class T1, class T2>
constexpr pair<typename decay<T1>::type, typename decay<T2>::type>
make_pair(T1&& a, T2&& b) {
    return pair<typename decay<T1>::type, typename decay<T2>::type>(
        sys::forward<T1>(a), sys::forward<T2>(b));
}

}  // namespace sys
