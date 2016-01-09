// -*- c++ -*-
#if !defined(BASICMATH_H)
/* ==========================================================================
   $File: BasicMath.hpp $
   $Version: 1.0 $
   $Notice: (C) Copyright 2015 Chris Osborne. All Rights Reserved. $
   $License: MIT: http://opensource.org/licenses/MIT $
   ========================================================================== */

#define BASICMATH_H
#include <cmath>
#include "../src/LethaniGlobalDefines.h"

union v2
{
    struct
    {
        f32 x, y;
    };
    struct
    {
        f32 u, v;
    };
    f32 vals[2];
};

union v3
{
    struct
    {
        f32 x, y, z;
    };
    struct
    {
        f32 u, v, w;
    };
    struct
    {
        f32 r, g, b;
    };
    struct
    {
        f32 h, c, l;
    };
    struct
    {
        v2 xy;
        f32 ignored0;
    };
    struct
    {
        f32 ignored1;
        v2 yz;
    };
    struct
    {
        v2 uv;
        f32 ignored3;
    };
    struct
    {
        f32 ignored4;
        v2 vw;
    };
    f32 vals[3];
};

namespace Constant {
/// pi 32
const f32 Pi = 3.1415926535897931160f;
/// tau 32
const f32 Tau = 2.0f*3.1415926535897931160f;
/// degrees to rads 32
const f32 DegToRad = Pi/180.0f;
/// rads to degrees 32
const f32 RadToDeg = 180.0f/Pi;
/// Epsilon 32
const f32 Epsilon = 1.0e-5;
/// pi 64
const f64 Pi64 = 3.1415926535897931160;
/// tau 64
const f64 Tau64 = 2.0*3.1415926535897931160;
/// degrees to rads 64
const f64 DegToRad64 = Pi64/180.0;
/// rads to degrees 64
const f64 RadToDeg64 = 180.0/Pi64;
/// Epsilon 64
const f64 Epsilon64 = 1.0e-8;
}

/// Templated square function
template<typename T> inline constexpr
T Square(T num) { return num*num; }

/// Templated cube function
template<typename T> inline constexpr
T Cube(T num) { return num*num*num; }

// Float math
/// Check whether two floating point values are equal within accuracy
inline bool Equals(f32 lhs, f32 rhs) { return lhs + Constant::Epsilon >= rhs && lhs - Constant::Epsilon <= rhs; }
/// Linear interpolation between two float values
inline f32 Lerp(f32 lhs, f32 rhs, f32 t) { return lhs * (1.0f - t) + rhs * t; }
/// Return the smaller of two floats
inline f32 Min(f32 lhs, f32 rhs) { return lhs < rhs ? lhs : rhs; }
/// Return the larger of two floats
inline f32 Max(float lhs, f32 rhs) { return lhs > rhs ? lhs : rhs; }
/// Return absolute value of a float
inline f32 Abs(f32 value) { return value >= 0.0f ? value : -value; }
/// Return the sign of a float (-1, 0 or 1)
inline f32 Sign(f32 value) { return value > 0.0f ? 1.0f : (value < 0.0f ? -1.0f : 0.0f); }

// Double math
/// Check whether two f64 are equal within accuracy
inline bool Equals(f64 lhs, f64 rhs) { return lhs + Constant::Epsilon64 >= rhs && lhs - Constant::Epsilon64 <= rhs; }
/// Linear interpolation between two f64
inline f64 Lerp(f64 lhs, f64 rhs, f64 t) { return lhs * (1.0f - t) + rhs * t; }
/// Return the smaller of two f64
inline f64 Min(f64 lhs, f64 rhs) { return lhs < rhs ? lhs : rhs; }
/// Return the larger of two f64
inline f64 Max(f64 lhs, f64 rhs) { return lhs > rhs ? lhs : rhs; }
/// Return absolute value of a f64
inline f64 Abs(f64 value) { return value >= 0.0f ? value : -value; }
/// Return the sign of a f64 (-1, 0 or 1)
inline f64 Sign(f64 value) { return value > 0.0f ? 1.0f : (value < 0.0f ? -1.0f : 0.0f); }

/// Check whether two elements (f64) are equal within a specified tolerance
inline bool EqualsTol(f64 lhs, f64 rhs, f64 tol) { return lhs + tol >= rhs && lhs - tol <= rhs; }
/// Check whether two elements (f32) are equal within a specified tolerance
inline bool EqualsTol(f32 lhs, f32 rhs, f32 tol) { return lhs + tol >= rhs && lhs - tol <= rhs; }

/// Clamp a float to a range
inline f32 Clamp(f32 value, f32 min, f32 max)
{
    if (value < min)
        return min;
    else if (value > max)
        return max;
    else
        return value;
}

/// Clamp a f64 to a range
inline f64 Clamp(f64 value, f64 min, f64 max)
{
    if (value < min)
        return min;
    else if (value > max)
        return max;
    else
        return value;
}

/// Smoothly damp between values
inline f32 SmoothStep(f32 lhs, f32 rhs, f32 t)
{
    t = Clamp((t - lhs) / (rhs - lhs), 0.0f, 1.0f); // Saturate t
    return t * t * (3.0f - 2.0f * t);
}

/// Smoothly damp between values
inline f64 SmoothStep(f64 lhs, f64 rhs, f64 t)
{
    t = Clamp((t - lhs) / (rhs - lhs), 0.0, 1.0); // Saturate t
    return t * t * (3.0 - 2.0 * t);
}

/// Return sine of an angle in degrees
inline f32 Sin(f32 angle) { return sinf(angle * Constant::DegToRad); }
/// Return cosine of an angle in degrees
inline f32 Cos(f32 angle) { return cosf(angle * Constant::DegToRad); }
/// Return tangent of an angle in degrees
inline f32 Tan(f32 angle) { return tanf(angle * Constant::DegToRad); }
/// Return arc sine in degrees
inline f32 Asin(f32 x) { return 1.0f / Constant::DegToRad * asinf(Clamp(x, -1.0f, 1.0f)); }
/// Return arc cosine in degrees
inline f32 Acos(f32 x) { return 1.0f / Constant::DegToRad * acosf(Clamp(x, -1.0f, 1.0f)); }
/// Return arc tangent in degrees
inline f32 Atan(f32 x) { return 1.0f / Constant::DegToRad * atanf(x); }
/// Return arc tangent of y/x in degrees
inline f32 Atan2(f32 y, f32 x) { return 1.0f / Constant::DegToRad * atan2f(y, x); }

/// Return sine of an angle in degrees
inline f64 Sin(f64 angle) { return sin(angle * Constant::DegToRad64); }
/// Return cosine of an angle in degrees
inline f64 Cos(f64 angle) { return cos(angle * Constant::DegToRad64); }
/// Return tangent of an angle in degrees
inline f64 Tan(f64 angle) { return tan(angle * Constant::DegToRad64); }
/// Return arc sine in degrees
inline f64 Asin(f64 x) { return 1.0 / Constant::DegToRad64 * asin(Clamp(x, -1.0, 1.0)); }
/// Return arc cosine in degrees
inline f64 Acos(f64 x) { return 1.0 / Constant::DegToRad64 * acos(Clamp(x, -1.0, 1.0)); }
/// Return arc tangent in degrees
inline f64 Atan(f64 x) { return 1.0 / Constant::DegToRad64 * atan(x); }
/// Return arc tangent of y/x in degrees
inline f64 Atan2(f64 y, f64 x) { return 1.0 / Constant::DegToRad64 * atan2(y, x); }

/// Return the smaller of two integers
inline int Min(int lhs, int rhs) { return lhs < rhs ? lhs : rhs; }
/// Return the larger of two integers
inline int Max(int lhs, int rhs) { return lhs > rhs ? lhs : rhs; }
/// Return absolute value of an integer
inline int Abs(int value) { return value >= 0 ? value : -value; }

/// Clamp an integer to a range
inline int Clamp(int value, int min, int max)
{
    if (value < min)
        return min;
    else if (value > max)
        return max;
    else
        return value;
}

/// Check whether an unsigned integer is a power of two
inline bool IsPowerOfTwo(unsigned value)
{
    if (!value)
        return true;
    while (!(value & 1))
        value >>= 1;
    return value == 1;
}

/// Round up to next power of two
inline unsigned NextPowerOfTwo(unsigned value)
{
    unsigned ret = 1;
    while (ret < value && ret < 0x80000000)
        ret <<= 1;
    return ret;
}

/// Count the number of set bits in a mask
inline unsigned CountSetBits(unsigned value)
{
    // Brian Kernighan's method
    unsigned count = 0;
    for (count = 0; value; count++)
        value &= value - 1;
    return count;
}

#endif
