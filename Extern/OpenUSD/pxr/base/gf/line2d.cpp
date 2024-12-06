// Modified from the original OpenUSD file.
// Copyright 2024 Mihail Mladenov

//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//

#include "pxr/pxr.h"
#include "pxr/base/gf/line2d.h"
#include "pxr/base/gf/math.h"

#include "pxr/base/tf/type.h"

PXR_NAMESPACE_OPEN_SCOPE

// CODE_COVERAGE_OFF_GCOV_BUG
TF_REGISTRY_FUNCTION(TfType) {
    TfType::Define<Line2d>();
}
// CODE_COVERAGE_ON_GCOV_BUG

Vec2d
Line2d::FindClosestPoint(const Vec2d &point, double *t) const
{
    // Compute the vector from the start point to the given point.
    Vec2d v = point - _p0;

    // Find the length of the projection of this vector onto the line.
    double lt = Dot(v, _dir);
    
    if (t)
        *t = lt;

    return GetPoint( lt );
}

bool
FindClosestPoints( const Line2d &l1, const Line2d &l2,
                     Vec2d *closest1, Vec2d *closest2,
                     double *t1, double *t2 )
{
    // Define terms:
    //   p1 = line 1's position
    //   d1 = line 1's direction
    //   p2 = line 2's position
    //   d2 = line 2's direction
    const Vec2d &p1 = l1._p0; 
    const Vec2d &d1 = l1._dir;
    const Vec2d &p2 = l2._p0;
    const Vec2d &d2 = l2._dir;

    // We want to find points closest1 and closest2 on each line.
    // Their parametric definitions are:
    //   closest1 = p1 + t1 * d1
    //   closest2 = p2 + t2 * d2
    //
    // We know that the line connecting closest1 and closest2 is
    // perpendicular to both the ray and the line segment. So:
    //   d1 . (closest2 - closest1) = 0
    //   d2 . (closest2 - closest1) = 0
    //
    // Substituting gives us:
    //   d1 . [ (p2 + t2 * d2) - (p1 + t1 * d1) ] = 0
    //   d2 . [ (p2 + t2 * d2) - (p1 + t1 * d1) ] = 0
    //
    // Rearranging terms gives us:
    //   t2 * (d1.d2) - t1 * (d1.d1) = d1.p1 - d1.p2
    //   t2 * (d2.d2) - t1 * (d2.d1) = d2.p1 - d2.p2
    //
    // Substitute to simplify:
    //   a = d1.d2
    //   b = d1.d1
    //   c = d1.p1 - d1.p2
    //   d = d2.d2
    //   e = d2.d1 (== a, if you're paying attention)
    //   f = d2.p1 - d2.p2
    double a = Dot(d1, d2);
    double b = Dot(d1, d1);
    double c = Dot(d1, p1) - Dot(d1, p2);
    double d = Dot(d2, d2);
    double e = a;
    double f = Dot(d2, p1) - Dot(d2, p2);

    // And we end up with:
    //  t2 * a - t1 * b = c
    //  t2 * d - t1 * e = f
    //
    // Solve for t1 and t2:
    //  t1 = (c * d - a * f) / (a * e - b * d)
    //  t2 = (c * e - b * f) / (a * e - b * d)
    //
    // Note the identical denominators...
    double denom = a * e - b * d;

    // Denominator == 0 means the lines are parallel; no intersection.
    if ( IsClose( denom, 0, 1e-6 ) )
        return false;

    double lt1 = (c * d - a * f) / denom;
    double lt2 = (c * e - b * f) / denom;

    if ( closest1 )
        *closest1 = l1.GetPoint( lt1 );

    if ( closest2 )
        *closest2 = l2.GetPoint( lt2 );

    if ( t1 )
        *t1 = lt1;

    if ( t2 )
        *t2 = lt2;
    
    return true;
}

PXR_NAMESPACE_CLOSE_SCOPE
