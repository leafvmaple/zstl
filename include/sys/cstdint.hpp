// sys/cstdint.hpp — minimal <cstdint> replacement.
#pragma once

#include <stdint.h>  // freestanding C header

namespace sys {

using ::int8_t;
using ::int16_t;
using ::int32_t;
using ::int64_t;
using ::uint8_t;
using ::uint16_t;
using ::uint32_t;
using ::uint64_t;
using ::intptr_t;
using ::uintptr_t;
using ::intmax_t;
using ::uintmax_t;

}  // namespace sys
