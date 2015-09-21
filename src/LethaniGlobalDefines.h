// -*- c++ -*-
#if !defined(LETHANIGLOBALDEFINES_H)
/* ==========================================================================
   $File: LethaniGlobalDefines.h $
   $Version: 2.1 $
   $Notice: (C) Copyright 2015 Chris Osborne. All Rights Reserved. $
   $License: MIT: http://opensource.org/licenses/MIT $
   ========================================================================== */

#define LETHANIGLOBALDEFINES_H
#include <stdint.h>
#include <cstddef>
// NOTE(Chris): Basic number types
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef float f32;
typedef double f64;
typedef long double f128;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef unsigned uint;

// NOTE(Chris): Static defines
#define FileScope static
#define LocalPersist static
#endif
