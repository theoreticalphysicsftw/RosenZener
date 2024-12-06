// Modified from the original OpenUSD file.
// Copyright 2024 Mihail Mladenov

//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef PXR_BASE_GF_HOMOGENEOUS_H
#define PXR_BASE_GF_HOMOGENEOUS_H

/// \file gf/homogeneous.h
/// \ingroup group_gf_LinearAlgebra
/// Utility functions for Vec4f and Vec4d as homogeneous vectors

#include "pxr/pxr.h"
#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/gf/vec4d.h"
#include "pxr/base/gf/vec4f.h"
#include "pxr/base/gf/api.h"

PXR_NAMESPACE_OPEN_SCOPE

/// Returns a vector which is \p v homogenized.  If the fourth element of \p v
/// is 0, it is set to 1.
/// \ingroup group_gf_LinearAlgebra
GF_API
Vec4f GetHomogenized(const Vec4f &v);

/// Homogenizes \p a and \p b and then performs the cross product on the first
/// three elements of each.  Returns the cross product as a homogenized
/// vector.
/// \ingroup group_gf_LinearAlgebra
GF_API
Vec4f HomogeneousCross(const Vec4f &a, const Vec4f &b);

GF_API
Vec4d GetHomogenized(const Vec4d &v);

/// Homogenizes \p a and \p b and then performs the cross product on the first
/// three elements of each.  Returns the cross product as a homogenized
/// vector.
/// \ingroup group_gf_LinearAlgebra
GF_API
Vec4d HomogeneousCross(const Vec4d &a, const Vec4d &b);

/// Projects homogeneous \p v into Euclidean space and returns the result as a
/// Vec3f.
inline Vec3f Project(const Vec4f &v) {
    float inv = (v[3] != 0.0f) ? 1.0f/v[3] : 1.0f;
    return Vec3f(inv * v[0], inv * v[1], inv * v[2]);
}

/// Projects homogeneous \p v into Euclidean space and returns the result as a
/// Vec3d.
inline Vec3d Project(const Vec4d &v) {
    double inv = (v[3] != 0.0) ? 1.0/v[3] : 1.0;
    return Vec3d(inv * v[0], inv * v[1], inv * v[2]);
}

PXR_NAMESPACE_CLOSE_SCOPE

#endif /* PXR_BASE_GF_HOMOGENEOUS_H */
