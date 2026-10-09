// sys/cstddef.hpp — minimal <cstddef> replacement.
#pragma once

namespace sys {

using size_t = decltype(sizeof(0));
using ptrdiff_t = decltype(static_cast<char*>(nullptr) - static_cast<char*>(nullptr));
using nullptr_t = decltype(nullptr);

}  // namespace sys
