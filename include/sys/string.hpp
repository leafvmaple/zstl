// sys/string.hpp — minimal basic_string<char> / basic_string<wchar_t>.
#pragma once

#include "sys/cstddef.hpp"
#include "sys/functional.hpp"
#include "sys/new.hpp"
#include "sys/utility.hpp"

namespace sys {

namespace detail {
inline size_t cstr_len_char(const char* s) noexcept {
    size_t n = 0; while (s && s[n]) ++n; return n;
}
inline size_t cstr_len_wchar(const wchar_t* s) noexcept {
    size_t n = 0; while (s && s[n]) ++n; return n;
}
}  // namespace detail

template <class CharT>
class basic_string {
public:
    using value_type = CharT;
    using size_type = size_t;
    using iterator = CharT*;
    using const_iterator = const CharT*;

    static constexpr size_t npos = static_cast<size_t>(-1);

    basic_string() noexcept { _set_empty(); }
    basic_string(const CharT* s) { _init_from(s, _cstr_len(s)); }
    basic_string(const CharT* s, size_t n) { _init_from(s, n); }
    basic_string(size_t n, CharT c) { _init_filled(n, c); }
    basic_string(const basic_string& o) { _init_from(o.data(), o.size()); }
    basic_string(basic_string&& o) noexcept { _move_from(o); }

    ~basic_string() { _free(); }

    basic_string& operator=(const basic_string& o) {
        if (this != &o) { _free(); _init_from(o.data(), o.size()); }
        return *this;
    }
    basic_string& operator=(basic_string&& o) noexcept {
        if (this != &o) { _free(); _move_from(o); }
        return *this;
    }
    basic_string& operator=(const CharT* s) {
        _free(); _init_from(s, _cstr_len(s)); return *this;
    }

    size_t size() const noexcept { return _size; }
    size_t length() const noexcept { return _size; }
    size_t capacity() const noexcept { return _cap; }
    bool empty() const noexcept { return _size == 0; }

    const CharT* c_str() const noexcept { return _data; }
    const CharT* data() const noexcept { return _data; }
    CharT* data() noexcept { return _data; }

    CharT& operator[](size_t i) { return _data[i]; }
    const CharT& operator[](size_t i) const { return _data[i]; }
    CharT& at(size_t i) { return _data[i]; }
    const CharT& at(size_t i) const { return _data[i]; }
    CharT& front() { return _data[0]; }
    const CharT& front() const { return _data[0]; }
    CharT& back() { return _data[_size - 1]; }
    const CharT& back() const { return _data[_size - 1]; }

    iterator begin() noexcept { return _data; }
    iterator end() noexcept { return _data + _size; }
    const_iterator begin() const noexcept { return _data; }
    const_iterator end() const noexcept { return _data + _size; }
    const_iterator cbegin() const noexcept { return _data; }
    const_iterator cend() const noexcept { return _data + _size; }

    void clear() noexcept { _size = 0; if (_data) _data[0] = CharT(0); }

    void reserve(size_t n) { if (n > _cap) _grow_to(n); }

    void resize(size_t n) { resize(n, CharT(0)); }
    void resize(size_t n, CharT c) {
        if (n < _size) {
            _size = n; _data[_size] = CharT(0);
        } else if (n > _size) {
            reserve(n);
            for (size_t i = _size; i < n; ++i) _data[i] = c;
            _size = n; _data[_size] = CharT(0);
        }
    }

    void push_back(CharT c) {
        if (_size + 1 >= _cap) _grow_to(_cap == 0 ? 8 : _cap * 2);
        _data[_size++] = c; _data[_size] = CharT(0);
    }

    basic_string& append(const CharT* s) { return append(s, _cstr_len(s)); }
    basic_string& append(const CharT* s, size_t n) {
        reserve(_size + n + 1);
        for (size_t i = 0; i < n; ++i) _data[_size + i] = s[i];
        _size += n;
        _data[_size] = CharT(0);
        return *this;
    }
    basic_string& append(const basic_string& o) { return append(o.data(), o.size()); }
    basic_string& operator+=(const basic_string& o) { return append(o); }
    basic_string& operator+=(const CharT* s) { return append(s); }
    basic_string& operator+=(CharT c) { push_back(c); return *this; }

    int compare(const basic_string& o) const noexcept {
        size_t n = _size < o._size ? _size : o._size;
        for (size_t i = 0; i < n; ++i) {
            if (_data[i] < o._data[i]) return -1;
            if (_data[i] > o._data[i]) return 1;
        }
        if (_size < o._size) return -1;
        if (_size > o._size) return 1;
        return 0;
    }

    bool operator==(const basic_string& o) const noexcept {
        if (_size != o._size) return false;
        for (size_t i = 0; i < _size; ++i) if (_data[i] != o._data[i]) return false;
        return true;
    }
    bool operator!=(const basic_string& o) const noexcept { return !(*this == o); }
    bool operator<(const basic_string& o) const noexcept { return compare(o) < 0; }

    bool operator==(const CharT* s) const noexcept {
        size_t n = _cstr_len(s);
        if (_size != n) return false;
        for (size_t i = 0; i < n; ++i) if (_data[i] != s[i]) return false;
        return true;
    }
    bool operator!=(const CharT* s) const noexcept { return !(*this == s); }

    size_t find(CharT c, size_t from = 0) const noexcept {
        for (size_t i = from; i < _size; ++i) if (_data[i] == c) return i;
        return npos;
    }

    basic_string substr(size_t pos, size_t n = npos) const {
        if (pos > _size) pos = _size;
        size_t avail = _size - pos;
        if (n > avail) n = avail;
        return basic_string(_data + pos, n);
    }

private:
    CharT* _data{nullptr};
    size_t _size{0};
    size_t _cap{0};  // includes room for null terminator

    static size_t _cstr_len(const CharT* s) noexcept {
        if constexpr (sizeof(CharT) == 1) return detail::cstr_len_char(reinterpret_cast<const char*>(s));
        else return detail::cstr_len_wchar(reinterpret_cast<const wchar_t*>(s));
    }

    void _set_empty() {
        _cap = 1;
        _data = static_cast<CharT*>(::operator new(sizeof(CharT)));
        _data[0] = CharT(0);
        _size = 0;
    }

    void _init_from(const CharT* s, size_t n) {
        _cap = n + 1;
        _data = static_cast<CharT*>(::operator new(_cap * sizeof(CharT)));
        for (size_t i = 0; i < n; ++i) _data[i] = s[i];
        _data[n] = CharT(0);
        _size = n;
    }

    void _init_filled(size_t n, CharT c) {
        _cap = n + 1;
        _data = static_cast<CharT*>(::operator new(_cap * sizeof(CharT)));
        for (size_t i = 0; i < n; ++i) _data[i] = c;
        _data[n] = CharT(0);
        _size = n;
    }

    void _move_from(basic_string& o) noexcept {
        _data = o._data; _size = o._size; _cap = o._cap;
        o._data = nullptr; o._size = 0; o._cap = 0;
        if (!_data) _set_empty();
    }

    void _grow_to(size_t newcap) {
        if (newcap < 8) newcap = 8;
        CharT* nd = static_cast<CharT*>(::operator new(newcap * sizeof(CharT)));
        for (size_t i = 0; i < _size; ++i) nd[i] = _data[i];
        nd[_size] = CharT(0);
        ::operator delete(_data);
        _data = nd; _cap = newcap;
    }

    void _free() noexcept {
        if (_data) ::operator delete(_data);
        _data = nullptr; _size = 0; _cap = 0;
    }
};

template <class CharT>
inline basic_string<CharT> operator+(const basic_string<CharT>& a, const basic_string<CharT>& b) {
    basic_string<CharT> r;
    r.reserve(a.size() + b.size());
    r.append(a);
    r.append(b);
    return r;
}
template <class CharT>
inline basic_string<CharT> operator+(const basic_string<CharT>& a, const CharT* b) {
    basic_string<CharT> r;
    size_t bn = 0; while (b && b[bn]) ++bn;
    r.reserve(a.size() + bn);
    r.append(a);
    r.append(b, bn);
    return r;
}
template <class CharT>
inline basic_string<CharT> operator+(const CharT* a, const basic_string<CharT>& b) {
    basic_string<CharT> r;
    size_t an = 0; while (a && a[an]) ++an;
    r.reserve(an + b.size());
    r.append(a, an);
    r.append(b);
    return r;
}

using string = basic_string<char>;
using wstring = basic_string<wchar_t>;

// hash<string>
template <>
struct hash<string> {
    size_t operator()(const string& s) const noexcept {
        return detail::fnv1a_bytes(s.data(), s.size());
    }
};

// Minimal to_string overloads. Buffer is sized for the largest 64-bit integer.
inline string to_string(long long v) {
    char buf[24];
    bool neg = v < 0;
    unsigned long long u = neg ? static_cast<unsigned long long>(-(v + 1)) + 1ULL
                               : static_cast<unsigned long long>(v);
    int i = sizeof(buf);
    if (u == 0) { buf[--i] = '0'; }
    while (u) { buf[--i] = static_cast<char>('0' + (u % 10)); u /= 10; }
    if (neg) buf[--i] = '-';
    return string(buf + i, sizeof(buf) - i);
}
inline string to_string(unsigned long long v) {
    char buf[24];
    int i = sizeof(buf);
    if (v == 0) { buf[--i] = '0'; }
    while (v) { buf[--i] = static_cast<char>('0' + (v % 10)); v /= 10; }
    return string(buf + i, sizeof(buf) - i);
}
inline string to_string(int v) { return to_string(static_cast<long long>(v)); }
inline string to_string(long v) { return to_string(static_cast<long long>(v)); }
inline string to_string(unsigned int v) { return to_string(static_cast<unsigned long long>(v)); }
inline string to_string(unsigned long v) { return to_string(static_cast<unsigned long long>(v)); }

}  // namespace sys
