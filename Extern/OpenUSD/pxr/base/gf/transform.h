// Modified from the original OpenUSD file.
// Copyright 2024 Mihail Mladenov

//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef PXR_BASE_GF_TRANSFORM_H
#define PXR_BASE_GF_TRANSFORM_H

/// \file gf/transform.h
/// \ingroup group_gf_LinearAlgebra

#include "pxr/pxr.h"
#include "pxr/base/gf/rotation.h"
#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/api.h"

#include <iosfwd>

PXR_NAMESPACE_OPEN_SCOPE

class Matrix4d;

/// \class Transform
/// \ingroup group_gf_LinearAlgebra
///
/// Basic type: Compound linear transformation.
///
/// This class represents a linear transformation specified as a series of
/// individual components: a \em translation, a \em rotation, a \em scale, a
/// \em pivotPosition, and a \em pivotOrientation.  When applied to a point,
/// the point will be transformed as follows (in order):
///
/// \li Scaled by the \em scale with respect to \em pivotPosition and the
/// orientation specified by the \em pivotOrientation.
/// \li Rotated by the \em rotation about \em pivotPosition.
/// \li Translated by \em Translation
///
/// That is, the cumulative matrix that this represents looks like this.
///
/// \code
/// M = -P * -O * S * O * R * P * T
/// \endcode
///
/// where
/// \li \em T is the \em translation matrix
/// \li \em P is the matrix that translates by \em pivotPosition
/// \li \em R is the \em rotation matrix
/// \li \em O is the matrix that rotates to \em pivotOrientation
/// \li \em S is the \em scale matrix
///
class Transform {

  public:

    /// The default constructor sets the component values to the
    /// identity transformation.
    Transform() {
        SetIdentity();
    }

    /// This constructor initializes the transformation from all
    /// component values.  This is the constructor used by 2x code.
    Transform(const Vec3d &scale,
                const Rotation &pivotOrientation,
                const Rotation &rotation,
                const Vec3d &pivotPosition,
                const Vec3d &translation) {
        Set(scale, pivotOrientation, rotation, pivotPosition, translation);
    }

    /// This constructor initializes the transformation from all
    /// component values.  This is the constructor used by 3x code.
    Transform(const Vec3d &translation,
                const Rotation &rotation,
                const Vec3d &scale,
                const Vec3d &pivotPosition,
                const Rotation &pivotOrientation) {
        Set(translation, rotation, scale, pivotPosition, pivotOrientation);
    }

    /// This constructor initializes the transformation with a matrix.  See
    /// SetMatrix() for more information.
    Transform(const Matrix4d &m) {
        SetIdentity();
        SetMatrix(m);
    }

    /// Sets the transformation from all component values.
    /// This constructor orders its arguments the way that 2x expects.
    GF_API
    Transform &       Set(const Vec3d &scale,
                            const Rotation &pivotOrientation,
                            const Rotation &rotation,
                            const Vec3d &pivotPosition,
                            const Vec3d &translation);

    /// Sets the transformation from all component values.
    /// This constructor orders its arguments the way that 3x expects.
    Transform &       Set(const Vec3d &translation,
                            const Rotation &rotation,
                            const Vec3d &scale,
                            const Vec3d &pivotPosition,
                            const Rotation &pivotOrientation) {
        return Set(scale, pivotOrientation, rotation, 
                   pivotPosition, translation);
    }

    /// Sets the transform components to implement the transformation
    /// represented by matrix \p m , ignoring any projection. This tries to
    /// leave the current center unchanged.
    GF_API
    Transform &       SetMatrix(const Matrix4d &m);

    /// Sets the transformation to the identity transformation.
    GF_API
    Transform &       SetIdentity();

    /// Sets the scale component, leaving all others untouched.
    void                SetScale(const Vec3d &scale) {
        _scale = scale;
    }

    /// Sets the pivot orientation component, leaving all others untouched.
    void                SetPivotOrientation(const Rotation &pivotOrient) {
        _pivotOrientation = pivotOrient;
    }

    /// Sets the pivot orientation component, leaving all others untouched.
    void                SetScaleOrientation(const Rotation &pivotOrient) {
        SetPivotOrientation(pivotOrient);
    }

    /// Sets the rotation component, leaving all others untouched.
    void                SetRotation(const Rotation &rotation) {
        _rotation = rotation;
    }

    /// Sets the pivot position component, leaving all others untouched.
    void                SetPivotPosition(const Vec3d &pivPos) {
        _pivotPosition = pivPos;
    }

    /// Sets the pivot position component, leaving all others untouched.
    void                SetCenter(const Vec3d &pivPos) {
        SetPivotPosition(pivPos);
    }

    /// Sets the translation component, leaving all others untouched.
    void                SetTranslation(const Vec3d &translation) {
        _translation = translation;
    }

    /// Returns the scale component.
    const Vec3d &     GetScale() const {
        return _scale;
    }

    /// Returns the pivot orientation component.
    const Rotation &  GetPivotOrientation() const {
        return _pivotOrientation;
    }

    /// Returns the scale orientation component.
    const Rotation &  GetScaleOrientation() const {
        return GetPivotOrientation();
    }

    /// Returns the rotation component.
    const Rotation &  GetRotation() const {
        return _rotation;
    }

    /// Returns the pivot position component.
    const Vec3d &     GetPivotPosition() const {
        return _pivotPosition;
    }

    /// Returns the pivot position component.
    const Vec3d &     GetCenter() const {
        return GetPivotPosition();
    }

    /// Returns the translation component.
    const Vec3d &     GetTranslation() const {
        return _translation;
    }

    /// Returns a \c Matrix4d that implements the cumulative transformation.
    GF_API
    Matrix4d          GetMatrix() const;

    /// Component-wise transform equality test. All components must match
    /// exactly for transforms to be considered equal.
    GF_API
    bool                operator ==(const Transform &xf) const;

    /// Component-wise transform inequality test. All components must match
    /// exactly for transforms to be considered equal.
    bool                operator !=(const Transform &xf) const {
        return ! (*this == xf);
    }

    /// Post-multiplies transform \p xf into this transform.
    GF_API
    Transform &       operator *=(const Transform &xf);

    /// Returns the product of transforms \p xf1 and \p xf2.
    friend Transform  operator *(const Transform &xf1,
                                   const Transform &xf2) {
        Transform xf = xf1;
        return xf *= xf2;
    }

  private:
    /// translation
    Vec3d             _translation;
    /// rotation
    Rotation          _rotation;
    /// scale factors
    Vec3d             _scale;
    /// orientation used for scaling and rotation
    Rotation          _pivotOrientation;
    /// center of rotation and scaling
    Vec3d             _pivotPosition;
};

/// Output a Transform using the format 
/// [scale, scaleorientation, rotation, center, translation].
/// \ingroup group_gf_DebuggingOutput
GF_API std::ostream& operator<<(std::ostream&, const Transform&);

PXR_NAMESPACE_CLOSE_SCOPE

#endif // PXR_BASE_GF_TRANSFORM_H
