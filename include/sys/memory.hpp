// sys/memory.hpp — unique_ptr + make_unique.
#pragma once

#include "sys/cstddef.hpp"
#include "sys/type_traits.hpp"
#include "sys/utility.hpp"

namespace sys {

template <class T>
struct default_delete {
    constexpr default_delete() noexcept = default;
    void operator()(T* p) const noexcept { delete p; }
};

template <class T>
struct default_delete<T[]> {
    constexpr default_delete() noexcept = default;
    void operator()(T* p) const noexcept { delete[] p; }
};

template <class T, class D = default_delete<T>>
class unique_ptr {
public:
    using pointer = T*;
    using element_type = T;
    using deleter_type = D;

    constexpr unique_ptr() noexcept : _ptr(nullptr) {}
    constexpr unique_ptr(decltype(nullptr)) noexcept : _ptr(nullptr) {}
    explicit unique_ptr(pointer p) noexcept : _ptr(p) {}
    unique_ptr(unique_ptr&& o) noexcept : _ptr(o._ptr), _del(sys::move(o._del)) { o._ptr = nullptr; }

    template <class U, class E>
    unique_ptr(unique_ptr<U, E>&& o) noexcept : _ptr(o.release()), _del() {}

    ~unique_ptr() { if (_ptr) _del(_ptr); }

    unique_ptr(const unique_ptr&) = delete;
    unique_ptr& operator=(const unique_ptr&) = delete;

    unique_ptr& operator=(unique_ptr&& o) noexcept {
        if (this != &o) {
            reset(o._ptr);
            o._ptr = nullptr;
            _del = sys::move(o._del);
        }
        return *this;
    }

    unique_ptr& operator=(decltype(nullptr)) noexcept { reset(); return *this; }

    pointer release() noexcept { pointer p = _ptr; _ptr = nullptr; return p; }

    void reset(pointer p = nullptr) noexcept {
        pointer old = _ptr;
        _ptr = p;
        if (old) _del(old);
    }

    pointer get() const noexcept { return _ptr; }
    deleter_type& get_deleter() noexcept { return _del; }
    const deleter_type& get_deleter() const noexcept { return _del; }

    explicit operator bool() const noexcept { return _ptr != nullptr; }
    T& operator*() const { return *_ptr; }
    pointer operator->() const noexcept { return _ptr; }

private:
    pointer _ptr;
    deleter_type _del;
};

template <class T, class D>
bool operator==(const unique_ptr<T, D>& a, decltype(nullptr)) noexcept { return !a; }
template <class T, class D>
bool operator!=(const unique_ptr<T, D>& a, decltype(nullptr)) noexcept { return static_cast<bool>(a); }
template <class T, class D>
bool operator==(decltype(nullptr), const unique_ptr<T, D>& a) noexcept { return !a; }
template <class T, class D>
bool operator!=(decltype(nullptr), const unique_ptr<T, D>& a) noexcept { return static_cast<bool>(a); }

template <class T, class... Args>
unique_ptr<T> make_unique(Args&&... args) {
    return unique_ptr<T>(new T(sys::forward<Args>(args)...));
}

}  // namespace sys
