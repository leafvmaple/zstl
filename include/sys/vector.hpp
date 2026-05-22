// sys/vector.hpp — dynamic array.
#pragma once

#include "sys/cstddef.hpp"
#include "sys/initializer_list.hpp"
#include "sys/new.hpp"
#include "sys/utility.hpp"

namespace sys {

template <class T>
class vector {
public:
    using value_type = T;
    using size_type = size_t;
    using iterator = T*;
    using const_iterator = const T*;

    vector() noexcept : _data(nullptr), _size(0), _cap(0) {}
    explicit vector(size_t n) : vector() { resize(n); }
    vector(size_t n, const T& v) : vector() { assign(n, v); }
    vector(initializer_list<T> il) : vector() { reserve(il.size()); for (const auto& v : il) push_back(v); }

    template <class It>
    vector(It first, It last) : vector() {
        reserve(static_cast<size_t>(last - first));
        for (; first != last; ++first) push_back(*first);
    }

    vector(const vector& o) : vector() {
        reserve(o._size);
        for (size_t i = 0; i < o._size; ++i) ::new (_data + i) T(o._data[i]);
        _size = o._size;
    }
    vector(vector&& o) noexcept : _data(o._data), _size(o._size), _cap(o._cap) {
        o._data = nullptr; o._size = 0; o._cap = 0;
    }
    ~vector() { _destroy_all(); ::operator delete(_data); }

    vector& operator=(const vector& o) {
        if (this != &o) {
            clear();
            reserve(o._size);
            for (size_t i = 0; i < o._size; ++i) ::new (_data + i) T(o._data[i]);
            _size = o._size;
        }
        return *this;
    }
    vector& operator=(vector&& o) noexcept {
        if (this != &o) {
            _destroy_all();
            ::operator delete(_data);
            _data = o._data; _size = o._size; _cap = o._cap;
            o._data = nullptr; o._size = 0; o._cap = 0;
        }
        return *this;
    }

    size_t size() const noexcept { return _size; }
    size_t capacity() const noexcept { return _cap; }
    bool empty() const noexcept { return _size == 0; }

    T* data() noexcept { return _data; }
    const T* data() const noexcept { return _data; }

    T& operator[](size_t i) { return _data[i]; }
    const T& operator[](size_t i) const { return _data[i]; }
    T& at(size_t i) { return _data[i]; }
    const T& at(size_t i) const { return _data[i]; }
    T& front() { return _data[0]; }
    const T& front() const { return _data[0]; }
    T& back() { return _data[_size - 1]; }
    const T& back() const { return _data[_size - 1]; }

    iterator begin() noexcept { return _data; }
    iterator end() noexcept { return _data + _size; }
    const_iterator begin() const noexcept { return _data; }
    const_iterator end() const noexcept { return _data + _size; }
    const_iterator cbegin() const noexcept { return _data; }
    const_iterator cend() const noexcept { return _data + _size; }

    void clear() noexcept { _destroy_all(); _size = 0; }

    void reserve(size_t n) {
        if (n <= _cap) return;
        _grow_to(n);
    }

    void resize(size_t n) {
        if (n < _size) {
            for (size_t i = n; i < _size; ++i) _data[i].~T();
            _size = n;
        } else if (n > _size) {
            reserve(n);
            for (size_t i = _size; i < n; ++i) ::new (_data + i) T();
            _size = n;
        }
    }
    void resize(size_t n, const T& v) {
        if (n < _size) {
            for (size_t i = n; i < _size; ++i) _data[i].~T();
            _size = n;
        } else if (n > _size) {
            reserve(n);
            for (size_t i = _size; i < n; ++i) ::new (_data + i) T(v);
            _size = n;
        }
    }

    void assign(size_t n, const T& v) {
        clear();
        reserve(n);
        for (size_t i = 0; i < n; ++i) ::new (_data + i) T(v);
        _size = n;
    }
    template <class It>
    void assign(It first, It last) {
        clear();
        reserve(static_cast<size_t>(last - first));
        for (; first != last; ++first) push_back(*first);
    }

    void push_back(const T& v) {
        if (_size == _cap) _grow_one();
        ::new (_data + _size) T(v);
        ++_size;
    }
    void push_back(T&& v) {
        if (_size == _cap) _grow_one();
        ::new (_data + _size) T(sys::move(v));
        ++_size;
    }

    template <class... Args>
    T& emplace_back(Args&&... args) {
        if (_size == _cap) _grow_one();
        T* p = ::new (_data + _size) T(sys::forward<Args>(args)...);
        ++_size;
        return *p;
    }

    void pop_back() { --_size; _data[_size].~T(); }

    iterator insert(iterator pos, const T& v) {
        size_t idx = static_cast<size_t>(pos - _data);
        if (_size == _cap) _grow_one();
        // Shift right.
        if (idx < _size) {
            ::new (_data + _size) T(sys::move(_data[_size - 1]));
            for (size_t i = _size - 1; i > idx; --i) _data[i] = sys::move(_data[i - 1]);
            _data[idx] = v;
        } else {
            ::new (_data + _size) T(v);
        }
        ++_size;
        return _data + idx;
    }

    iterator erase(iterator pos) {
        size_t idx = static_cast<size_t>(pos - _data);
        for (size_t i = idx + 1; i < _size; ++i) _data[i - 1] = sys::move(_data[i]);
        --_size;
        _data[_size].~T();
        return _data + idx;
    }
    iterator erase(iterator first, iterator last) {
        size_t a = static_cast<size_t>(first - _data);
        size_t b = static_cast<size_t>(last - _data);
        size_t n = b - a;
        if (n == 0) return first;
        for (size_t i = b; i < _size; ++i) _data[i - n] = sys::move(_data[i]);
        for (size_t i = _size - n; i < _size; ++i) _data[i].~T();
        _size -= n;
        return _data + a;
    }

private:
    T* _data;
    size_t _size;
    size_t _cap;

    void _destroy_all() noexcept {
        for (size_t i = 0; i < _size; ++i) _data[i].~T();
    }

    void _grow_one() { _grow_to(_cap == 0 ? 4 : _cap * 2); }

    void _grow_to(size_t newcap) {
        T* nd = static_cast<T*>(::operator new(newcap * sizeof(T)));
        for (size_t i = 0; i < _size; ++i) {
            ::new (nd + i) T(sys::move(_data[i]));
            _data[i].~T();
        }
        ::operator delete(_data);
        _data = nd;
        _cap = newcap;
    }
};

}  // namespace sys
