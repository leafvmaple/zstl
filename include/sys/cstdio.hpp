// sys/cstdio.hpp — minimal <cstdio> replacement.
//
// Forwards the handful of variadic I/O entry points used by zocos to the
// freestanding `<stdio.h>` C header. Note: `stderr` / `stdout` / `stdin` are
// macros in `<stdio.h>`, so callers refer to them unqualified — e.g.
// `sys::fprintf(stderr, ...)`.
#pragma once

#include <stdio.h>  // freestanding C header

namespace sys {

using ::FILE;

using ::fprintf;
using ::snprintf;
using ::printf;
using ::fputs;
using ::fputc;
using ::fwrite;
using ::fflush;

}  // namespace sys
