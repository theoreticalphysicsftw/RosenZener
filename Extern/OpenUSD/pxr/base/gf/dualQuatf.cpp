// Modified from the original OpenUSD file.
// Copyright 2024 Mihail Mladenov

//
// Copyright 2021 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
////////////////////////////////////////////////////////////////////////
// This file is generated by a script.  Do not edit directly.  Edit the
// dualQuat.template.cpp file to make changes.

#include "pxr/pxr.h"
#include "pxr/base/gf/dualQuatf.h"
#include "pxr/base/gf/ostreamHelpers.h"
#include "pxr/base/tf/type.h"

#include "pxr/base/gf/dualQuatd.h"
#include "pxr/base/gf/dualQuath.h"

PXR_NAMESPACE_OPEN_SCOPE

TF_REGISTRY_FUNCTION(TfType) {
    TfType::Define<DualQuatf>();
}

DualQuatf::DualQuatf(const DualQuatd &other)
    : _real(other.GetReal()) , _dual(other.GetDual())
{
}
DualQuatf::DualQuatf(const DualQuath &other)
    : _real(other.GetReal()) , _dual(other.GetDual())
{
}

std::pair<float, float>
DualQuatf::GetLength() const
{
    const float realLength = _real.GetLength();

    if (realLength == 0)
        return std::pair<float, float>{0, 0};

    return std::pair<float, float>{ realLength, Dot(_real, _dual)/realLength };
}

DualQuatf
DualQuatf::GetNormalized(float eps) const
{
    DualQuatf dq(*this);
    dq.Normalize(eps);

    return dq;
}

std::pair<float, float>
DualQuatf::Normalize(float eps)
{
    const std::pair<float, float> length = GetLength();
    const float realLength = length.first;

    if (realLength < eps) {
        (*this) = DualQuatf::GetIdentity();
    } else {
        const float invRealLength = 1.0 / realLength;
        _real *= invRealLength;
        _dual *= invRealLength;

        _dual -= (Dot(_real, _dual) * _real);
    }

    return length;
}

DualQuatf
DualQuatf::GetConjugate() const
{
    return DualQuatf( _real.GetConjugate(), _dual.GetConjugate() );
}

DualQuatf
DualQuatf::GetInverse() const
{
    // DQ * DQ.GetInverse() == GetIdentity()
    const float realLengthSqr = Dot(_real, _real);

    if ( realLengthSqr <= 0.0 )
        return DualQuatf::GetIdentity();

    const float invRealLengthSqr = 1.0 / realLengthSqr;
    const DualQuatf conjInvLenSqr = GetConjugate() * invRealLengthSqr;
    const Quatf realPart = conjInvLenSqr.GetReal();
    const Quatf dualPart = conjInvLenSqr.GetDual() -
        (2.0 * invRealLengthSqr * Dot(_real, _dual) * conjInvLenSqr.GetReal());

    return DualQuatf( realPart, dualPart );
}

void
DualQuatf::SetTranslation( const Vec3f &translation )
{
    // compute and set the dual part
    _dual = Quatf( 0.0, 0.5*translation ) * _real;
}

Vec3f
DualQuatf::GetTranslation() const
{
    // _dual = Quatf(0, 0.5*translation) * _real
    // => translation = 2 * (_dual * _real.GetConjugate()).GetImaginary()

    // Assume that this dual quaternion is normalized
    TF_DEV_AXIOM(IsClose(_real.GetLength(), 1.0, 0.001));
    const float r1 = _dual.GetReal();
    const float r2 = _real.GetReal();
    const Vec3f &i1 = _dual.GetImaginary();
    const Vec3f &i2 = _real.GetImaginary();

    // Translation of the dual quaternion: -2.0 * (r1*i2 - r2*i1 + i1^i2)
    return Vec3f( -2.0*(r1*i2[0] - r2*i1[0] + (i1[1]*i2[2] - i1[2]*i2[1])),
                               -2.0*(r1*i2[1] - r2*i1[1] + (i1[2]*i2[0] - i1[0]*i2[2])),
                               -2.0*(r1*i2[2] - r2*i1[2] + (i1[0]*i2[1] - i1[1]*i2[0])) );
}

DualQuatf &
DualQuatf::operator *=(const DualQuatf &dq)
{
    const Quatf tempReal = GetReal() * dq.GetReal();
    const Quatf tempDual = GetReal() * dq.GetDual() + GetDual() * dq.GetReal();

    SetReal(tempReal);
    SetDual(tempDual);

    return *this;
}

Vec3f
DualQuatf::Transform(const Vec3f &vec) const
{
    // Apply rotation and translation
    return GetReal().Transform(vec) + GetTranslation();
}

std::ostream &
operator<<(std::ostream &out, const DualQuatf &dq)
{
    return(out << '(' << _OstreamHelperP(dq.GetReal()) << ", "
           << _OstreamHelperP(dq.GetDual()) << ')');
}

PXR_NAMESPACE_CLOSE_SCOPE