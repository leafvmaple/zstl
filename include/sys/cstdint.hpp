// sys/cstdint.hpp — minimal <cstdint> replacement.
#pragma once

namespace sys {

#if defined(__INT8_TYPE__)
using int8_t = __INT8_TYPE__;
using int16_t = __INT16_TYPE__;
using int32_t = __INT32_TYPE__;
using int64_t = __INT64_TYPE__;
using uint8_t = __UINT8_TYPE__;
using uint16_t = __UINT16_TYPE__;
using uint32_t = __UINT32_TYPE__;
using uint64_t = __UINT64_TYPE__;
using intptr_t = __INTPTR_TYPE__;
using uintptr_t = __UINTPTR_TYPE__;
using intmax_t = __INTMAX_TYPE__;
using uintmax_t = __UINTMAX_TYPE__;
#elif defined(_MSC_VER)
using int8_t = signed char;
using int16_t = short;
using int32_t = int;
using int64_t = long long;
using uint8_t = unsigned char;
using uint16_t = unsigned short;
using uint32_t = unsigned int;
using uint64_t = unsigned long long;
#if defined(_WIN64)
using intptr_t = int64_t;
using uintptr_t = uint64_t;
#else
using intptr_t = int32_t;
using uintptr_t = uint32_t;
#endif
using intmax_t = int64_t;
using uintmax_t = uint64_t;
#else
#error "zstl requires compiler integer type definitions"
#endif

}  // namespace sys
