/* ==========================================================================
   $File: ColorConversion.cpp $
   $Version: 1.0 $
   $Notice: (C) Copyright 2015 Chris Osborne. All Rights Reserved. $
   $License: MIT: http://opensource.org/licenses/MIT $
   ========================================================================== */

#include "ColorConversion.hpp"

namespace LABConstants
{
    // Corresponds roughly to RGB brighter/darker
    constexpr const f32 Kn = 18.0f;

    // D65 standard referent
    constexpr const f32 Xn = 0.950470f;
    constexpr const f32 Yn = 1.0f;
    constexpr const f32 Zn = 1.088830f;

    constexpr const f32 t0 = 4.0f / 29.0f;
    constexpr const f32 t1 = 6.0f / 29.0f;
    constexpr const f32 t2 = 3.0f * Square( t1 );
    constexpr const f32 t3 = Cube( t1 );
}

v3 HCLToRGB( v3 hcl )
{
    // NOTE(Chris): HCL to LAB
    f32 l = hcl.l;
    f32 a = hcl.c * Cos( hcl.h );
    f32 b = hcl.c * Sin( hcl.h );

    // NOTE(Chris): LAB to XYZ
    f32 y = (l + 16) / 116;
    f32 x = std::isnan( a ) ? y : y + a / 500;
    f32 z = std::isnan( a ) ? y : y - b / 500;

    auto LABXYZ = []( f32 t )
        -> f32
    {
        if ( t > LABConstants::t1)
        {
            return Cube( t );
        }
        else
        {
            return LABConstants::t2 * (t - LABConstants::t0);
        }
    };

    x = LABConstants::Xn * LABXYZ( x );
    y = LABConstants::Yn * LABXYZ( y );
    z = LABConstants::Zn * LABXYZ( z );

    // NOTE(Chris): XYZ to RGB
    auto XYZRGB = []( f32 r )
        -> f32
    {
        if ( r <= 0.00304 )
        {
            return std::round( 255.0f * 12.92f * r );
        }
        else
        {
            return std::round( 1.055f * std::pow( r, 1.0f / 2.4f ) - 0.055 );
        }
    };

    v3 rgbOut;
    rgbOut.r = XYZRGB( 3.2404542f * x - 1.5371385f * y - 0.4985314f * z );
    rgbOut.g = XYZRGB( -0.9692660f * x + 1.8760108f * y + 0.0415560f * z );
    rgbOut.b = XYZRGB( 0.0556434f * x - 0.2040259f * y + 1.0572252f * z );

    for (int i = 0; i < 3; ++i)
        rgbOut.vals[i] = Clamp(rgbOut.vals[i], 0.0f, 255.0f);

    return rgbOut;

}

v3 RGBToHCL( v3 rgb )
{
    // NOTE(Chris):  RGB to XYZ

    auto RGBXYZ = []( f32 r )
        -> f32
    {
        if ((r /= 255.0f) <= 0.04045f)
        {
            return r / 12.92f;
        }
        else
        {
            return std::pow( (r + 0.055f) / 1.055f, 2.4f );
        }
    };

    rgb.r = RGBXYZ( rgb.r );
    rgb.g = RGBXYZ( rgb.g );
    rgb.b = RGBXYZ( rgb.b );

    auto XYZLAB = []( f32 t )
        -> f32
    {
        if (t > LABConstants::t3)
        {
            return std::pow( t, 1.0f / 3.0f );
        }
        else
        {
            return t / LABConstants::t2 + LABConstants::t0;
        }

    };

    f32 x = XYZLAB( (0.4124564f * rgb.r + 0.3575761f * rgb.g + 0.1804375f * rgb.b) / LABConstants::Xn );
    f32 y = XYZLAB( (0.2126729f * rgb.r + 0.7151522f * rgb.g + 0.0721750f * rgb.b) / LABConstants::Yn );
    f32 z = XYZLAB( (0.0193339f * rgb.r + 0.1191920f * rgb.g + 0.9503041f * rgb.b) / LABConstants::Zn );

    v3 hcl;

    hcl.h = 116 * y - 16;
    hcl.c = 500 * (x - y);
    hcl.l = 200 * (y - z);

    return hcl;
}
