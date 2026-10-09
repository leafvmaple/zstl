#include <sys/inplace_vector.hpp>
#include <sys/array.hpp>
#include <sys/memory.hpp>
#include <sys/iterator.hpp>
#include <sys/mutex.hpp>
#include <cassert>
#include <type_traits>

struct Tracked {
    inline static int live = 0;
    inline static int constructed = 0;
    int value;
    explicit Tracked(int v) noexcept : value(v) { ++live; ++constructed; }
    Tracked(const Tracked& other) noexcept : Tracked(other.value) {}
    Tracked(Tracked&& other) noexcept : Tracked(other.value) { other.value = -1; }
    Tracked& operator=(const Tracked&) = default;
    Tracked& operator=(Tracked&& other) noexcept { value = other.value; other.value = -1; return *this; }
    ~Tracked() { --live; }
};
struct alignas(128) Aligned { int value; explicit Aligned(int v) : value(v) {} };
struct Immobile {
    explicit Immobile(int) {}
    Immobile(const Immobile&) = delete;
    Immobile(Immobile&&) = delete;
};
struct TrivialMoveOnly {
    int value;
    explicit TrivialMoveOnly(int v) : value(v) {}
    TrivialMoveOnly(const TrivialMoveOnly&) = delete;
    TrivialMoveOnly& operator=(const TrivialMoveOnly&) = delete;
    TrivialMoveOnly(TrivialMoveOnly&&) = default;
    TrivialMoveOnly& operator=(TrivialMoveOnly&&) = default;
};
struct CustomCopy {
    int value;
    explicit CustomCopy(int v) : value(v) {}
    CustomCopy(const CustomCopy& other) : value(other.value + 1) {}
    CustomCopy(CustomCopy&&) = default;
    CustomCopy& operator=(const CustomCopy&) = default;
    CustomCopy& operator=(CustomCopy&&) = default;
};
struct FakeMutex {
    bool held = false;
    int locks = 0, unlocks = 0;
    void lock() { assert(!held); held = true; ++locks; }
    void unlock() { assert(held); held = false; ++unlocks; }
};
struct PrivateDestructor { private: ~PrivateDestructor() = default; };
static_assert(sys::is_trivially_destructible_v<int>);
static_assert(sys::is_trivially_destructible_v<int&>);
static_assert(sys::is_trivially_destructible_v<int[2]>);
static_assert(!sys::is_trivially_destructible_v<void>);
static_assert(!sys::is_trivially_destructible_v<int[]>);
static_assert(!sys::is_trivially_destructible_v<Tracked>);
static_assert(!sys::is_trivially_destructible_v<PrivateDestructor>);
using OwnerVector = sys::inplace_vector<sys::unique_ptr<int>, 2>;
static_assert(!std::is_copy_constructible_v<OwnerVector>);
static_assert(!std::is_copy_assignable_v<OwnerVector>);
static_assert(std::is_move_constructible_v<OwnerVector>);
static_assert(std::is_move_assignable_v<OwnerVector>);
static_assert(std::is_nothrow_move_constructible_v<OwnerVector>);
static_assert(std::is_nothrow_move_assignable_v<OwnerVector>);
static_assert(std::is_move_constructible_v<sys::inplace_vector<TrivialMoveOnly, 2>>);
static_assert(!std::is_copy_constructible_v<sys::inplace_vector<TrivialMoveOnly, 2>>);
static_assert(std::is_copy_constructible_v<sys::inplace_vector<CustomCopy, 2>>);
static_assert(std::is_trivially_copyable_v<sys::inplace_vector<int, 4>>);
static_assert(std::is_trivially_destructible_v<sys::inplace_vector<int, 4>>);
static_assert(!std::is_copy_constructible_v<sys::inplace_vector<Immobile, 1>>);
static_assert(std::is_copy_constructible_v<sys::inplace_vector<Immobile, 0>>);
static_assert(!std::is_copy_constructible_v<sys::lock_guard<FakeMutex>>);
static_assert(sys::is_pointer_v<int* const volatile>);
static_assert(!sys::is_pointer_v<int&>);
constexpr int raw[] = {1, 2, 3};
static_assert(sys::size(raw) == 3);
constexpr sys::array<int, 2> fixed{{4, 5}};
static_assert(sys::size(fixed) == 2);

void lifetime_and_capacity() {
    assert(Tracked::live == 0);
    {
        sys::inplace_vector<Tracked, 2> values;
        assert(values.empty() && Tracked::live == 0);
        Tracked* first = values.try_emplace_back(10);
        assert(first && first->value == 10 && Tracked::live == 1);
        assert(values.try_emplace_back(20));
        int constructed = Tracked::constructed;
        assert(!values.try_emplace_back(30));
        assert(constructed == Tracked::constructed && Tracked::live == 2);
        assert(first == values.data() && first->value == 10);
        sys::inplace_vector<Tracked, 2> copy(values);
        assert(copy.size() == 2 && copy.front().value == 10 && Tracked::live == 4);
        copy[0].value = 11;
        assert(values.front().value == 10);
        copy.pop_back();
        assert(Tracked::live == 3);
        copy = values;
        assert(copy.size() == 2 && Tracked::live == 4);
        values.pop_back();
        copy = values;
        assert(copy.size() == 1 && Tracked::live == 2);
        copy = copy;
        assert(copy.front().value == 10);
        sys::inplace_vector<Tracked, 2> moved(sys::move(values));
        assert(moved.size() == 1 && values.size() == 1 && values.front().value == -1);
        copy = sys::move(moved);
        assert(copy.front().value == 10 && moved.size() == 1);
        copy = sys::move(copy);
        assert(copy.front().value == 10);
        values.clear();
        assert(Tracked::live == 2);
        assert(values.try_emplace_back(40) == first);
        assert(first->value == 40);
    }
    assert(Tracked::live == 0);
}
void move_only_and_zero_capacity() {
    OwnerVector values;
    assert(values.try_emplace_back(new int(1)));
    assert(values.try_emplace_back(new int(2)));
    sys::unique_ptr<int> candidate(new int(3));
    assert(!values.try_push_back(sys::move(candidate)));
    assert(candidate && *candidate == 3);
    OwnerVector moved(sys::move(values));
    assert(values.size() == 2 && !values[0] && !values[1]);
    assert(*moved[0] == 1 && *moved[1] == 2);
    OwnerVector assigned;
    assigned = sys::move(moved);
    assert(assigned.size() == 2 && *assigned[1] == 2 && moved.size() == 2);
    assigned.pop_back();
    assert(assigned.try_push_back(sys::move(candidate)) && !candidate);
    sys::inplace_vector<Tracked, 0> empty;
    int constructed = Tracked::constructed;
    assert(empty.empty() && empty.capacity() == 0 && empty.begin() == empty.end());
    assert(!empty.try_emplace_back(1) && constructed == Tracked::constructed);
    sys::inplace_vector<Immobile, 1> immobile;
    assert(immobile.try_emplace_back(1));
    sys::inplace_vector<Aligned, 2> aligned;
    auto* address = aligned.try_emplace_back(7);
    assert(address && reinterpret_cast<sys::size_t>(address) % alignof(Aligned) == 0);
    sys::inplace_vector<int, 2> numbers;
    assert(numbers.try_push_back(42));
    auto snapshot = numbers;
    numbers.clear();
    assert(snapshot.size() == 1 && snapshot[0] == 42);
    sys::inplace_vector<TrivialMoveOnly, 2> trivial_owner;
    assert(trivial_owner.try_emplace_back(7));
    auto trivial_moved = sys::move(trivial_owner);
    assert(trivial_moved.size() == 1 && trivial_moved[0].value == 7);
    sys::inplace_vector<CustomCopy, 2> custom;
    assert(custom.try_emplace_back(7));
    auto custom_copy = custom;
    assert(custom_copy[0].value == 8);
}
void guard_contract() {
    FakeMutex mutex;
    { sys::lock_guard guard(mutex); assert(mutex.held && mutex.locks == 1); }
    assert(!mutex.held && mutex.unlocks == 1);
    mutex.lock();
    { sys::lock_guard guard(mutex, sys::adopt_lock); assert(mutex.held && mutex.locks == 2); }
    assert(!mutex.held && mutex.unlocks == 2);
}
int main() { lifetime_and_capacity(); move_only_and_zero_capacity(); guard_contract(); }
