// sys/functional.hpp — function, hash, less, equal_to.
#pragma once

#include "sys/cstddef.hpp"
#include "sys/cstdint.hpp"
#include "sys/new.hpp"
#include "sys/type_traits.hpp"
#include "sys/utility.hpp"

namespace sys {

template <class T = void>
struct less {
    constexpr bool operator()(const T& a, const T& b) const { return a < b; }
};
template <> struct less<void> {
    template <class A, class B>
    constexpr auto operator()(A&& a, B&& b) const -> decltype(a < b) { return a < b; }
};

template <class T = void>
struct equal_to {
    constexpr bool operator()(const T& a, const T& b) const { return a == b; }
};
template <> struct equal_to<void> {
    template <class A, class B>
    constexpr auto operator()(A&& a, B&& b) const -> decltype(a == b) { return a == b; }
};

// Generic hash. Specialized below for builtin scalar types and (in string.hpp)
// for sys::string. FNV-1a is used for byte ranges.
template <class T>
struct hash;

namespace detail {
inline size_t fnv1a_bytes(const void* data, size_t n) noexcept {
    const auto* p = static_cast<const unsigned char*>(data);
    size_t h = 1469598103934665603ULL;  // FNV offset basis (64-bit)
    for (size_t i = 0; i < n; ++i) {
        h ^= static_cast<size_t>(p[i]);
        h *= 1099511628211ULL;
    }
    return h;
}
template <class Int>
inline size_t hash_integer(Int v) noexcept {
    // Mix using a multiplicative constant; simple but fine for hash tables.
    auto u = static_cast<uint64_t>(static_cast<typename sys::conditional<sizeof(Int) <= sizeof(uint64_t), uint64_t, uint64_t>::type>(v));
    u ^= u >> 33;
    u *= 0xff51afd7ed558ccdULL;
    u ^= u >> 33;
    u *= 0xc4ceb9fe1a85ec53ULL;
    u ^= u >> 33;
    return static_cast<size_t>(u);
}
}  // namespace detail

#define ZSTL_HASH_INT(T) \
    template <> struct hash<T> { size_t operator()(T v) const noexcept { return detail::hash_integer(static_cast<uint64_t>(v)); } }

ZSTL_HASH_INT(bool);
ZSTL_HASH_INT(char);
ZSTL_HASH_INT(signed char);
ZSTL_HASH_INT(unsigned char);
ZSTL_HASH_INT(short);
ZSTL_HASH_INT(unsigned short);
ZSTL_HASH_INT(int);
ZSTL_HASH_INT(unsigned int);
ZSTL_HASH_INT(long);
ZSTL_HASH_INT(unsigned long);
ZSTL_HASH_INT(long long);
ZSTL_HASH_INT(unsigned long long);
ZSTL_HASH_INT(char32_t);
ZSTL_HASH_INT(char16_t);
ZSTL_HASH_INT(wchar_t);
#undef ZSTL_HASH_INT

template <class T>
struct hash<T*> {
    size_t operator()(T* p) const noexcept {
        return detail::hash_integer(reinterpret_cast<uintptr_t>(p));
    }
};

// ---- function<R(Args...)> — small-buffer-optimized type-erasing callable.
template <class>
class function;  // primary undefined

template <class R, class... Args>
class function<R(Args...)> {
public:
    function() noexcept : _vt(nullptr) {}
    function(decltype(nullptr)) noexcept : _vt(nullptr) {}

    template <class F,
              class = enable_if_t<!is_same<decay_t<F>, function>::value>>
    function(F&& f) : _vt(nullptr) { _construct(sys::forward<F>(f)); }

    function(const function& o) : _vt(o._vt) {
        if (_vt) _vt->copy(&_storage, &o._storage);
    }
    function(function&& o) noexcept : _vt(o._vt) {
        if (_vt) _vt->move(&_storage, &o._storage);
        o._vt = nullptr;
    }

    ~function() { reset(); }

    function& operator=(const function& o) {
        if (this != &o) { reset(); _vt = o._vt; if (_vt) _vt->copy(&_storage, &o._storage); }
        return *this;
    }
    function& operator=(function&& o) noexcept {
        if (this != &o) {
            reset();
            _vt = o._vt;
            if (_vt) _vt->move(&_storage, &o._storage);
            o._vt = nullptr;
        }
        return *this;
    }
    function& operator=(decltype(nullptr)) noexcept { reset(); return *this; }

    template <class F,
              class = enable_if_t<!is_same<decay_t<F>, function>::value>>
    function& operator=(F&& f) {
        reset();
        _construct(sys::forward<F>(f));
        return *this;
    }

    explicit operator bool() const noexcept { return _vt != nullptr; }

    R operator()(Args... args) const {
        // Calling an empty function is fail-fast; mirrors the contract callers expect.
        return _vt->invoke(const_cast<void*>(static_cast<const void*>(&_storage)),
                           sys::forward<Args>(args)...);
    }

private:
    static constexpr size_t kBufBytes = sizeof(void*) * 4;

    struct VTable {
        R (*invoke)(void* self, Args&&... args);
        void (*copy)(void* dst, const void* src);
        void (*move)(void* dst, void* src);
        void (*destroy)(void* self);
        bool heap;
    };

    union Storage {
        unsigned char small[kBufBytes];
        void* heap;
        Storage() {}
        ~Storage() {}
    };

    Storage _storage;
    const VTable* _vt;

    template <class F>
    void _construct(F&& f) {
        using Fn = decay_t<F>;
        static const VTable vt = make_vtable<Fn>();
        _vt = &vt;
        if constexpr (sizeof(Fn) <= kBufBytes && alignof(Fn) <= alignof(Storage)) {
            ::new (static_cast<void*>(&_storage.small)) Fn(sys::forward<F>(f));
        } else {
            _storage.heap = ::operator new(sizeof(Fn));
            ::new (_storage.heap) Fn(sys::forward<F>(f));
        }
    }

    template <class Fn>
    static constexpr VTable make_vtable() {
        VTable v{};
        constexpr bool small = sizeof(Fn) <= kBufBytes && alignof(Fn) <= alignof(Storage);
        v.heap = !small;
        v.invoke = [](void* self, Args&&... args) -> R {
            Storage* s = static_cast<Storage*>(self);
            Fn* p = small ? reinterpret_cast<Fn*>(&s->small) : static_cast<Fn*>(s->heap);
            return (*p)(sys::forward<Args>(args)...);
        };
        v.copy = [](void* dst, const void* src) {
            Storage* d = static_cast<Storage*>(dst);
            const Storage* s = static_cast<const Storage*>(src);
            const Fn* sp = small ? reinterpret_cast<const Fn*>(&s->small)
                                 : static_cast<const Fn*>(s->heap);
            if constexpr (small) {
                ::new (static_cast<void*>(&d->small)) Fn(*sp);
            } else {
                d->heap = ::operator new(sizeof(Fn));
                ::new (d->heap) Fn(*sp);
            }
        };
        v.move = [](void* dst, void* src) {
            Storage* d = static_cast<Storage*>(dst);
            Storage* s = static_cast<Storage*>(src);
            if constexpr (small) {
                Fn* sp = reinterpret_cast<Fn*>(&s->small);
                ::new (static_cast<void*>(&d->small)) Fn(sys::move(*sp));
                sp->~Fn();
            } else {
                d->heap = s->heap;
                s->heap = nullptr;
            }
        };
        v.destroy = [](void* self) {
            Storage* s = static_cast<Storage*>(self);
            if constexpr (small) {
                reinterpret_cast<Fn*>(&s->small)->~Fn();
            } else if (s->heap) {
                static_cast<Fn*>(s->heap)->~Fn();
                ::operator delete(s->heap);
                s->heap = nullptr;
            }
        };
        return v;
    }

    void reset() noexcept {
        if (_vt) {
            _vt->destroy(&_storage);
            _vt = nullptr;
        }
    }
};

template <class R, class... A>
bool operator==(const function<R(A...)>& f, decltype(nullptr)) noexcept { return !f; }
template <class R, class... A>
bool operator!=(const function<R(A...)>& f, decltype(nullptr)) noexcept { return static_cast<bool>(f); }
template <class R, class... A>
bool operator==(decltype(nullptr), const function<R(A...)>& f) noexcept { return !f; }
template <class R, class... A>
bool operator!=(decltype(nullptr), const function<R(A...)>& f) noexcept { return static_cast<bool>(f); }

}  // namespace sys
