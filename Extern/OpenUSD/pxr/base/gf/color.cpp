// Modified from the original OpenUSD file.
// Copyright 2024 Mihail Mladenov

//
// Copyright 2024 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#include "pxr/pxr.h"
#include "pxr/base/gf/color.h"
#include "pxr/base/gf/colorSpace.h"
#include "pxr/base/gf/ostreamHelpers.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/tf/registryManager.h"
#include "pxr/base/tf/type.h"
#include "nc/nanocolor.h"
#include "colorSpace_data.h"
#include <iostream>

PXR_NAMESPACE_OPEN_SCOPE

TF_REGISTRY_FUNCTION(TfType) {
    TfType::Define<Color>();
}

std::ostream& 
operator<<(std::ostream &out, Color const &v)
{
    Vec3f rgb = v.GetRGB();
    return out << '(' 
        << _OstreamHelperP(rgb[0]) << ", " 
        << _OstreamHelperP(rgb[1]) << ", " 
        << _OstreamHelperP(rgb[2]) << ", "
        << _OstreamHelperP(v.GetColorSpace().GetName().GetString()) << ')';
}

// The default constructor creates black, in the "lin_rec709" space.
Color::Color()
: Color(ColorSpace(ColorSpaceNames->LinearRec709))
{
}

// Construct from a colorspace.
Color::Color(const ColorSpace& colorSpace)
: _colorSpace(colorSpace)
, _rgb(0, 0, 0)
{
}

// Construct from an rgb tuple and colorspace.
Color::Color(const Vec3f &rgb, const ColorSpace& colorSpace)
: _colorSpace(colorSpace)
, _rgb(rgb)
{
}

// Construct a color from another color into the specified color space.
Color::Color(const Color &srcColor, const ColorSpace& dstColorSpace)
: _colorSpace(dstColorSpace)
{
    const NcColorSpace* src = srcColor._colorSpace._data->colorSpace;
    const NcColorSpace* dst = dstColorSpace._data->colorSpace;
    NcRGB srcRGB = { srcColor._rgb[0], srcColor._rgb[1], srcColor._rgb[2] };
    NcRGB dstRGB = NcTransformColor(dst, src, srcRGB);
    _rgb = Vec3f(dstRGB.r, dstRGB.g, dstRGB.b);
}

// Set the color from the Planckian locus (blackbody radiation) temperature
// in Kelvin, in the existing color space.
// Values are computed for temperatures between 1000K and 15000K.
// Note that temperatures below 1900K are out of gamut for Rec709.
void Color::SetFromPlanckianLocus(float kelvin, float lumimance)
{
    NcYxy c = NcKelvinToYxy(kelvin, lumimance);
    NcRGB rgb = NcYxyToRGB(_colorSpace._data->colorSpace, c);
    _rgb = Vec3f(rgb.r, rgb.g, rgb.b);
}

// Get the CIEXY coordinate of the color in the chromaticity chart,
// For use in testing.
GF_API
Vec2f Color::_GetChromaticity() const {
    NcRGB src = {_rgb[0], _rgb[1], _rgb[2]};
    NcXYZ rgb = NcRGBToXYZ(_colorSpace._data->colorSpace, src);
    NcYxy chroma = NcXYZToYxy(rgb);
    return Vec2f(chroma.x, chroma.y);
}

// Set the color from a CIEXY coordinate in the chromaticity chart.
// For use in testing.
GF_API
void Color::_SetFromChromaticity(const Vec2f& xy) {
    NcYxy c = { 1.f, xy[0], xy[1] };
    NcRGB rgb = NcYxyToRGB(_colorSpace._data->colorSpace, c);
    _rgb = Vec3f(rgb.r, rgb.g, rgb.b);
}

PXR_NAMESPACE_CLOSE_SCOPE
