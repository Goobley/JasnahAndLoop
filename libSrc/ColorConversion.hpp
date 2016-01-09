// -*- c++ -*-
#if !defined(COLORCONVERSION_H)
/* ==========================================================================
   $File: ColorConversion.hpp $
   $Version: 1.0 $
   $Notice: (C) Copyright 2015 Chris Osborne. All Rights Reserved. $
   $License: MIT: http://opensource.org/licenses/MIT $
   ========================================================================== */
/* ==========================================================================
   The following routines are entirely based on Chroma.js. (C) Gregor
   Aisch under BSD License
   ========================================================================== */

#define COLORCONVERSION_H
#include "../src/LethaniGlobalDefines.h"
#include "BasicMath.hpp"

/// Colorspace conversion routines
v3 HCLToRGB( v3 hcl );
v3 RGBToHCL( v3 rgb );

#endif
