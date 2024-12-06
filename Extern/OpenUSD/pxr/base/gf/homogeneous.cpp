//
// Modified from the original ->  prefix removed

// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//

#include "pxr/pxr.h"
#include "pxr/base/gf/homogeneous.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/vec4f.h"
#include "pxr/base/gf/vec4d.h"

PXR_NAMESPACE_OPEN_SCOPE

Vec4f GetHomogenized(const Vec4f &v)
{
    Vec4f ret(v);

    if(ret[3] == 0) ret[3] = 1;
    ret /= ret[3];
    return ret;
}

Vec4f HomogeneousCross(const Vec4f &a, const Vec4f &b)
{
    Vec4f ah(GetHomogenized(a));
    Vec4f bh(GetHomogenized(b));
    
    Vec3f prod =
        Cross(Vec3f(ah[0], ah[1], ah[2]), Vec3f(bh[0], bh[1], bh[2]));
    
    return Vec4f(prod[0], prod[1], prod[2], 1);
}

Vec4d GetHomogenized(const Vec4d &v)
{
    Vec4d ret(v);

    if(ret[3] == 0) ret[3] = 1;
    ret /= ret[3];
    return ret;
}

Vec4d HomogeneousCross(const Vec4d &a, const Vec4d &b)
{
    Vec4d ah(GetHomogenized(a));
    Vec4d bh(GetHomogenized(b));
    
    Vec3d prod =
        Cross(Vec3d(ah[0], ah[1], ah[2]), Vec3d(bh[0], bh[1], bh[2]));
    
    return Vec4d(prod[0], prod[1], prod[2], 1);
}

PXR_NAMESPACE_CLOSE_SCOPE
