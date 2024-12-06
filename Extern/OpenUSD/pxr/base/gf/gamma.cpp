// Modified from the original OpenUSD file.
// Copyright 2024 Mihail Mladenov

//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//

#include "pxr/pxr.h"
#include "pxr/base/gf/gamma.h"

#include "pxr/base/gf/math.h"
#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/gf/vec4d.h"
#include "pxr/base/gf/vec4f.h"

PXR_NAMESPACE_OPEN_SCOPE

// Display colors (such as colors for UI elements) are always gamma 2.2
// and aspects of interactive rendering such as OpenGL's sRGB texture
// format assume that space as well. So, gamma 2.2 is hard coded here
// as the display gamma. In the future if those assumptions change we may
// need to move this to a higher level and get the gamma from somewhere else.
static const double _DisplayGamma = 2.2;

double GetDisplayGamma() {
    return _DisplayGamma;
}

Vec3f ApplyGamma(const Vec3f &v, double g) {
    return Vec3f(pow(v[0],g),pow(v[1],g),pow(v[2],g));
}

Vec3d ApplyGamma(const Vec3d &v, double g) {
    return Vec3d(pow(v[0],g),pow(v[1],g),pow(v[2],g));
}

Vec3h ApplyGamma(const Vec3h &v, double g) {
    // Explicitly cast half to float to avoid ambiguous call to pow(...)
    return Vec3h(pow(static_cast<float>(v[0]),g),
                   pow(static_cast<float>(v[1]),g),
                   pow(static_cast<float>(v[2]),g));
}

Vec4f ApplyGamma(const Vec4f &v, double g) {
    return Vec4f(pow(v[0],g),pow(v[1],g),pow(v[2],g),v[3]);
}

Vec4d ApplyGamma(const Vec4d &v, double g) {
    return Vec4d(pow(v[0],g),pow(v[1],g),pow(v[2],g),v[3]);
}

Vec4h ApplyGamma(const Vec4h &v, double g) {
    // Explicitly cast half to float to avoid ambiguous call to pow(...)
    return Vec4h(pow(static_cast<float>(v[0]),g),
                   pow(static_cast<float>(v[1]),g),
                   pow(static_cast<float>(v[2]),g),
                   v[3]);
}

float ApplyGamma(const float &v, double g)
{
    return pow(v, g);
}

unsigned char ApplyGamma(const unsigned char &v, double g)
{
    return (unsigned char) (pow((v / 255.0), g) * 255);
}


template <class T>
static T _ConvertLinearToDisplay(const T& v) {
    return ApplyGamma(v,1.0/_DisplayGamma);
}

template <class T>
static T _ConvertDisplayToLinear(const T& v) {
    return ApplyGamma(v,_DisplayGamma);
}

Vec3f ConvertLinearToDisplay(const Vec3f &v) { return _ConvertLinearToDisplay(v); }
Vec3d ConvertLinearToDisplay(const Vec3d &v) { return _ConvertLinearToDisplay(v); }
Vec3h ConvertLinearToDisplay(const Vec3h &v) { return _ConvertLinearToDisplay(v); }
Vec4f ConvertLinearToDisplay(const Vec4f &v) { return _ConvertLinearToDisplay(v); }
Vec4d ConvertLinearToDisplay(const Vec4d &v) { return _ConvertLinearToDisplay(v); }
Vec4h ConvertLinearToDisplay(const Vec4h &v) { return _ConvertLinearToDisplay(v); }
float ConvertLinearToDisplay(const float &v)   { return _ConvertLinearToDisplay(v); }
unsigned char ConvertLinearToDisplay(const unsigned char &v) { return _ConvertLinearToDisplay(v); }

Vec3f ConvertDisplayToLinear(const Vec3f &v) { return _ConvertDisplayToLinear(v); }
Vec3d ConvertDisplayToLinear(const Vec3d &v) { return _ConvertDisplayToLinear(v); }
Vec3h ConvertDisplayToLinear(const Vec3h &v) { return _ConvertDisplayToLinear(v); }
Vec4f ConvertDisplayToLinear(const Vec4f &v) { return _ConvertDisplayToLinear(v); }
Vec4d ConvertDisplayToLinear(const Vec4d &v) { return _ConvertDisplayToLinear(v); }
Vec4h ConvertDisplayToLinear(const Vec4h &v) { return _ConvertDisplayToLinear(v); }
float ConvertDisplayToLinear(const float &v)   { return _ConvertDisplayToLinear(v); }
unsigned char ConvertDisplayToLinear(const unsigned char &v) { return _ConvertDisplayToLinear(v); }

PXR_NAMESPACE_CLOSE_SCOPE
