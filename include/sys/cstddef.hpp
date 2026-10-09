// sys/cstddef.hpp — minimal <cstddef> replacement.
#pragma once

namespace sys {

using size_t = __SIZE_TYPE__;
using ptrdiff_t = __PTRDIFF_TYPE__;
using nullptr_t = decltype(nullptr);

}  // namespace sys
