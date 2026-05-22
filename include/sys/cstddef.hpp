// sys/cstddef.hpp — minimal <cstddef> replacement.
#pragma once

#include <stddef.h>  // host-provided; freestanding C headers are always available.

namespace sys {

using ::size_t;
using ::ptrdiff_t;
using nullptr_t = decltype(nullptr);

}  // namespace sys
