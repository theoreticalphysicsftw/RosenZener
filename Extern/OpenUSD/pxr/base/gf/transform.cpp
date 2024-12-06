// Modified from the original OpenUSD file.
// Copyright 2024 Mihail Mladenov

//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//

#include "pxr/pxr.h"
#include "pxr/base/gf/matrix4d.h"
#include "pxr/base/gf/transform.h"

#include "pxr/base/tf/type.h"

#include <ostream>

PXR_NAMESPACE_OPEN_SCOPE

// CODE_COVERAGE_OFF_GCOV_BUG
TF_REGISTRY_FUNCTION(TfType) {
    TfType::Define<Transform>();
}
// CODE_COVERAGE_ON_GCOV_BUG

Transform &
Transform::Set(const Vec3d &scale,
		 const Rotation &pivotOrientation,
		 const Rotation &rotation,
		 const Vec3d &pivotPosition,
		 const Vec3d &translation)
{
    _scale = scale;
    _pivotOrientation = pivotOrientation;
    _rotation = rotation;
    _pivotPosition = pivotPosition;
    _translation = translation;

    return *this;
}
    
Transform &
Transform::SetMatrix(const Matrix4d &m)
{
    // Factor the matrix into the components, while trying to leave
    // the pivotPosition field unchanged.

    // Create a matrix, [mNoPivot], given by:
    //          [pivotPositionInverse][mNoPivot][pivotPosition] = [m]
    //
    // So, [mNoPivot] = [pivotPosition][m][pivotPositionInverse]
    Matrix4d mPivotPos, mPivotPosInv;
    mPivotPos.SetTranslate(_pivotPosition);
    mPivotPosInv.SetTranslate(-_pivotPosition);
    Matrix4d mNoPivot = mPivotPos * m * mPivotPosInv;

    // Factor mNoPivot into the other components
    Matrix4d shearRotMat, rotMat, projMat;

    // Factor returns false if the given matrix is singular,
    // but produces a result anyway.  We use that result regardless,
    // because singular matrices (such as zero scales) are still
    // valid for constructing Transforms.
    mNoPivot.Factor(&shearRotMat, &_scale, &rotMat,
                     &_translation, &projMat);

    _rotation = rotMat.ExtractRotation();

    // Don't set the scale orientation if the scale is unity
    if (_scale != Vec3d(1.0, 1.0, 1.0))
        _pivotOrientation = shearRotMat.GetTranspose().ExtractRotation();
    else
        _pivotOrientation.SetIdentity();

    return *this;
}

Transform &
Transform::SetIdentity()
{
    _scale.Set(1.0, 1.0, 1.0);
    _pivotOrientation.SetIdentity();
    _rotation.SetIdentity();
    _pivotPosition      = Vec3d(0);
    _translation = Vec3d(0);

    return *this;
}

Matrix4d
Transform::GetMatrix() const
{
    bool        doPivot      = (_pivotPosition != Vec3d(0));
    bool        doScale       = (_scale  != Vec3d(1.0, 1.0, 1.0));
    bool        doScaleOrient = (_pivotOrientation.GetAngle() != 0.0);
    bool        doRotation    = (_rotation.GetAngle() != 0.0);
    bool        doTranslation = (_translation != Vec3d(0));
    bool        anySet        = false;
    Matrix4d  mtx;

    //
    // When multiplying matrices A*B, the effects of A are more local
    // than the effects of B (A's operation takes place before
    // B's). So we use post-multiplication of matrices (with the '*='
    // operator), in the order we want the operations to be applied.
    //

#define _GF_ACCUM(mtxOp)                        \
    {                                           \
        if (anySet) {                           \
            Matrix4d tmp;                     \
            tmp.mtxOp;                          \
            mtx *= tmp;                         \
        }                                       \
        else {                                  \
            mtx.mtxOp;                          \
            anySet = true;                      \
        }                                       \
    }

    if (doPivot)
        _GF_ACCUM(SetTranslate(-_pivotPosition));

    if (doScale) {
        if (doScaleOrient)
            _GF_ACCUM(SetRotate(_pivotOrientation.GetInverse()));

        _GF_ACCUM(SetScale(_scale));

        if (doScaleOrient)
            _GF_ACCUM(SetRotate(_pivotOrientation));
    }

    if (doRotation)
        _GF_ACCUM(SetRotate(_rotation));

    if (doPivot)
        _GF_ACCUM(SetTranslate(_pivotPosition));

    if (doTranslation)
        _GF_ACCUM(SetTranslate(_translation));

#undef _GF_ACCUM

    if (! anySet)
        mtx.SetIdentity();

    return mtx;
}

bool
Transform::operator ==(const Transform &xf) const
{
    return (GetScale()            == xf.GetScale() &&
	    GetPivotOrientation() == xf.GetPivotOrientation() &&
	    GetRotation()         == xf.GetRotation() &&
	    GetPivotPosition()    == xf.GetPivotPosition() &&
	    GetTranslation()      == xf.GetTranslation());
}

Transform &
Transform::operator *=(const Transform &xf)
{
    return SetMatrix(GetMatrix() * xf.GetMatrix());
}

std::ostream &
operator<<(std::ostream& out, const Transform& xf)
{
    const Vec3d &t = xf.GetTranslation();

    const Rotation &rotation = xf.GetRotation();
    const Vec3d &rax = rotation.GetAxis();
    double rang = rotation.GetAngle();

    const Vec3d &s = xf.GetScale();

    const Vec3d &c = xf.GetPivotPosition();

    const Rotation &pivotOrientation = xf.GetPivotOrientation();
    const Vec3d &pax = pivotOrientation.GetAxis();
    double pang = pivotOrientation.GetAngle();

    // This class doesn't use the same precision helper that everyone
    // else uses (see _OstreamHelperP) for some reason.

    // note:  we currently specify the same orientation for both scale and
    // rotation, but the format allows for different orientations.
    return out
        << "( "
        << "(" << s[0] << ", " << s[1] << ", " << s[2] << ", 0), "
        << "(" << pax[0] << ", " << pax[1] << ", " << pax[2] << ", "
        << pang << "), "
        << "(" << rax[0] << ", " << rax[1] << ", " << rax[2] << ", "
        << rang << "), "
   
        << "(" << c[0] << ", " << c[1] << ", " << c[2] << ", 0), "
        << "(" << t[0] << ", " << t[1] << ", " << t[2] << ", 0) "
        << ")";
}

PXR_NAMESPACE_CLOSE_SCOPE
