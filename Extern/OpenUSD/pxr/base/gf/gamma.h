// Modified from the original OpenUSD file.
// Copyright 2024 Mihail Mladenov

// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef PXR_BASE_GF_GAMMA_H
#define PXR_BASE_GF_GAMMA_H

#include "pxr/pxr.h"
#include "pxr/base/gf/api.h"

/// \file gf/gamma.h
/// Utilities to map colors between gamma spaces.

PXR_NAMESPACE_OPEN_SCOPE

class Vec3f;
class Vec3d;
class Vec4f;
class Vec4d;
class Vec3h;
class Vec4h;

/// Return a new vector with each component of \p v raised to the power \p
/// gamma
GF_API
Vec3f ApplyGamma(const Vec3f &v, double gamma);

/// Return a new vector with each component of \p v raised to the power \p
/// gamma
GF_API
Vec3d ApplyGamma(const Vec3d &v, double gamma);

/// \copydoc ApplyGamma(Vec3d,double)
GF_API
Vec3h ApplyGamma(const Vec3h &v, double gamma);

/// Return a new vector with the first three components of \p v raised to the
/// power \p gamma and the fourth component unchanged.
GF_API
Vec4f ApplyGamma(const Vec4f &v, double gamma);

/// Return a new vector with the first three components of \p v raised to the
/// power \p gamma and the fourth component unchanged.
GF_API
Vec4d ApplyGamma(const Vec4d &v, double gamma);

/// \copydoc ApplyGamma(Vec4h,double)
GF_API
Vec4h ApplyGamma(const Vec4h &v, double gamma);

/// Return a new float raised to the power \p gamma
GF_API
float ApplyGamma(const float &v, double gamma);

/// Return a new char raised to the power \p gamma
GF_API
unsigned char ApplyGamma(const unsigned char &v, double gamma);

/// Return the system display gamma
GF_API
double GetDisplayGamma();

/// Given a vec, \p v, representing an energy-linear RGB(A) color, return a
/// vec of the same type converted to the system's display gamma.
GF_API Vec3f ConvertLinearToDisplay(const Vec3f &v);
GF_API Vec3d ConvertLinearToDisplay(const Vec3d &v);
GF_API Vec3h ConvertLinearToDisplay(const Vec3h &v);
GF_API Vec4f ConvertLinearToDisplay(const Vec4f &v);
GF_API Vec4d ConvertLinearToDisplay(const Vec4d &v);
GF_API Vec4h ConvertLinearToDisplay(const Vec4h &v);
GF_API float ConvertLinearToDisplay(const float &v);
GF_API unsigned char ConvertLinearToDisplay(const unsigned char &v);

/// Given a vec, \p v, representing an RGB(A) color in the system's display
/// gamma space, return an energy-linear vec of the same type.
GF_API Vec3f ConvertDisplayToLinear(const Vec3f &v);
GF_API Vec3d ConvertDisplayToLinear(const Vec3d &v);
GF_API Vec3h ConvertDisplayToLinear(const Vec3h &v);
GF_API Vec4f ConvertDisplayToLinear(const Vec4f &v);
GF_API Vec4d ConvertDisplayToLinear(const Vec4d &v);
GF_API Vec4h ConvertDisplayToLinear(const Vec4h &v);
GF_API float ConvertDisplayToLinear(const float &v);
GF_API unsigned char ConvertDisplayToLinear(const unsigned char &v);

PXR_NAMESPACE_CLOSE_SCOPE

#endif // PXR_BASE_GF_GAMMA_H 
