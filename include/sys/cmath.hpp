// sys/cmath.hpp — minimal <cmath> replacement.
//
// Forwards float / double math primitives to the freestanding `<math.h>` C
// header. Only the entry points actually exercised by zocos are exposed;
// extend as needed.
#pragma once

#include <math.h>  // freestanding C header

namespace sys {

inline float cos(float x) noexcept { return ::cosf(x); }
inline double cos(double x) noexcept { return ::cos(x); }

inline float sin(float x) noexcept { return ::sinf(x); }
inline double sin(double x) noexcept { return ::sin(x); }

inline float fabs(float x) noexcept { return ::fabsf(x); }
inline double fabs(double x) noexcept { return ::fabs(x); }

inline float ceil(float x) noexcept { return ::ceilf(x); }
inline double ceil(double x) noexcept { return ::ceil(x); }

inline float floor(float x) noexcept { return ::floorf(x); }
inline double floor(double x) noexcept { return ::floor(x); }

inline float sqrt(float x) noexcept { return ::sqrtf(x); }
inline double sqrt(double x) noexcept { return ::sqrt(x); }

inline float pow(float base, float exp) noexcept { return ::powf(base, exp); }
inline double pow(double base, double exp) noexcept { return ::pow(base, exp); }

}  // namespace sys
