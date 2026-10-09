// sys/memory.hpp — unique_ptr + make_unique.
#pragma once

#include "sys/cstddef.hpp"
#include "sys/type_traits.hpp"
#include "sys/utility.hpp"
#include "sys/new.hpp"

namespace sys {

template <class T>
struct default_delete {
    constexpr default_delete() noexcept = default;
    template <class U, class = enable_if_t<is_convertible<U*, T*>::value>>
    constexpr default_delete(const default_delete<U>&) noexcept {}
    void operator()(T* p) const noexcept {
        static_assert(sizeof(T) > 0, "deleting an incomplete type");
        delete p;
    }
};

template <class T>
struct default_delete<T[]> {
    constexpr default_delete() noexcept = default;
    template <class U, class = enable_if_t<is_convertible<U(*)[], T(*)[]>::value>>
    constexpr default_delete(const default_delete<U[]>&) noexcept {}
    template <class U, class = enable_if_t<is_convertible<U(*)[], T(*)[]>::value>>
    void operator()(U* p) const noexcept {
        static_assert(sizeof(U) > 0, "deleting an incomplete element type");
        delete[] p;
    }
};

namespace detail {
// C++17 empty-base optimization: stateless deleters take no extra pointer space.
template <class D, bool = is_empty<D>::value && !is_final<D>::value>
class unique_deleter {
public:
    constexpr unique_deleter() = default;
    explicit unique_deleter(const D& d) : _del(d) {}
    explicit unique_deleter(D&& d) : _del(sys::move(d)) {}
    D& deleter() noexcept { return _del; }
    const D& deleter() const noexcept { return _del; }
private:
    D _del{};
};
template <class D>
class unique_deleter<D, true> : private D {
public:
    constexpr unique_deleter() = default;
    explicit unique_deleter(const D& d) : D(d) {}
    explicit unique_deleter(D&& d) : D(sys::move(d)) {}
    D& deleter() noexcept { return *this; }
    const D& deleter() const noexcept { return *this; }
};
template <class T> struct unbounded_array : false_type {};
template <class T> struct unbounded_array<T[]> : true_type {};
}  // namespace detail

template <class T, class D = default_delete<T>>
class unique_ptr : private detail::unique_deleter<D> {
    using storage = detail::unique_deleter<D>;
public:
    using pointer = T*;
    using element_type = T;
    using deleter_type = D;

    template <class E = D, class = enable_if_t<is_constructible<E>::value>>
    constexpr unique_ptr() noexcept : _ptr(nullptr) {}
    template <class E = D, class = enable_if_t<is_constructible<E>::value>>
    constexpr unique_ptr(decltype(nullptr)) noexcept : _ptr(nullptr) {}
    template <class E = D, class = enable_if_t<is_constructible<E>::value>>
    explicit unique_ptr(pointer p) noexcept : _ptr(p) {}
    unique_ptr(pointer p, const D& d) noexcept : storage(d), _ptr(p) {}
    unique_ptr(pointer p, D&& d) noexcept : storage(sys::move(d)), _ptr(p) {}
    unique_ptr(unique_ptr&& o) noexcept : storage(sys::move(o.get_deleter())), _ptr(o.release()) {}

    template <class U, class E, class = enable_if_t<!is_array<U>::value &&
        is_convertible<U*, T*>::value && is_constructible<D, E&&>::value>>
    unique_ptr(unique_ptr<U, E>&& o) noexcept : storage(sys::move(o.get_deleter())), _ptr(o.release()) {}

    ~unique_ptr() { reset(); }

    unique_ptr(const unique_ptr&) = delete;
    unique_ptr& operator=(const unique_ptr&) = delete;

    unique_ptr& operator=(unique_ptr&& o) noexcept {
        if (this != &o) {
            reset(o.release());
            get_deleter() = sys::move(o.get_deleter());
        }
        return *this;
    }

    template <class U, class E, class = enable_if_t<!is_array<U>::value &&
        is_convertible<U*, T*>::value && is_assignable<D&, E&&>::value>>
    unique_ptr& operator=(unique_ptr<U, E>&& o) noexcept {
        reset(o.release());
        get_deleter() = sys::move(o.get_deleter());
        return *this;
    }

    unique_ptr& operator=(decltype(nullptr)) noexcept { reset(); return *this; }

    [[nodiscard]] pointer release() noexcept { pointer p = _ptr; _ptr = nullptr; return p; }

    void reset(pointer p = nullptr) noexcept {
        pointer old = _ptr;
        _ptr = p;
        if (old) get_deleter()(old);
    }

    pointer get() const noexcept { return _ptr; }
    deleter_type& get_deleter() noexcept { return storage::deleter(); }
    const deleter_type& get_deleter() const noexcept { return storage::deleter(); }

    void swap(unique_ptr& o) noexcept {
        sys::swap(_ptr, o._ptr);
        sys::swap(get_deleter(), o.get_deleter());
    }

    explicit operator bool() const noexcept { return _ptr != nullptr; }
    T& operator*() const { return *_ptr; }
    pointer operator->() const noexcept { return _ptr; }

private:
    pointer _ptr;
};

// Array ownership uses the same storage/cleanup mechanics but no single-object dereference.
template <class T, class D>
class unique_ptr<T[], D> {
    unique_ptr<T, D> _owner;
public:
    using pointer = T*;
    using element_type = T;
    using deleter_type = D;
    unique_ptr() noexcept = default;
    unique_ptr(decltype(nullptr)) noexcept : _owner(nullptr) {}
    template <class U, class = enable_if_t<is_convertible<U(*)[], T(*)[]>::value>>
    explicit unique_ptr(U* p) noexcept : _owner(p) {}
    template <class U, class = enable_if_t<is_convertible<U(*)[], T(*)[]>::value>>
    unique_ptr(U* p, const D& d) noexcept : _owner(p, d) {}
    template <class U, class = enable_if_t<is_convertible<U(*)[], T(*)[]>::value>>
    unique_ptr(U* p, D&& d) noexcept : _owner(p, sys::move(d)) {}
    unique_ptr(unique_ptr&&) noexcept = default;
    unique_ptr& operator=(unique_ptr&&) noexcept = default;
    unique_ptr(const unique_ptr&) = delete;
    unique_ptr& operator=(const unique_ptr&) = delete;
    template <class U, class E, class = enable_if_t<is_convertible<U(*)[], T(*)[]>::value &&
        is_constructible<D, E&&>::value>>
    unique_ptr(unique_ptr<U[], E>&& o) noexcept : _owner(o.release(), sys::move(o.get_deleter())) {}
    template <class U, class E, class = enable_if_t<is_convertible<U(*)[], T(*)[]>::value &&
        is_assignable<D&, E&&>::value>>
    unique_ptr& operator=(unique_ptr<U[], E>&& o) noexcept {
        _owner.reset(o.release());
        get_deleter() = sys::move(o.get_deleter());
        return *this;
    }
    unique_ptr& operator=(decltype(nullptr)) noexcept { reset(); return *this; }
    pointer get() const noexcept { return _owner.get(); }
    [[nodiscard]] pointer release() noexcept { return _owner.release(); }
    void reset(decltype(nullptr) = nullptr) noexcept { _owner.reset(); }
    template <class U, class = enable_if_t<is_convertible<U(*)[], T(*)[]>::value>>
    void reset(U* p) noexcept { _owner.reset(p); }
    D& get_deleter() noexcept { return _owner.get_deleter(); }
    const D& get_deleter() const noexcept { return _owner.get_deleter(); }
    explicit operator bool() const noexcept { return static_cast<bool>(_owner); }
    T& operator[](size_t i) const { return get()[i]; }
    void swap(unique_ptr& o) noexcept { _owner.swap(o._owner); }
};

template <class T, class D>
void swap(unique_ptr<T, D>& a, unique_ptr<T, D>& b) noexcept { a.swap(b); }

template <class T, class D>
bool operator==(const unique_ptr<T, D>& a, decltype(nullptr)) noexcept { return !a; }
template <class T, class D>
bool operator!=(const unique_ptr<T, D>& a, decltype(nullptr)) noexcept { return static_cast<bool>(a); }
template <class T, class D>
bool operator==(decltype(nullptr), const unique_ptr<T, D>& a) noexcept { return !a; }
template <class T, class D>
bool operator!=(decltype(nullptr), const unique_ptr<T, D>& a) noexcept { return static_cast<bool>(a); }

template <class T, class... Args>
[[nodiscard]] enable_if_t<!is_array<T>::value, unique_ptr<T>> make_unique(Args&&... args) {
    return unique_ptr<T>(new T(sys::forward<Args>(args)...));
}

template <class T>
[[nodiscard]] enable_if_t<detail::unbounded_array<T>::value, unique_ptr<T>> make_unique(size_t count) {
    return unique_ptr<T>(new remove_extent_t<T>[count]());
}
template <class T, class... Args>
enable_if_t<is_array<T>::value && !detail::unbounded_array<T>::value, void> make_unique(Args&&...) = delete;

}  // namespace sys
