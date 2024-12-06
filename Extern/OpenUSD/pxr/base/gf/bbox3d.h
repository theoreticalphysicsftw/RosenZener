// Modified from the original OpenUSD file.
// Copyright 2024 Mihail Mladenov

// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef PXR_BASE_GF_BBOX3D_H
#define PXR_BASE_GF_BBOX3D_H

/// \file gf/bbox3d.h
/// \ingroup group_gf_BasicGeometry

#include "pxr/pxr.h"
#include "pxr/base/gf/matrix4d.h"
#include "pxr/base/gf/range3d.h"
#include "pxr/base/gf/api.h"

#include <iosfwd>

PXR_NAMESPACE_OPEN_SCOPE

/// \class BBox3d
/// \ingroup group_gf_BasicGeometry
/// Basic type: arbitrarily oriented 3D bounding box.
///
/// This class represents a three-dimensional bounding box as an
/// axis-aligned box (\c Range3d) and a matrix (\c Matrix4d) to
/// transform it into the correct space.
///
/// A \c BBox3d is more useful than using just \c Range3d instances
/// (which are always axis-aligned) for these reasons:
///
/// \li When an axis-aligned bounding box is transformed several times,
/// each transformation can result in inordinate growth of the bounding
/// box. By storing the transformation separately, it can be applied once
/// at the end, resulting in a much better fit.  For example, if the
/// bounding box at the leaf of a scene graph is transformed through
/// several levels of the graph hierarchy to the coordinate space at the
/// root, a \c BBox3d is generally much smaller than the \c Range3d
/// computed by transforming the box at each level.
///
/// \li When two or more such bounding boxes are combined, having the
/// transformations stored separately means that there is a better
/// opportunity to choose a better coordinate space in which to combine
/// the boxes.
///
/// \anchor bbox3d_zeroAreaFlag
/// <b> The Zero-area Primitives Flag </b>
///
/// When bounding boxes are used in intersection test culling, it is
/// sometimes useful to extend them a little bit to allow
/// lower-dimensional objects with zero area, such as lines and points,
/// to be intersected. For example, consider a cube constructed of line
/// segments. The bounding box for this shape fits the cube exactly. If
/// an application wants to allow a near-miss of the silhouette edges of
/// the cube to be considered an intersection, it has to loosen the bbox
/// culling test a little bit.
///
/// To distinguish when this loosening is necessary, each \c BBox3d
/// instance maintains a flag indicating whether any zero-area primitives
/// are contained within it. The application is responsible for setting
/// this flag correctly by calling \c SetHasZeroAreaPrimitives(). The
/// flag can be accessed during intersection tests by calling \c
/// HasZeroAreaPrimitives(). This flag is set by default in all
/// constructors to \c false.
///
class BBox3d {

  public:

    /// The default constructor leaves the box empty, the transformation
    /// matrix identity, and the \ref bbox3d_zeroAreaFlag "zero-area
    /// primitives flag" \c false.
    BBox3d() {
        _matrix.SetIdentity();
        _inverse.SetIdentity();
        _isDegenerate = false;
        _hasZeroAreaPrimitives = false;
    }

    /// This constructor takes a box and sets the matrix to identity.
    BBox3d(const Range3d &box) :
        _box(box) {
        _matrix.SetIdentity();
        _inverse.SetIdentity();
        _isDegenerate = false;
        _hasZeroAreaPrimitives = false;
    }

    /// This constructor takes a box and a transformation matrix.
    BBox3d(const Range3d &box, const Matrix4d &matrix) {
        Set(box, matrix);
        _hasZeroAreaPrimitives = false;
    }

    /// Sets the axis-aligned box and transformation matrix.
    void                Set(const Range3d &box, const Matrix4d &matrix) {
        _box = box;
        _SetMatrices(matrix);
    }

    /// Sets the transformation matrix only.  The axis-aligned box is not
    /// modified.
    void                SetMatrix(const Matrix4d& matrix) {
        _SetMatrices(matrix);
    }

    /// Sets the range of the axis-aligned box only.  The transformation
    /// matrix is not modified.
    void                SetRange(const Range3d& box) {
        _box = box;
    }

    /// Returns the range of the axis-aligned untransformed box.
    const Range3d &   GetRange() const {
        return _box;
    }

    /// Returns the range of the axis-aligned untransformed box.
    /// This synonym of \c GetRange exists for compatibility purposes.
    const Range3d &	GetBox() const {
        return GetRange();
    }

    /// Returns the transformation matrix.
    const Matrix4d &  GetMatrix() const {
        return _matrix;
    }

    /// Returns the inverse of the transformation matrix. This will be the
    /// identity matrix if the transformation matrix is not invertible.
    const Matrix4d &  GetInverseMatrix() const {
        return _inverse;
    }

    /// Sets the \ref bbox3d_zeroAreaFlag "zero-area primitives flag" to the
    /// given value.
    void                SetHasZeroAreaPrimitives(bool hasThem) {
        _hasZeroAreaPrimitives = hasThem;
    }

    /// Returns the current state of the \ref bbox3d_zeroAreaFlag "zero-area
    /// primitives flag".
    bool                HasZeroAreaPrimitives() const {
        return _hasZeroAreaPrimitives;
    }

    /// Returns the volume of the box (0 for an empty box).
    GF_API
    double              GetVolume() const;

    /// Transforms the bounding box by the given matrix, which is assumed to
    /// be a global transformation to apply to the box. Therefore, this just
    /// post-multiplies the box's matrix by \p matrix.
    void                Transform(const Matrix4d &matrix) {
        _SetMatrices(_matrix * matrix);
    }

    /// Returns the axis-aligned range (as a \c Range3d) that results from
    /// applying the transformation matrix to the wxis-aligned box and
    /// aligning the result.
    GF_API
    Range3d           ComputeAlignedRange() const;

    /// Returns the axis-aligned range (as a \c Range3d) that results from
    /// applying the transformation matrix to the axis-aligned box and
    /// aligning the result. This synonym for \c ComputeAlignedRange exists
    /// for compatibility purposes.
    Range3d           ComputeAlignedBox() const {
        return ComputeAlignedRange();
    }

    /// Combines two bboxes, returning a new bbox that contains both.  This
    /// uses the coordinate space of one of the two original boxes as the
    /// space of the result; it uses the one that produces whe smaller of the
    /// two resulting boxes.
    GF_API
    static BBox3d     Combine(const BBox3d &b1, const BBox3d &b2);

    /// Returns the centroid of the bounding box.
    /// The centroid is computed as the transformed centroid of the range.
    GF_API
    Vec3d             ComputeCentroid() const;

    /// Hash.
    friend inline size_t hash_value(const BBox3d &b) {
        return TfHash::Combine(
            b._box,
            b._matrix
        );
    }
    
    /// Component-wise equality test. The axis-aligned boxes and
    /// transformation matrices match exactly for bboxes to be considered
    /// equal. (To compare equality of the actual boxes, you can compute both
    /// aligned boxes and test the results for equality.)
    bool                operator ==(const BBox3d &b) const {
        return (_box    == b._box &&
                _matrix == b._matrix);
    }

    /// Component-wise inequality test. The axis-aligned boxes and
    /// transformation matrices match exactly for bboxes to be considered
    /// equal. (To compare equality of the actual boxes, you can compute both
    /// aligned boxes and test the results for equality.)
    bool                operator !=(const BBox3d &that) const {
        return !(*this == that);
    }

  private:
    /// The axis-aligned box.
    Range3d           _box;
    /// Transformation matrix.
    Matrix4d          _matrix;
    /// Inverse of the transformation matrix.
    Matrix4d          _inverse;
    /// Flag indicating whether the matrix is degenerate.
    bool                _isDegenerate;
    /// Flag indicating whether the bbox contains zero-area primitives.
    bool                _hasZeroAreaPrimitives;

    /// Sets the transformation matrix and the inverse, checking for
    /// degeneracies.
    GF_API
    void                _SetMatrices(const Matrix4d &matrix);

    /// This is used by \c Combine() when it is determined which coordinate
    /// space to use to combine two boxes: \p b2 is transformed into the space
    /// of \p b1 and the results are merged in that space.
    static BBox3d     _CombineInOrder(const BBox3d &b1, const BBox3d &b2);
};

/// Output a BBox3d using the format [(range) matrix zeroArea]
///
/// The zeroArea flag is true or false and indicates whether the
/// bbox has zero area primitives in it.
///
/// \ingroup group_gf_DebuggingOutput
GF_API std::ostream& operator<<(std::ostream&, const BBox3d&);

PXR_NAMESPACE_CLOSE_SCOPE

#endif // PXR_BASE_GF_BBOX3D_H
