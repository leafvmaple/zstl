// sys/cmath.hpp — minimal <cmath> replacement.
//
// No system headers: the embedding runtime supplies the C math symbols.
#pragma once

// Declarations only: the embedding platform supplies the math runtime.
#if !defined(ZSTL_RUNTIME_DECLARATIONS_PROVIDED)
extern "C" {
float cosf(float);
double cos(double);
float sinf(float);
double sin(double);
float fabsf(float);
double fabs(double);
float ceilf(float);
double ceil(double);
float floorf(float);
double floor(double);
float sqrtf(float);
double sqrt(double);
float powf(float, float);
double pow(double, double);
float fmodf(float, float);
double fmod(double, double);
float acosf(float);
double acos(double);
float ldexpf(float, int);
double ldexp(double, int);
}
#endif

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

inline float fmod(float x, float y) noexcept { return ::fmodf(x, y); }
inline double fmod(double x, double y) noexcept { return ::fmod(x, y); }
inline float acos(float x) noexcept { return ::acosf(x); }
inline double acos(double x) noexcept { return ::acos(x); }
inline float ldexp(float x, int exp) noexcept { return ::ldexpf(x, exp); }
inline double ldexp(double x, int exp) noexcept { return ::ldexp(x, exp); }

}  // namespace sys
