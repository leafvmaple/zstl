// sys/cstring.hpp — minimal <cstring> replacement.
//
// Forwards the byte / C-string primitives used by zocos to the freestanding
// `<string.h>` C header.
#pragma once

#include <string.h>  // freestanding C header

namespace sys {

using ::memcpy;
using ::memmove;
using ::memset;
using ::memcmp;
using ::memchr;
using ::strlen;
using ::strcmp;
using ::strncmp;
using ::strcpy;
using ::strncpy;
using ::strchr;
using ::strrchr;
using ::strstr;

}  // namespace sys
