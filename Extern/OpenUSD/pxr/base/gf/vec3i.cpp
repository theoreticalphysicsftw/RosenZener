// Modified from the original OpenUSD file.
// Copyright 2024 Mihail Mladenov

//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
////////////////////////////////////////////////////////////////////////
// This file is generated by a script.  Do not edit directly.  Edit the
// vec.template.cpp file to make changes.

#include "pxr/base/gf/vec3i.h"

#include "pxr/pxr.h"
#include "pxr/base/gf/math.h"
#include "pxr/base/gf/ostreamHelpers.h"
#include "pxr/base/tf/type.h"

// Include headers for other vec types to support wrapping conversions and
// operators.
#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/gf/vec3h.h"

#include <vector>
#include <ostream>

PXR_NAMESPACE_OPEN_SCOPE

TF_REGISTRY_FUNCTION(TfType) {
    TfType::Define<Vec3i>();
}

std::ostream& 
operator<<(std::ostream &out, Vec3i const &v)
{
    return out << '(' 
        << _OstreamHelperP(v[0]) << ", " 
        << _OstreamHelperP(v[1]) << ", " 
        << _OstreamHelperP(v[2]) << ')';
}


bool
Vec3i::operator==(Vec3d const &other) const
{
    return _data[0] == other[0] &&
           _data[1] == other[1] &&
           _data[2] == other[2];
}
bool
Vec3i::operator==(Vec3f const &other) const
{
    return _data[0] == other[0] &&
           _data[1] == other[1] &&
           _data[2] == other[2];
}
bool
Vec3i::operator==(Vec3h const &other) const
{
    return _data[0] == other[0] &&
           _data[1] == other[1] &&
           _data[2] == other[2];
}


PXR_NAMESPACE_CLOSE_SCOPE
