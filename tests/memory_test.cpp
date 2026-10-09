#include <sys/memory.hpp>

void check(bool condition) { if (!condition) __builtin_trap(); }

struct Counts { int destroyed{}; };
struct Base {
    explicit Base(Counts& counts) : counts(counts) {}
    virtual ~Base() { ++counts.destroyed; }
    Counts& counts;
};
struct Derived : Base { using Base::Base; };

template <class T>
struct StatefulDelete {
    explicit StatefulDelete(int* calls) : calls(calls) {}
    template <class U> StatefulDelete(StatefulDelete<U>&& other) : calls(other.calls) {}
    void operator()(T* object) const noexcept { ++*calls; delete object; }
    int* calls;
};

static_assert(sizeof(sys::unique_ptr<int>) == sizeof(int*));
static_assert(sizeof(sys::unique_ptr<int[]>) == sizeof(int*));
static_assert(sys::is_same<decltype(sys::declval<int&>()), int&>::value);
static_assert(sys::is_convertible<int&, const int&>::value);
static_assert(!sys::is_convertible<int&, int&&>::value);
static_assert(sys::is_convertible<const void, volatile void>::value);
static_assert(!sys::is_constructible<sys::unique_ptr<int>, const sys::unique_ptr<int>&>::value);
static_assert(!sys::is_constructible<sys::unique_ptr<Derived>, sys::unique_ptr<Base>&&>::value);
static_assert(!sys::is_constructible<sys::unique_ptr<Base[]>, Derived*>::value);
static_assert(!sys::is_constructible<sys::unique_ptr<int[]>, sys::unique_ptr<int>&&>::value);
static_assert(!sys::is_constructible<sys::unique_ptr<Base, StatefulDelete<Base>>>::value);

int main() {
    Counts counts;
    int calls = 0;
    {
        sys::unique_ptr<Derived, StatefulDelete<Derived>> source(new Derived(counts), StatefulDelete<Derived>(&calls));
        sys::unique_ptr<Base, StatefulDelete<Base>> target(sys::move(source));
        check(!source && target && target.get_deleter().calls == &calls);
        target.reset();
        check(calls == 1 && counts.destroyed == 1);
    }
    {
        sys::unique_ptr<Derived> source(new Derived(counts));
        sys::unique_ptr<Base> target(new Base(counts));
        target = sys::move(source);
        check(!source && counts.destroyed == 2);
        auto& alias = target;
        target = sys::move(alias);
        check(target && counts.destroyed == 2);
        Base* raw = target.release();
        check(!target);
        sys::unique_ptr<Base> receiver(raw);
    }
    check(counts.destroyed == 3);
    {
        auto first = sys::make_unique<int[]>(4);
        check(first[0] == 0 && first[3] == 0);
        first[3] = 42;
        sys::unique_ptr<const int[]> second(sys::move(first));
        check(!first && second[3] == 42);
        auto third = sys::make_unique<int[]>(2);
        second = sys::move(third);
        check(!third && second[1] == 0);
        second.reset();
        check(!second);
    }
    {
        int a = 0, b = 0;
        sys::unique_ptr<Base, StatefulDelete<Base>> first(new Base(counts), StatefulDelete<Base>(&a));
        sys::unique_ptr<Base, StatefulDelete<Base>> second(new Base(counts), StatefulDelete<Base>(&b));
        first.swap(second);
        check(first.get_deleter().calls == &b && second.get_deleter().calls == &a);
        first = sys::move(second);
        check(b == 1 && !second && first.get_deleter().calls == &a);
        first.reset();
        check(a == 1 && b == 1);
    }
    check(counts.destroyed == 5);
    return 0;
}
