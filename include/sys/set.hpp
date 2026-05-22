// sys/set.hpp — sorted-vector backed unique set.
//
// Mini-cocos uses set sparingly (Vulkan queue-family de-dup). A sorted vector
// keeps the implementation trivial; lookups are O(log n) and inserts are O(n).
#pragma once

#include "sys/algorithm.hpp"
#include "sys/cstddef.hpp"
#include "sys/functional.hpp"
#include "sys/utility.hpp"
#include "sys/vector.hpp"

namespace sys {

template <class T, class Compare = less<T>>
class set {
public:
    using value_type = T;
    using iterator = typename vector<T>::iterator;
    using const_iterator = typename vector<T>::const_iterator;

    set() = default;
    set(const set&) = default;
    set(set&&) noexcept = default;
    set& operator=(const set&) = default;
    set& operator=(set&&) noexcept = default;

    iterator begin() noexcept { return _items.begin(); }
    iterator end() noexcept { return _items.end(); }
    const_iterator begin() const noexcept { return _items.begin(); }
    const_iterator end() const noexcept { return _items.end(); }

    size_t size() const noexcept { return _items.size(); }
    bool empty() const noexcept { return _items.empty(); }
    void clear() noexcept { _items.clear(); }

    pair<iterator, bool> insert(const T& v) {
        size_t i = _lower_bound_index(v);
        if (i < _items.size() && !_cmp(v, _items[i]) && !_cmp(_items[i], v))
            return {_items.begin() + i, false};
        _items.insert(_items.begin() + i, v);
        return {_items.begin() + i, true};
    }

    size_t erase(const T& v) {
        size_t i = _lower_bound_index(v);
        if (i < _items.size() && !_cmp(v, _items[i]) && !_cmp(_items[i], v)) {
            _items.erase(_items.begin() + i);
            return 1;
        }
        return 0;
    }

    iterator find(const T& v) {
        size_t i = _lower_bound_index(v);
        if (i < _items.size() && !_cmp(v, _items[i]) && !_cmp(_items[i], v))
            return _items.begin() + i;
        return _items.end();
    }
    const_iterator find(const T& v) const {
        return const_cast<set*>(this)->find(v);
    }

    size_t count(const T& v) const { return find(v) == end() ? 0 : 1; }

private:
    vector<T> _items;
    Compare _cmp{};

    size_t _lower_bound_index(const T& v) const {
        size_t lo = 0, hi = _items.size();
        while (lo < hi) {
            size_t mid = lo + (hi - lo) / 2;
            if (_cmp(_items[mid], v)) lo = mid + 1;
            else hi = mid;
        }
        return lo;
    }
};

}  // namespace sys
