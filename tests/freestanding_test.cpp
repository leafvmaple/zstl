#include <sys/cstddef.hpp>
#include <sys/cstdint.hpp>
#include <sys/memory.hpp>
#include <sys/array.hpp>
#include <sys/vector.hpp>

static_assert(sizeof(sys::uint64_t) == 8);
static_assert(sizeof(sys::size_t) == sizeof(void*));
static_assert(sizeof(sys::unique_ptr<int>) == sizeof(int*));

sys::unique_ptr<int> make_owner() { return sys::unique_ptr<int>(new (sys::nothrow) int(42)); }
sys::vector<int> make_vector() { return {1, 2, 3}; }
sys::array<int, 2> make_array() { return {{4, 5}}; }
int sum(sys::initializer_list<int> values) {
    int result = 0;
    for (int value : values) result += value;
    return result;
}
int probe() { return sum({1, 2, 3}); }
