// sys/cstdio.hpp — minimal <cstdio> replacement.
//
// Declarations only; the embedding runtime supplies stream and output symbols.
#pragma once

#include "sys/cstddef.hpp"
#include "sys/cstdarg.hpp"
#if !defined(ZSTL_RUNTIME_DECLARATIONS_PROVIDED)
// Override with the platform's stream type before including this header.
#ifndef ZSTL_FILE_TYPE
struct zstl_file;
#define ZSTL_FILE_TYPE zstl_file
#endif
using FILE = ZSTL_FILE_TYPE;
extern "C" {
extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;
int fprintf(FILE*, const char*, ...);
int snprintf(char*, sys::size_t, const char*, ...);
int vsnprintf(char*, sys::size_t, const char*, sys::va_list);
int printf(const char*, ...);
int fputs(const char*, FILE*);
int fputc(int, FILE*);
sys::size_t fwrite(const void*, sys::size_t, sys::size_t, FILE*);
int fflush(FILE*);
}
#endif

namespace sys {

using ::FILE;

using ::fprintf;
using ::snprintf;
using ::vsnprintf;
using ::printf;
using ::fputs;
using ::fputc;
using ::fwrite;
using ::fflush;

}  // namespace sys
