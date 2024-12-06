// Modified from the original OpenUSD file.
// Copyright 2024 Mihail Mladenov

//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef PXR_BASE_GF_ROTATION_H
#define PXR_BASE_GF_ROTATION_H

/// \file gf/rotation.h
/// \ingroup group_gf_LinearAlgebra

#include "pxr/pxr.h"
#include "pxr/base/gf/quaternion.h"
#include "pxr/base/gf/quatd.h"
#include "pxr/base/gf/matrix4d.h"
#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/gf/api.h"
#include "pxr/base/tf/hash.h"

#include <iosfwd>

PXR_NAMESPACE_OPEN_SCOPE

/// \class Rotation
/// \ingroup group_gf_LinearAlgebra
///
/// Basic type: 3-space rotation specification.
///
/// This class represents a rotation in 3-space. This stores an axis as a
/// normalized vector of 3 \c doubles and an angle in degrees (as a double).
/// Rotations follow the right-hand rule: a positive rotation about an axis
/// vector appears counter-clockwise when looking from the end of the vector
/// toward the origin.
///
class Rotation {

  public:

    /// The default constructor leaves the rotation undefined.
    Rotation() {
    }

    /// This constructor initializes the rotation to be \p angle
    /// degrees about \p axis.
    Rotation(const Vec3d &axis, double angle) {
        SetAxisAngle(axis, angle);
    }

    /// This constructor initializes the rotation from a quaternion.
    Rotation(const Quaternion &quaternion) {
        SetQuaternion(quaternion);
    }

    /// This constructor initializes the rotation from a quaternion.  Note that
    /// this constructor accepts Quatf and Quath since they implicitly
    /// convert to Quatd.
    Rotation(const Quatd &quat) { SetQuat(quat); }

    /// This constructor initializes the rotation to one that brings
    /// the \p rotateFrom vector to align with \p rotateTo. The passed
    /// vectors need not be unit length.
    GF_API
    Rotation(const Vec3d &rotateFrom, const Vec3d &rotateTo) {
        SetRotateInto(rotateFrom, rotateTo);
    }

    /// Sets the rotation to be \p angle degrees about \p axis.
    Rotation &        SetAxisAngle(const Vec3d &axis, double angle) {
        _axis = axis;
        _angle = angle;
        if (!IsClose(_axis * _axis, 1.0, 1e-10))
            _axis.Normalize();
        return *this;
    }

    /// Sets the rotation from a quaternion.  Note that this method accepts
    /// Quatf and Quath since they implicitly convert to Quatd.
    GF_API
    Rotation &        SetQuat(const Quatd &quat);

    /// Sets the rotation from a quaternion.
    Rotation &        SetQuaternion(const Quaternion &quat) {
        return SetQuat(Quatd(quat.GetReal(), quat.GetImaginary()));
    }

    /// Sets the rotation to one that brings the \p rotateFrom vector
    /// to align with \p rotateTo. The passed vectors need not be unit
    /// length.
    GF_API
    Rotation &        SetRotateInto(const Vec3d &rotateFrom,
                                      const Vec3d &rotateTo);

    /// Sets the rotation to an identity rotation.
    /// (This is chosen to be 0 degrees around the positive X axis.)
    Rotation &        SetIdentity() {
        _axis.Set(1.0, 0.0, 0.0);
        _angle = 0.0;
        return *this;
    }

    /// Returns the axis of rotation.
    const Vec3d &     GetAxis() const {
        return _axis;
    }

    /// Returns the rotation angle in degrees.
    double              GetAngle() const {
        return _angle;
    }

    /// Returns the rotation expressed as a quaternion.
    Quaternion        GetQuaternion() const {
        auto quat = GetQuat();
        return Quaternion(quat.GetReal(), quat.GetImaginary());
    }

    /// Returns the rotation expressed as a quaternion.
    GF_API
    Quatd             GetQuat() const;

    /// Returns the inverse of this rotation.
    Rotation          GetInverse() const {
        return Rotation(_axis, -_angle);
    }

    /// Decompose rotation about 3 orthogonal axes. 
    /// If the axes are not orthogonal, warnings will be spewed.
    GF_API
    Vec3d Decompose( const Vec3d &axis0,
                       const Vec3d &axis1,
                       const Vec3d &axis2 ) const;

    // Full-featured method to  Decompose a rotation matrix into Cardarian 
    // angles.
    // Axes have must be normalized. If useHint is specified
    // then the current values stored within thetaTw, thetaFB, thetaLR,
    // and thetaSw will be treated as hint and  used to help choose 
    // an equivalent rotation that is as close as possible to the hints.
    //
    // One can use this routine to generate any combination of the three 
    // angles by passing in nullptr for the angle that is to be omitted.
    // 
    // Passing in valid pointers for all four angles will decompose into
    // Tw, FB, and LR but allows Sw to be used for best matching of hint 
    // values.  It also allows an swShift value to be passed in as a 
    // Sw that is applied after the rotation matrix to get a best fit rotation
    // in four angles.
    //
    // Angles are in radians.
    //
    // Specify \p handedness as -1.0 or 1.0, same as for MultiRotate.
    //
    // NOTE:
    // Geppetto math function Originally brought over to extMover 
    // from //depot/main/tools/src/menv/lib/gpt/util.h [10/16/06]
    // And moved into Rotation[12/1/08].  Updated for any 
    // combination of three angles [12/1/11].
    //
    GF_API
    static void DecomposeRotation(const Matrix4d &rot,
                                  const Vec3d &TwAxis,
                                  const Vec3d &FBAxis,
                                  const Vec3d &LRAxis,
                                  double handedness,
                                  double *thetaTw,
                                  double *thetaFB,
                                  double *thetaLR,
                                  double *thetaSw = nullptr,
                                  bool   useHint=false,
                                  const double *swShift=nullptr);

    // This function projects the vectors \p v1 and \p v2 onto the plane 
    // normal to \p axis, and then returns the rotation about \p axis that 
    // brings \p v1 onto \p v2.
    GF_API
    static Rotation RotateOntoProjected(const Vec3d &v1,
                                          const Vec3d &v2,
                                          const Vec3d &axis);

    /// Replace the hint angles with the closest rotation of the given
    /// rotation to the hint.
    ///
    /// Each angle in the rotation will be within Pi of the corresponding
    /// hint angle and the sum of the differences with the hint will
    /// be minimized. If a given rotation value is null then that angle will
    /// be treated as 0.0 and ignored in the calculations.
    ///
    /// All angles are in radians. The rotation order is Tw/FB/LR/Sw.
    GF_API
    static void MatchClosestEulerRotation(
        double targetTw, double targetFB, double targetLR, double targetSw,
        double *thetaTw, double *thetaFB, double *thetaLR, double *thetaSw);

    /// Transforms row vector \p vec by the rotation, returning the result. 
    GF_API
    Vec3d TransformDir( const Vec3d &vec ) const;

    /// Hash.
    friend inline size_t hash_value(const Rotation &r) {
        return TfHash::Combine(r._axis, r._angle);
    }

    /// Component-wise rotation equality test. The axes and angles must match
    /// exactly for rotations to be considered equal. (To compare equality of
    /// the actual rotations, you can convert both to quaternions and test the
    /// results for equality.)
    bool        operator ==(const Rotation &r) const {
        return (_axis  == r._axis &&
            _angle == r._angle);
    }

    /// Component-wise rotation inequality test. The axes and angles must
    /// match exactly for rotations to be considered equal. (To compare
    /// equality of the actual rotations, you can convert both to quaternions
    /// and test the results for equality.)
    bool        operator !=(const Rotation &r) const {
        return ! (*this == r);
    }

    /// Post-multiplies rotation \p r into this rotation.
    GF_API
    Rotation &        operator *=(const Rotation &r);

    /// Scales rotation angle by multiplying by \p scale.
    Rotation &        operator *=(double scale) {
        _angle *= scale;
        return *this;
    }

    /// Scales rotation angle by dividing by \p scale.
    Rotation &    operator /=(double scale) {
        _angle /= scale;
        return *this;
    }

    /// Returns composite rotation of rotations \p r1 and \p r2.
    friend Rotation   operator *(const Rotation &r1,
                   const Rotation &r2) {
        Rotation r  = r1;
        return     r *= r2;
    }

    /// Returns a rotation equivalent to \p r with its angle multiplied
    /// by \p scale.
    friend Rotation   operator *(const Rotation &r, double scale) {
        Rotation rTmp  = r;
        return     rTmp *= scale;
    }

    /// Returns a rotation equivalent to \p r with its angle multiplied
    /// by \p scale.
    friend Rotation   operator *(double scale, const Rotation &r) {
        return (r * scale);
    }

    /// Returns a rotation equivalent to \p r with its angle divided
    /// by \p scale.
    friend Rotation   operator /(const Rotation &r, double scale) {
        Rotation rTmp  = r;
        return     rTmp /= scale;
    }

  private:
    /// Axis storage.
    /// This axis is normalized to unit length whenever it is set.
    Vec3d     _axis;
    /// Angle storage (represented in degrees).
    double      _angle;
};

/// Output a Rotation using the format [(x y z) a].
/// \ingroup group_gf_DebuggingOutput
GF_API std::ostream& operator<<(std::ostream&, const Rotation&);

PXR_NAMESPACE_CLOSE_SCOPE

#endif // PXR_BASE_GF_ROTATION_H
