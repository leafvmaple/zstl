// sys/algorithm.hpp — minimal algorithm set used by mini-cocos.
#pragma once

#include "sys/utility.hpp"

namespace sys {

template <class T>
constexpr const T& min(const T& a, const T& b) { return (b < a) ? b : a; }

template <class T>
constexpr const T& max(const T& a, const T& b) { return (a < b) ? b : a; }

template <class T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
    return v < lo ? lo : (hi < v ? hi : v);
}

template <class It, class T>
It find(It first, It last, const T& value) {
    for (; first != last; ++first) if (*first == value) return first;
    return last;
}

template <class It, class Pred>
It find_if(It first, It last, Pred p) {
    for (; first != last; ++first) if (p(*first)) return first;
    return last;
}

template <class It, class Pred>
It remove_if(It first, It last, Pred p) {
    It out = first;
    for (; first != last; ++first) {
        if (!p(*first)) {
            if (out != first) *out = sys::move(*first);
            ++out;
        }
    }
    return out;
}

template <class It, class T>
It remove(It first, It last, const T& value) {
    return sys::remove_if(first, last, [&](const auto& x) { return x == value; });
}

template <class It, class Fn>
Fn for_each(It first, It last, Fn fn) {
    for (; first != last; ++first) fn(*first);
    return fn;
}

template <class It, class Pred>
auto count_if(It first, It last, Pred p) -> decltype(last - first) {
    decltype(last - first) c = 0;
    for (; first != last; ++first) if (p(*first)) ++c;
    return c;
}

template <class It, class T>
void fill(It first, It last, const T& value) {
    for (; first != last; ++first) *first = value;
}

template <class InIt, class OutIt>
OutIt copy(InIt first, InIt last, OutIt out) {
    for (; first != last; ++first, ++out) *out = *first;
    return out;
}

// Insertion sort — O(n^2) but simple and stable. Good enough for the small
// containers mini-cocos sorts (render commands, listeners, children).
template <class It, class Cmp>
void stable_sort(It first, It last, Cmp cmp) {
    for (It i = first; i != last; ++i) {
        for (It j = i; j != first;) {
            It prev = j;
            --prev;
            if (cmp(*j, *prev)) {
                using sys::swap;
                swap(*j, *prev);
                j = prev;
            } else {
                break;
            }
        }
    }
}

template <class It>
void stable_sort(It first, It last) {
    sys::stable_sort(first, last, [](const auto& a, const auto& b) { return a < b; });
}

template <class It, class Cmp>
void sort(It first, It last, Cmp cmp) { sys::stable_sort(first, last, cmp); }

template <class It>
void sort(It first, It last) { sys::stable_sort(first, last); }

}  // namespace sys
