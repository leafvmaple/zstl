// C++20 backport of the capacity-fallible C++26 inplace_vector core (P0843R14).
// Embedded storage: insertion never allocates, and only [begin(), end()) is live.
#pragma once

#include "sys/cstddef.hpp"
#include "sys/new.hpp"
#include "sys/type_traits.hpp"
#include "sys/utility.hpp"

namespace sys {
namespace detail {
template<class T, size_t N>
union inplace_storage {
    char empty;
    T elements[N];
    constexpr inplace_storage() noexcept : empty{} {}
    inplace_storage(const inplace_storage&) = default;
    inplace_storage(inplace_storage&&) = default;
    inplace_storage& operator=(const inplace_storage&) = default;
    inplace_storage& operator=(inplace_storage&&) = default;
    ~inplace_storage() requires (N == 0 || is_trivially_destructible_v<T>) = default;
    ~inplace_storage() requires (!is_trivially_destructible_v<T>) {}
    T* data() noexcept { return elements; }
    const T* data() const noexcept { return elements; }
};
template<class T>
union inplace_storage<T, 0> {
    char empty;
    constexpr inplace_storage() noexcept : empty{} {}
    T* data() noexcept { return nullptr; }
    const T* data() const noexcept { return nullptr; }
};
} // namespace detail

template<class T, size_t N>
class inplace_vector {
    static constexpr bool trivial_copy = N == 0 || (__is_trivially_constructible(T, const T&) && is_trivially_destructible_v<T>);
    static constexpr bool trivial_move = N == 0 || (__is_trivially_constructible(T, T&&) && is_trivially_destructible_v<T>);
    static constexpr bool trivial_copy_assignment = trivial_copy && (N == 0 || __is_trivially_assignable(T&, const T&));
    static constexpr bool trivial_move_assignment = trivial_move && (N == 0 || __is_trivially_assignable(T&, T&&));
public:
    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = T*;
    using const_iterator = const T*;

    constexpr inplace_vector() noexcept = default;
    inplace_vector(const inplace_vector&) requires (trivial_copy) = default;
    inplace_vector(const inplace_vector& other)
        requires (!trivial_copy && is_constructible<T, const T&>::value) {
        construction_guard guard{*this};
        for (const auto& value : other) (void)try_emplace_back(value);
        guard.complete = true;
    }
    inplace_vector(inplace_vector&&) requires (trivial_move) = default;
    inplace_vector(inplace_vector&& other) noexcept(__is_nothrow_constructible(T, T&&))
        requires (!trivial_move && is_constructible<T, T&&>::value) {
        construction_guard guard{*this};
        for (auto& value : other) (void)try_emplace_back(sys::move(value));
        guard.complete = true;
    }
    inplace_vector& operator=(const inplace_vector&) requires (trivial_copy_assignment) = default;
    inplace_vector& operator=(const inplace_vector& other)
        requires (!trivial_copy_assignment && is_constructible<T, const T&>::value && is_assignable<T&, const T&>::value) {
        if (this != &other) assign(other);
        return *this;
    }
    inplace_vector& operator=(inplace_vector&&) requires (trivial_move_assignment) = default;
    inplace_vector& operator=(inplace_vector&& other) noexcept(__is_nothrow_constructible(T, T&&) && __is_nothrow_assignable(T&, T&&))
        requires (!trivial_move_assignment && is_constructible<T, T&&>::value && is_assignable<T&, T&&>::value) {
        if (this != &other) {
            size_t i = 0;
            for (; i < size_ && i < other.size_; ++i) data()[i] = sys::move(other[i]);
            while (size_ > other.size_) pop_back();
            for (; i < other.size_; ++i) (void)try_emplace_back(sys::move(other[i]));
        }
        return *this;
    }
    ~inplace_vector() requires (N == 0 || is_trivially_destructible_v<T>) = default;
    ~inplace_vector() requires (N != 0 && !is_trivially_destructible_v<T>) { clear(); }

    template<class... Args>
    [[nodiscard]] T* try_emplace_back(Args&&... args) requires (is_constructible<T, Args...>::value) {
        if (size_ == N) return nullptr;
        T* result = ::new (static_cast<void*>(data() + size_)) T(sys::forward<Args>(args)...);
        ++size_;
        return result;
    }
    [[nodiscard]] T* try_push_back(const T& value) requires (is_constructible<T, const T&>::value) {
        return try_emplace_back(value);
    }
    [[nodiscard]] T* try_push_back(T&& value) requires (is_constructible<T, T&&>::value) {
        return try_emplace_back(sys::move(value));
    }
    // Precondition: not empty. Capacity failure belongs to try_* insertion.
    void pop_back() noexcept {
        if (empty()) __builtin_trap();
        --size_;
        if constexpr (!is_trivially_destructible_v<T>) data()[size_].~T();
    }
    void clear() noexcept { while (!empty()) pop_back(); }
    [[nodiscard]] constexpr size_t size() const noexcept { return size_; }
    [[nodiscard]] static constexpr size_t capacity() noexcept { return N; }
    [[nodiscard]] static constexpr size_t max_size() noexcept { return N; }
    [[nodiscard]] constexpr bool empty() const noexcept { return size_ == 0; }
    T* data() noexcept { return storage_.data(); }
    const T* data() const noexcept { return storage_.data(); }
    T& operator[](size_t i) noexcept { return data()[i]; }
    const T& operator[](size_t i) const noexcept { return data()[i]; }
    T& front() noexcept { return (*this)[0]; }
    const T& front() const noexcept { return (*this)[0]; }
    T& back() noexcept { return (*this)[size_ - 1]; }
    const T& back() const noexcept { return (*this)[size_ - 1]; }
    iterator begin() noexcept { return data(); }
    const_iterator begin() const noexcept { return data(); }
    const_iterator cbegin() const noexcept { return data(); }
    iterator end() noexcept { if constexpr (N == 0) return data(); else return data() + size_; }
    const_iterator end() const noexcept { if constexpr (N == 0) return data(); else return data() + size_; }
    const_iterator cend() const noexcept { return end(); }
private:
    struct construction_guard {
        inplace_vector& owner;
        bool complete = false;
        ~construction_guard() { if (!complete) owner.clear(); }
    };
    void assign(const inplace_vector& other) {
        size_t i = 0;
        for (; i < size_ && i < other.size_; ++i) data()[i] = other[i];
        while (size_ > other.size_) pop_back();
        for (; i < other.size_; ++i) (void)try_emplace_back(other[i]);
    }
    detail::inplace_storage<T, N> storage_{};
    size_t size_{};
};
} // namespace sys
