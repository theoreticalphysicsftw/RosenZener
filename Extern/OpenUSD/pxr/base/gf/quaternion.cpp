//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//

#include "pxr/pxr.h"
#include "pxr/base/gf/quaternion.h"

#include "pxr/base/gf/ostreamHelpers.h"
#include "pxr/base/tf/type.h"

#include <ostream>

PXR_NAMESPACE_OPEN_SCOPE

// CODE_COVERAGE_OFF_GCOV_BUG
TF_REGISTRY_FUNCTION(TfType) {
    TfType::Define<Quaternion>();
}
// CODE_COVERAGE_ON_GCOV_BUG

double
Quaternion::GetLength() const
{
    return sqrt(_GetLengthSquared());
}


Quaternion
Quaternion::GetNormalized(double eps) const
{
    double length = sqrt(_GetLengthSquared());

    if (length < eps)
        return GetIdentity();
    else
        return (*this) / length;
}

double
Quaternion::Normalize(double eps)
{
    double length = sqrt(_GetLengthSquared());

    if (length < eps)
        *this = GetIdentity();
    else
        *this /= length;

    return length;
}

Quaternion
Quaternion::GetInverse() const
{
    return Quaternion(GetReal(), -GetImaginary()) / _GetLengthSquared();
}

Quaternion &
Quaternion::operator *=(const Quaternion &q)
{
    double r1         =   GetReal();
    double r2         = q.GetReal();
    const Vec3d &i1 =   GetImaginary();
    const Vec3d &i2 = q.GetImaginary();

    double r = r1 * r2 - Dot(i1, i2);

    Vec3d i(r1 * i2[0] + r2 * i1[0] + (i1[1] * i2[2] - i1[2] * i2[1]),
	      r1 * i2[1] + r2 * i1[1] + (i1[2] * i2[0] - i1[0] * i2[2]),
	      r1 * i2[2] + r2 * i1[2] + (i1[0] * i2[1] - i1[1] * i2[0]));

    SetReal(r);
    SetImaginary(i);

    return *this;
}

Quaternion &
Quaternion::operator *=(double s)
{
    _real      *= s;
    _imaginary *= s;
    return *this;
}

Quaternion
Slerp(const Quaternion& q0, const Quaternion& q1, double alpha)
{
    return Slerp(alpha, q0, q1);
}

Quaternion
Slerp(double alpha, const Quaternion& q0, const Quaternion& q1)
{
    double  cosTheta = q0.GetImaginary() * q1.GetImaginary() +
                         q0.GetReal() * q1.GetReal();
    bool    flip1 = false;

    if (cosTheta < 0.0) {
        cosTheta = -cosTheta;
        flip1 = true;
    }

    double scale0, scale1;

    if (1.0 - cosTheta > 0.00001 ) {
        // standard case
        double theta = acos(cosTheta),
               sinTheta = sin(theta);

        scale0 = sin((1.0 - alpha) * theta) / sinTheta;
        scale1 = sin(alpha * theta) / sinTheta;
    } else {        
        // rot0 and rot1 very close - just do linear interp and renormalize.
        scale0 = 1.0 - alpha;
        scale1 = alpha;
    }

    if (flip1)
        scale1 = -scale1;
    
    return scale0 * q0 + scale1 * q1;
}

std::ostream &
operator<<(std::ostream& out, const Quaternion& q)
{
    return(out << '(' << _OstreamHelperP(q.GetReal()) << " + " 
           << _OstreamHelperP(q.GetImaginary()) << ')');
}

PXR_NAMESPACE_CLOSE_SCOPE
