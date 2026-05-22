// sys/unordered_map.hpp — chaining hash map.
//
// Minimal interface used by mini-cocos: operator[], find, count, erase, emplace,
// insert, size, clear, begin/end iteration.
#pragma once

#include "sys/cstddef.hpp"
#include "sys/functional.hpp"
#include "sys/new.hpp"
#include "sys/utility.hpp"
#include "sys/vector.hpp"

namespace sys {

template <class K, class V, class Hash = hash<K>, class KeyEqual = equal_to<K>>
class unordered_map {
public:
    using key_type = K;
    using mapped_type = V;
    using value_type = pair<const K, V>;

private:
    struct Node {
        value_type kv;
        Node* next;
        template <class KK, class VV>
        Node(KK&& k, VV&& v, Node* n)
            : kv(sys::forward<KK>(k), sys::forward<VV>(v)), next(n) {}
    };

    Node** _buckets{nullptr};
    size_t _bucket_count{0};
    size_t _size{0};
    Hash _hash{};
    KeyEqual _eq{};

    static constexpr size_t kInitialBuckets = 16;
    static constexpr float kMaxLoad = 1.0f;

public:
    class iterator {
    public:
        iterator() = default;
        iterator(Node** buckets, size_t bc, size_t idx, Node* node)
            : _buckets(buckets), _bc(bc), _idx(idx), _node(node) {}

        value_type& operator*() const { return _node->kv; }
        value_type* operator->() const { return &_node->kv; }

        iterator& operator++() { _advance(); return *this; }
        iterator operator++(int) { iterator t = *this; _advance(); return t; }

        bool operator==(const iterator& o) const { return _node == o._node; }
        bool operator!=(const iterator& o) const { return _node != o._node; }

    private:
        Node** _buckets{nullptr};
        size_t _bc{0};
        size_t _idx{0};
        Node* _node{nullptr};

        void _advance() {
            if (!_node) return;
            if (_node->next) { _node = _node->next; return; }
            for (++_idx; _idx < _bc; ++_idx) {
                if (_buckets[_idx]) { _node = _buckets[_idx]; return; }
            }
            _node = nullptr;
        }

        friend class unordered_map;
    };

    using const_iterator = iterator;  // const view not strictly enforced

    unordered_map() = default;

    unordered_map(const unordered_map& o) { _copy_from(o); }
    unordered_map(unordered_map&& o) noexcept { _move_from(o); }

    ~unordered_map() { clear(); ::operator delete(_buckets); }

    unordered_map& operator=(const unordered_map& o) {
        if (this != &o) { _free_all(); _copy_from(o); }
        return *this;
    }
    unordered_map& operator=(unordered_map&& o) noexcept {
        if (this != &o) { _free_all(); _move_from(o); }
        return *this;
    }

    size_t size() const noexcept { return _size; }
    bool empty() const noexcept { return _size == 0; }

    void clear() noexcept {
        if (!_buckets) return;
        for (size_t i = 0; i < _bucket_count; ++i) {
            Node* n = _buckets[i];
            while (n) { Node* nx = n->next; delete n; n = nx; }
            _buckets[i] = nullptr;
        }
        _size = 0;
    }

    iterator begin() {
        if (_size == 0) return end();
        for (size_t i = 0; i < _bucket_count; ++i)
            if (_buckets[i]) return iterator(_buckets, _bucket_count, i, _buckets[i]);
        return end();
    }
    iterator end() { return iterator(_buckets, _bucket_count, _bucket_count, nullptr); }
    const_iterator begin() const { return const_cast<unordered_map*>(this)->begin(); }
    const_iterator end() const { return const_cast<unordered_map*>(this)->end(); }

    iterator find(const K& k) {
        if (_bucket_count == 0) return end();
        size_t i = _bucket_index(k);
        for (Node* n = _buckets[i]; n; n = n->next)
            if (_eq(n->kv.first, k)) return iterator(_buckets, _bucket_count, i, n);
        return end();
    }
    const_iterator find(const K& k) const { return const_cast<unordered_map*>(this)->find(k); }

    size_t count(const K& k) const { return find(k) == end() ? 0 : 1; }

    V& operator[](const K& k) {
        _ensure_buckets();
        size_t i = _bucket_index(k);
        for (Node* n = _buckets[i]; n; n = n->next)
            if (_eq(n->kv.first, k)) return n->kv.second;
        if (_should_rehash()) { _rehash(_bucket_count * 2); i = _bucket_index(k); }
        Node* nn = new Node(k, V{}, _buckets[i]);
        _buckets[i] = nn;
        ++_size;
        return nn->kv.second;
    }

    template <class KK, class VV>
    pair<iterator, bool> emplace(KK&& key, VV&& val) {
        _ensure_buckets();
        size_t i = _bucket_index(key);
        for (Node* n = _buckets[i]; n; n = n->next)
            if (_eq(n->kv.first, key)) return {iterator(_buckets, _bucket_count, i, n), false};
        if (_should_rehash()) { _rehash(_bucket_count * 2); i = _bucket_index(key); }
        Node* nn = new Node(sys::forward<KK>(key), sys::forward<VV>(val), _buckets[i]);
        _buckets[i] = nn;
        ++_size;
        return {iterator(_buckets, _bucket_count, i, nn), true};
    }

    pair<iterator, bool> insert(const value_type& v) { return emplace(v.first, v.second); }
    pair<iterator, bool> insert(value_type&& v) {
        return emplace(sys::move(const_cast<K&>(v.first)), sys::move(v.second));
    }

    size_t erase(const K& k) {
        if (_bucket_count == 0) return 0;
        size_t i = _bucket_index(k);
        Node* prev = nullptr;
        for (Node* n = _buckets[i]; n; prev = n, n = n->next) {
            if (_eq(n->kv.first, k)) {
                if (prev) prev->next = n->next;
                else _buckets[i] = n->next;
                delete n;
                --_size;
                return 1;
            }
        }
        return 0;
    }

    iterator erase(iterator it) {
        if (it == end()) return end();
        const K& k = it->first;
        size_t i = it._idx;
        Node* target = it._node;
        ++it;
        Node* prev = nullptr;
        for (Node* n = _buckets[i]; n; prev = n, n = n->next) {
            if (n == target) {
                if (prev) prev->next = n->next;
                else _buckets[i] = n->next;
                delete n;
                --_size;
                break;
            }
        }
        (void)k;
        return it;
    }

private:
    size_t _bucket_index(const K& k) const { return _hash(k) % _bucket_count; }

    void _ensure_buckets() {
        if (_bucket_count == 0) _rehash(kInitialBuckets);
    }

    bool _should_rehash() const {
        return static_cast<float>(_size + 1) > kMaxLoad * static_cast<float>(_bucket_count);
    }

    void _rehash(size_t newbc) {
        if (newbc < kInitialBuckets) newbc = kInitialBuckets;
        Node** nb = static_cast<Node**>(::operator new(sizeof(Node*) * newbc));
        for (size_t i = 0; i < newbc; ++i) nb[i] = nullptr;

        for (size_t i = 0; i < _bucket_count; ++i) {
            Node* n = _buckets[i];
            while (n) {
                Node* nx = n->next;
                size_t ni = _hash(n->kv.first) % newbc;
                n->next = nb[ni];
                nb[ni] = n;
                n = nx;
            }
        }
        ::operator delete(_buckets);
        _buckets = nb;
        _bucket_count = newbc;
    }

    void _copy_from(const unordered_map& o) {
        _buckets = nullptr; _bucket_count = 0; _size = 0;
        _hash = o._hash; _eq = o._eq;
        if (o._size == 0) return;
        _rehash(o._bucket_count);
        for (size_t i = 0; i < o._bucket_count; ++i)
            for (Node* n = o._buckets[i]; n; n = n->next)
                emplace(n->kv.first, n->kv.second);
    }

    void _move_from(unordered_map& o) noexcept {
        _buckets = o._buckets; _bucket_count = o._bucket_count; _size = o._size;
        _hash = sys::move(o._hash); _eq = sys::move(o._eq);
        o._buckets = nullptr; o._bucket_count = 0; o._size = 0;
    }

    void _free_all() noexcept {
        clear();
        ::operator delete(_buckets);
        _buckets = nullptr; _bucket_count = 0;
    }
};

}  // namespace sys
