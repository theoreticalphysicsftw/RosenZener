// Modified from the original OpenUSD file.
// Copyright 2024 Mihail Mladenov

//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//

#include "pxr/pxr.h"
#include "pxr/base/gf/bbox3d.h"
#include "pxr/base/gf/math.h"
#include "pxr/base/gf/ostreamHelpers.h"

#include "pxr/base/tf/type.h"

PXR_NAMESPACE_OPEN_SCOPE

TF_REGISTRY_FUNCTION(TfType)
{
    TfType::Define<BBox3d>();
}

void
BBox3d::_SetMatrices(const Matrix4d &matrix)
{
    const double PRECISION_LIMIT = 1.0e-13;
    double det;

    _isDegenerate = false;
    _matrix = matrix; 
    _inverse = matrix.GetInverse(&det, PRECISION_LIMIT);

    // Check for degenerate matrix:
    if (Abs(det) <= PRECISION_LIMIT) {
        _isDegenerate = true;
        _inverse.SetIdentity();
    }
}

double
BBox3d::GetVolume() const
{
    if (_box.IsEmpty())
        return 0.0;

    // The volume of a transformed box is just its untransformed
    // volume times the determinant of the upper-left 3x3 of the xform
    // matrix. Pretty cool, indeed.
    Vec3d size = _box.GetSize();
    return fabs(_matrix.GetDeterminant3() * size[0] * size[1] * size[2]);
}

Range3d
BBox3d::ComputeAlignedRange() const
{
    if (_box.IsEmpty())
	return _box;
    
    // Method: James Arvo, Graphics Gems I, pp 548-550

    // Translate the origin and use the result as the min and max.
    Vec3d trans(_matrix[3][0], _matrix[3][1], _matrix[3][2]);
    Vec3d alignedMin = trans;
    Vec3d alignedMax = trans;

    const Vec3d &min = _box.GetMin();
    const Vec3d &max = _box.GetMax();

    for (int j = 0; j < 3; j++) {
        for (int i = 0; i < 3; i++) {
            double a = min[i] * _matrix[i][j];
            double b = max[i] * _matrix[i][j];
            if (a < b) {
                alignedMin[j] += a;
                alignedMax[j] += b;
            }
            else {
                alignedMin[j] += b;
                alignedMax[j] += a;
            }
        }
    }

    return Range3d(alignedMin, alignedMax);
}

BBox3d
BBox3d::Combine(const BBox3d &b1, const BBox3d &b2)
{
    BBox3d result;

    // If either box is empty, use the other as is
    if (b1.GetRange().IsEmpty())
        result = b2;
    else if (b2.GetRange().IsEmpty())
        result = b1;

    // If both boxes are degenerate, combine their projected
    // boxes. Otherwise, transform the degenerate box into the space
    // of the other box and combine the results in that space.
    else if (b1._isDegenerate) {
        if (b2._isDegenerate)
            result = BBox3d(Range3d::GetUnion(b1.ComputeAlignedRange(),
                                                  b2.ComputeAlignedRange()));
        else
            result = _CombineInOrder(b2, b1);
    }
    else if (b2._isDegenerate)
        result = _CombineInOrder(b1, b2);

    // Non-degenerate case: Neither box is empty and they are in
    // different spaces. To get the best results, we'll perform the
    // merge of the two boxes in each of the two spaces. Whichever
    // merge ends up being smaller (by volume) is the one we'll use.
    // Note that we don't use ComputeAlignedRange() as part of the test.
    // This is because projecting almost always adds a little extra
    // space and it gives an unfair advantage to the box that is more
    // closely aligned to the coordinate axes.
    else {
        BBox3d result1 = _CombineInOrder(b1, b2);
        BBox3d result2 = _CombineInOrder(b2, b1);

        // Test within a tolerance (based on volume) to make this
        // reasonably deterministic.
        double v1 = result1.GetVolume();
        double v2 = result2.GetVolume();
        double tolerance = Max(1e-10, 1e-6 * Abs(Max(v1, v2)));

        result = (Abs(v1 - v2) <= tolerance ? result1 :
                  (v1 < v2 ? result1 : result2));
    }

    // The _hasZeroAreaPrimitives is set to true if either of the
    // input boxes has it set to true.
    result.SetHasZeroAreaPrimitives(b1.HasZeroAreaPrimitives() ||
                                    b2.HasZeroAreaPrimitives());

    return result;
}

BBox3d
BBox3d::_CombineInOrder(const BBox3d &b1, const BBox3d &b2)
{
    // Transform b2 into b1's space to get b2t
    BBox3d b2t;
    b2t._box = b2._box;
    b2t._matrix  = b2._matrix * b1._inverse;
    b2t._inverse = b1._matrix * b2._inverse;

    // Compute the projection of this box into b1's space.
    Range3d proj = b2t.ComputeAlignedRange();

    // Extend b1 by this box to get the result.
    BBox3d result = b1;
    result._box.UnionWith(proj);
    return result;
}

Vec3d
BBox3d::ComputeCentroid() const
{
    Vec3d a = GetRange().GetMax();
    Vec3d b = GetRange().GetMin(); 

    return GetMatrix().Transform( .5 * (a + b) );
}

std::ostream &
operator<<(std::ostream& out, const BBox3d& b)
{
    return out
        << "[("
        << _OstreamHelperP(b.GetRange()) << ") (" 
        << _OstreamHelperP(b.GetMatrix()) << ") "
        << (b.HasZeroAreaPrimitives() ? "true" : "false")
        << ']';
}

PXR_NAMESPACE_CLOSE_SCOPE
