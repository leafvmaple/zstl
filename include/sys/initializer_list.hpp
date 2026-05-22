// sys/initializer_list.hpp — compiler-magic header.
//
// std::initializer_list is special: the compiler synthesizes it from
// brace-enclosed initializers and looks it up by name in namespace std. So we
// can't fully replace it without the host <initializer_list>. We include the
// host header and re-export under sys.
#pragma once

#include <initializer_list>

namespace sys {

template <class T>
using initializer_list = ::std::initializer_list<T>;

}  // namespace sys
