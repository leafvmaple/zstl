// Generic lock ownership; the embedding runtime supplies BasicLockable objects.
#pragma once

namespace sys {
struct adopt_lock_t { explicit adopt_lock_t() = default; };
inline constexpr adopt_lock_t adopt_lock{};

template<class Mutex>
class lock_guard {
public:
    using mutex_type = Mutex;
    explicit lock_guard(Mutex& mutex) : mutex_(mutex) { mutex_.lock(); }
    lock_guard(Mutex& mutex, adopt_lock_t) noexcept : mutex_(mutex) {}
    ~lock_guard() { mutex_.unlock(); }
    lock_guard(const lock_guard&) = delete;
    lock_guard& operator=(const lock_guard&) = delete;
private:
    Mutex& mutex_;
};
} // namespace sys
