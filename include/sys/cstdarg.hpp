// Compiler-provided variadic ABI; no hosted headers required.
#pragma once

// A platform runtime may already define the standard macro spellings.
#undef va_start
#undef va_arg
#undef va_end
#undef va_copy

#if defined(__clang__) || defined(__GNUC__)
namespace sys { using va_list = __builtin_va_list; }

#define va_start(ap, last) __builtin_va_start(ap, last)
#define va_arg(ap, type) __builtin_va_arg(ap, type)
#define va_end(ap) __builtin_va_end(ap)
#define va_copy(destination, source) __builtin_va_copy(destination, source)
#elif defined(_MSC_VER) && defined(_M_X64)
namespace sys { using va_list = char*; }
// MSVC's compiler intrinsic sets up the register argument home area.
extern "C" void __cdecl __va_start(sys::va_list*, ...);
#define va_start(ap, last) static_cast<void>(__va_start(&(ap), last))
#define va_arg(ap, type) \
    ((sizeof(type) > 8 || (sizeof(type) & (sizeof(type) - 1)) != 0) \
        ? **reinterpret_cast<type**>(((ap) += 8) - 8) \
        : *reinterpret_cast<type*>(((ap) += 8) - 8))
#define va_end(ap) static_cast<void>((ap) = nullptr)
#define va_copy(destination, source) static_cast<void>((destination) = (source))
#else
#error "Unsupported compiler variadic ABI (supported: Clang, GCC, MSVC x64)"
#endif
