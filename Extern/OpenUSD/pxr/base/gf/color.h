// Modified from the original OpenUSD file.
// Copyright 2024 Mihail Mladenov

// Copyright 2024 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef PXR_BASE_GF_COLOR_H
#define PXR_BASE_GF_COLOR_H

/// \file gf/color.h
/// \ingroup group_gf_Color

#include "pxr/pxr.h"
#include "pxr/base/gf/colorSpace.h"
#include "pxr/base/gf/vec2f.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/gf/matrix3f.h"
#include "pxr/base/gf/api.h"

#include <iosfwd>

PXR_NAMESPACE_OPEN_SCOPE

/// \class Color
/// \brief Represents a color in a specific color space.
/// \ingroup group_gf_Color
/// 
/// Basic type: Color
///
/// The Color class represents a color in a specific color space. It provides
/// various methods for constructing, manipulating, and retrieving color values.
///
/// The color values are stored as an RGB tuple and are associated with a color
/// space. The color space determines the interpretation of the RGB values. The
/// values are colorimetric, but not photometric as there is no normalizing
/// constant (such as a luminance factor).
///
/// This class provides methods for setting and getting color values, converting
/// between color spaces, normalizing luminance, and comparing colors.

class Color {
public:
    /// The default constructor creates black, in the "lin_rec709" color space.
    GF_API Color();

    /// Construct a black color in the given color space.
    /// \param colorSpace The color space.
    GF_API
    explicit Color(const ColorSpace& colorSpace);

    /// Construct a color from an RGB tuple and color space.
    /// \param rgb The RGB tuple (red, green, blue), in the color space
    /// provided.
    /// \param colorSpace The color space.
    GF_API
    Color(const Vec3f &rgb, const ColorSpace& colorSpace);

    /// Construct a color by converting the source color into the specified color space.
    /// \param color The color to convert, in its color space.
    /// \param colorSpace The desired color space.
    GF_API
    Color(const Color &color, const ColorSpace& colorSpace);

    /// Set the color from the Planckian locus (blackbody radiation) temperature
    /// in Kelvin, in the existing color space.
    /// Values are computed for temperatures between 1000K and 15000K.
    /// Note that temperatures below 1900K are out of gamut for Rec709.
    /// \param kelvin The temperature in Kelvin.
    /// \param luminance The desired luminance.
    GF_API
    void SetFromPlanckianLocus(float kelvin, float luminance);

    /// Get the RGB tuple.
    /// \return The RGB tuple.
    Vec3f GetRGB() const { return _rgb; }

    /// Get the color space.
    /// \return The color space.
    ColorSpace GetColorSpace() const { return _colorSpace; }

    /// Equality operator.
    /// \param rh The right-hand side color.
    /// \return True if the colors are equal, false otherwise.
    bool operator ==(const Color &rh) const {
        return _rgb == rh._rgb && _colorSpace == rh._colorSpace;
    }

    /// Inequality operator.
    /// \param r The right-hand side color.
    /// \return True if the colors are not equal, false otherwise.
    bool operator !=(const Color &rh) const { return !(*this == rh); }

protected:
    ColorSpace _colorSpace; ///< The color space.
    Vec3f      _rgb;        ///< The RGB tuple.

    // Get the CIEXY coordinate of the color in the chromaticity chart,
    // For use in testing.
    GF_API
    Vec2f _GetChromaticity() const;

    // Set the color from a CIEXY coordinate in the chromaticity chart.
    // For use in testing.
    GF_API
    void _SetFromChromaticity(const Vec2f& xy);
};


/// Tests for equality of the RGB tuple in a color with a given tolerance, 
/// returning \c true if the length of the difference vector is less than or 
/// equal to \p tolerance. This comparison does not adapt the colors to the
/// same color space before comparing, and is not a perceptual comparison.
inline bool
IsClose(Color const &c1, Color const &c2, double tolerance)
{
    return IsClose(c1.GetRGB(), c2.GetRGB(), tolerance);
}

/// Output a Color.
/// \ingroup group_gf_DebuggingOutput
/// @brief Stream insertion operator for outputting Color objects to an output stream.
///
/// This operator allows Color objects to be written to an output stream.
///
/// @param os The output stream to write to.
/// @param color The Color object to be outputted.
/// @return The output stream after writing the Color object.
std::ostream& operator<<(std::ostream &, Color const &);

PXR_NAMESPACE_CLOSE_SCOPE

#endif // PXR_BASE_GF_COLOR_H