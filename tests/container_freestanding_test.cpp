#include <sys/inplace_vector.hpp>
#include <sys/array.hpp>
#include <sys/mutex.hpp>
#include <sys/cstring.hpp>
#include <sys/iterator.hpp>

struct NoDefault {
    int value;
    explicit NoDefault(int v) : value(v) {}
    ~NoDefault() { value = 0; }
};
struct Lock { void lock(); void unlock(); };
sys::inplace_vector<NoDefault, 2> make_inline() {
    sys::inplace_vector<NoDefault, 2> values;
    (void)values.try_emplace_back(1);
    return values;
}
int probe_containers(Lock& lock, char* buffer) {
    sys::lock_guard guard(lock);
    auto values = make_inline();
    auto snapshot = values;
    snapshot.clear();
    sys::array<int, 2> table{};
    table.fill(4);
    sys::strcpy(buffer, "abc");
    return values[0].value + table[0] + sys::size(table) + sys::strlen(buffer);
}
