// sys/limits.hpp — minimal numeric_limits.
#pragma once

#include "sys/cstdint.hpp"

namespace sys {

template <class T>
struct numeric_limits {
    static constexpr T max() noexcept { return T{}; }
    static constexpr T min() noexcept { return T{}; }
    static constexpr T lowest() noexcept { return T{}; }
};

#define ZSTL_NUMLIM_INT(T, MIN_, MAX_)                                  \
    template <>                                                         \
    struct numeric_limits<T> {                                          \
        static constexpr T max() noexcept { return (MAX_); }            \
        static constexpr T min() noexcept { return (MIN_); }            \
        static constexpr T lowest() noexcept { return (MIN_); }         \
        static constexpr bool is_signed = ((MIN_) < 0);                 \
        static constexpr bool is_integer = true;                        \
    }

ZSTL_NUMLIM_INT(signed char, -128, 127);
ZSTL_NUMLIM_INT(unsigned char, 0, 255);
ZSTL_NUMLIM_INT(char, -128, 127);
ZSTL_NUMLIM_INT(short, -32768, 32767);
ZSTL_NUMLIM_INT(unsigned short, 0, 65535);
ZSTL_NUMLIM_INT(int, (-2147483647 - 1), 2147483647);
ZSTL_NUMLIM_INT(unsigned int, 0u, 4294967295u);
ZSTL_NUMLIM_INT(long long, (-9223372036854775807LL - 1), 9223372036854775807LL);
ZSTL_NUMLIM_INT(unsigned long long, 0ULL, 18446744073709551615ULL);

// long / unsigned long are LP32/LP64-dependent. Use sizeof to pick.
template <>
struct numeric_limits<long> {
    static constexpr long max() noexcept {
        return sizeof(long) == 4 ? 2147483647L : 9223372036854775807LL;
    }
    static constexpr long min() noexcept {
        return sizeof(long) == 4 ? (-2147483647L - 1) : (-9223372036854775807LL - 1);
    }
    static constexpr long lowest() noexcept { return min(); }
};
template <>
struct numeric_limits<unsigned long> {
    static constexpr unsigned long max() noexcept {
        return sizeof(unsigned long) == 4 ? 4294967295UL : 18446744073709551615ULL;
    }
    static constexpr unsigned long min() noexcept { return 0; }
    static constexpr unsigned long lowest() noexcept { return 0; }
};

#undef ZSTL_NUMLIM_INT

template <>
struct numeric_limits<float> {
    static constexpr float max() noexcept { return 3.402823466e+38f; }
    static constexpr float min() noexcept { return 1.175494351e-38f; }
    static constexpr float lowest() noexcept { return -3.402823466e+38f; }
    static constexpr float epsilon() noexcept { return 1.192092896e-7f; }
};

template <>
struct numeric_limits<double> {
    static constexpr double max() noexcept { return 1.7976931348623158e+308; }
    static constexpr double min() noexcept { return 2.2250738585072014e-308; }
    static constexpr double lowest() noexcept { return -1.7976931348623158e+308; }
    static constexpr double epsilon() noexcept { return 2.2204460492503131e-16; }
};

}  // namespace sys
