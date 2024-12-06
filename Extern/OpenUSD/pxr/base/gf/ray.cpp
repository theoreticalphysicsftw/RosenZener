//
// Modified from the original ->  prefix removed.

// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//

#include "pxr/pxr.h"
#include "pxr/base/gf/ray.h"
#include "pxr/base/gf/bbox3d.h"
#include "pxr/base/gf/line.h"
#include "pxr/base/gf/lineSeg.h"
#include "pxr/base/gf/math.h"
#include "pxr/base/gf/ostreamHelpers.h"
#include "pxr/base/gf/plane.h"
#include "pxr/base/gf/range3d.h"
#include "pxr/base/gf/rotation.h"
#include "pxr/base/gf/vec2d.h"
#include "pxr/base/tf/type.h"

#include <ostream>

PXR_NAMESPACE_OPEN_SCOPE

// Tolerance for IsClose
static const double tolerance = 1e-6;

// CODE_COVERAGE_OFF_GCOV_BUG - gcov is finicky
TF_REGISTRY_FUNCTION(TfType) {
    TfType::Define<Ray>();
}
// CODE_COVERAGE_ON_GCOV_BUG

// Menv 2x uses SetDirection, but it looks like it is only in
// sgu/rayIntersectAction.cpp so I'm just going to change the name
// and let the compiler complain about any problems.
void
Ray::SetPointAndDirection(const Vec3d &startPoint,
                            const Vec3d &direction) {
    _startPoint = startPoint;
    _direction  = direction;
}

void
Ray::SetEnds(const Vec3d &startPoint, const Vec3d &endPoint)
{
    _startPoint = startPoint;
    _direction  = endPoint - startPoint;
}

Ray &
Ray::Transform(const Matrix4d &matrix)
{
    _startPoint = matrix.Transform(_startPoint);
    _direction = matrix.TransformDir(_direction);
    
    return *this;
}

Vec3d
Ray::FindClosestPoint(const Vec3d &point, double *rayDistance) const
{
    Line l;
    double len = l.Set(_startPoint, _direction);
    double lrd;
    (void) l.FindClosestPoint(point, &lrd);

    if (lrd < 0.0) lrd = 0.0;

    if (rayDistance)
	*rayDistance = lrd / len;

    return l.GetPoint(lrd);
}

bool
FindClosestPoints(const Ray &ray, const Line &line,
		    Vec3d *rayPoint,
		    Vec3d *linePoint,
		    double *rayDistance,
		    double *lineDistance)
{
    Line l;
    double len = l.Set(ray._startPoint, ray._direction);

    Vec3d rp, lp;
    double rd,ld;

    if (!FindClosestPoints(l, line, &rp, &lp, &rd, &ld))
	return false;

    if (rd < 0.0) rd = 0.0;

    if (rayPoint)
	*rayPoint = l.GetPoint(rd);

    if (linePoint)
	*linePoint = lp;

    if (rayDistance)
	*rayDistance = rd / len;

    if (lineDistance)
	*lineDistance = ld;

    return true;
}

bool 
FindClosestPoints(const Ray &ray, const LineSeg &seg,
		    Vec3d *rayPoint,
		    Vec3d *segPoint,
		    double *rayDistance,
		    double *segDistance)
{
    Line l;
    double len = l.Set(ray._startPoint, ray._direction);

    Vec3d rp, sp;
    double rd,sd;

    if (!FindClosestPoints(l, seg, &rp, &sp, &rd, &sd))
	return false;

    if (rd < 0.0) rd = 0.0;

    if (rayPoint)
	*rayPoint = l.GetPoint(rd);

    if (segPoint)
	*segPoint = sp;

    if (rayDistance)
	*rayDistance = rd / len;

    if (segDistance)
	*segDistance = sd;

    return true;
}

bool
Ray::Intersect(const Vec3d &p0, const Vec3d &p1, const Vec3d &p2,
                 double *distance,
                 Vec3d *barycentricCoords, bool *frontFacing,
                 double maxDist) const
{
    // Intersect the ray with the plane containing the three points.
    Plane plane(p0, p1, p2);
    double intersectionDist;
    if (! Intersect(plane, &intersectionDist, frontFacing))
        return false;

    if (intersectionDist > maxDist)
        return false;

    // Find the largest component of the plane normal. The other two
    // dimensions are the axes of the aligned plane we will use to
    // project the triangle.
    double xAbs = Abs(plane.GetNormal()[0]);
    double yAbs = Abs(plane.GetNormal()[1]);
    double zAbs = Abs(plane.GetNormal()[2]);
    unsigned int axis0, axis1;
    if (xAbs > yAbs && xAbs > zAbs) {
	axis0 = 1;
	axis1 = 2;
    }
    else if (yAbs > zAbs) {
	axis0 = 2;
	axis1 = 0;
    }
    else {
	axis0 = 0;
	axis1 = 1;
    }

    // Determine if the projected intersection (of the ray's line and
    // the triangle's plane) lies within the projected triangle.
    // Since we deal with only 2 components, we can avoid the third
    // computation.
    double  inter0 = _startPoint[axis0] + intersectionDist * _direction[axis0];
    double  inter1 = _startPoint[axis1] + intersectionDist * _direction[axis1];
    Vec2d d0(inter0    - p0[axis0], inter1     - p0[axis1]);
    Vec2d d1(p1[axis0] - p0[axis0], p1[axis1] - p0[axis1]);
    Vec2d d2(p2[axis0] - p0[axis0], p2[axis1] - p0[axis1]);

    // XXX This code can miss some intersections on very tiny tris.
    double alpha;
    double beta = ((d0[1] * d1[0] - d0[0] * d1[1]) /
                   (d2[1] * d1[0] - d2[0] * d1[1]));
    // clamp beta to 0 if it is only very slightly less than 0
    if (beta < 0 && beta > -GF_MIN_VECTOR_LENGTH) {
	// CODE_COVERAGE_OFF_NO_REPORT - architecture dependent numerics
        beta = 0;
	// CODE_COVERAGE_ON_NO_REPORT
    }
    if (beta < 0.0 || beta > 1.0) {
        return false;
    }
    alpha = -1.0;
    if (d1[1] < -GF_MIN_VECTOR_LENGTH || d1[1] > GF_MIN_VECTOR_LENGTH) 
	alpha = (d0[1] - beta * d2[1]) / d1[1];
    else
        alpha = (d0[0] - beta * d2[0]) / d1[0];

    // clamp alpha to 0 if it is only very slightly less than 0
    if (alpha < 0 && alpha > -GF_MIN_VECTOR_LENGTH) {
	// CODE_COVERAGE_OFF_NO_REPORT - architecture dependent numerics
        alpha = 0;
	// CODE_COVERAGE_ON_NO_REPORT
    }

    // clamp gamma to 0 if it is only very slightly less than 0
    float gamma = 1.0 - (alpha + beta);
    if (gamma < 0 && gamma > -GF_MIN_VECTOR_LENGTH) {
	// CODE_COVERAGE_OFF_NO_REPORT - architecture dependent numerics
        gamma = 0;
	// CODE_COVERAGE_ON_NO_REPORT
    }
    if (alpha < 0.0 || gamma < 0.0)
        return false;

    if (distance)
        *distance = intersectionDist;
    if (barycentricCoords)
        barycentricCoords->Set(gamma, alpha, beta);

    return true;
}

bool
Ray::Intersect(const Plane &plane,
		 double *distance, bool *frontFacing) const
{
    // The dot product of the ray direction and the plane normal
    // indicates the angle between them. Reject glancing
    // intersections. Note: this also rejects ill-formed planes with
    // zero normals.
    double d = Dot(_direction, plane.GetNormal());
    if (d < GF_MIN_VECTOR_LENGTH && d > -GF_MIN_VECTOR_LENGTH)
        return false;

    // Get a point on the plane.
    Vec3d planePoint = plane.GetDistanceFromOrigin() * plane.GetNormal();

    // Compute the parametric distance t to the plane. Reject
    // intersections outside the ray bounds.
    double t = Dot(planePoint - _startPoint, plane.GetNormal()) / d;
    if (t < 0.0)
	return false;

    if (distance)
	*distance = t;
    if (frontFacing)
	*frontFacing = (d < 0.0);

    return true;
}

bool
Ray::Intersect(const Range3d &box,
		 double *enterDistance, double *exitDistance) const
{
    if (box.IsEmpty())
	return false;

    // Compute the intersection distance of all 6 planes of the
    // box. Save the largest near-plane intersection and the smallest
    // far-plane intersection.
    double maxNearest = -DBL_MAX, minFarthest = DBL_MAX;
    for (size_t i = 0; i < 3; i++) {

        // Skip dimensions almost parallel to the ray.
        double d = GetDirection()[i];
        if (Abs(d) < GF_MIN_VECTOR_LENGTH) {
            // ray is parallel to this set of planes.
            // If origin is not between them, no intersection.
            if (GetStartPoint()[i] < box.GetMin()[i] ||
                GetStartPoint()[i] > box.GetMax()[i]) {
                return false;
            } else {
                continue;
            }
        }

        d = 1.0 / d;
        double t1 = d * (box.GetMin()[i] - GetStartPoint()[i]);
        double t2 = d * (box.GetMax()[i] - GetStartPoint()[i]);

        // Make sure t1 is the nearer one
        if (t1 > t2) {
            double tmp = t1;
            t1 = t2;
            t2 = tmp;
        }

        // Update the min and max
        if (t1 > maxNearest)
            maxNearest = t1;
        if (t2 < minFarthest)
            minFarthest = t2;
    }

    // If the largest near-plane intersection is after the smallest
    // far-plane intersection, the ray's line misses the box. Also
    // check if both intersections are completely outside the near/far
    // bounds.
    if (maxNearest  >  minFarthest ||
        minFarthest < 0.0)
        return false;
        
    if (enterDistance)
        *enterDistance = maxNearest;
    if (exitDistance)
        *exitDistance = minFarthest;
    return true;
}

bool
Ray::Intersect(const BBox3d& box,
                 double* enterDistance, double* exitDistance ) const
{
    // Transform the ray to the local space of the bbox.
    Ray localRay(*this);
    localRay.Transform(box.GetInverseMatrix());

    // We take advantage of the fact that the time of intersection is
    // invariant before/after transformation to just return the results of
    // the intersection in local space.
    return localRay.Intersect(box.GetRange(), enterDistance, exitDistance);
}

bool
Ray::Intersect(const Vec3d& center, double radius, 
                 double* enterDistance, double* exitDistance ) const
{

    Vec3d p1 = _startPoint;
    Vec3d p2 = p1 + _direction;

    // Intersection pt: p = p1 + u (p2 -p1)
    // we are solving for u.
    // where p1 = [x1 y1 z1],  p2 = [x2 y2 z2]
    //
    double A, B, C;
    double x1, x2, x3, y1, y2, y3, z1, z2, z3;
    x1 = p1[0];     y1 = p1[1];     z1 = p1[2];
    x2 = p2[0];     y2 = p2[1];     z2 = p2[2];
    x3 = center[0]; y3 = center[1]; z3 = center[2];

    A = (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) + (z2-z1)*(z2-z1);
    B = 2 * ((x2-x1)*(x1-x3) + (y2-y1)*(y1-y3) + (z2-z1)*(z1-z3));
    C = x3*x3 + y3*y3 + z3*z3 + x1*x1 + y1*y1 + z1*z1
        - 2*(x3*x1 +y3*y1 +z3*z1) - radius*radius;

    return _SolveQuadratic(A, B, C, enterDistance, exitDistance);
}

bool
Ray::Intersect(const Vec3d &origin,
                 const Vec3d &axis,
                 const double radius,
                 double *enterDistance,
                 double *exitDistance) const
{
    Vec3d unitAxis = axis.GetNormalized();
    
    Vec3d delta = _startPoint - origin;
    Vec3d u = _direction - Dot(_direction, unitAxis) * unitAxis;
    Vec3d v = delta - Dot(delta, unitAxis) * unitAxis;
    
    // Quadratic equation for implicit infinite cylinder
    double a = Dot(u, u);
    double b = 2.0 * Dot(u, v);
    double c = Dot(v, v) - Sqr(radius);
    
    return _SolveQuadratic(a, b, c, enterDistance, exitDistance);
}

bool
Ray::Intersect(const Vec3d &origin,
                 const Vec3d &axis,
                 const double radius, 
                 const double height,
                 double *enterDistance,
                 double *exitDistance) const
{ 
    Vec3d unitAxis = axis.GetNormalized();
    
    // Apex of cone
    Vec3d apex = origin + height * unitAxis;
    
    Vec3d delta = _startPoint - apex;
    Vec3d u =_direction - Dot(_direction, unitAxis) * unitAxis;
    Vec3d v = delta - Dot(delta, unitAxis) * unitAxis;
    
    double p = Dot(_direction, unitAxis);
    double q = Dot(delta, unitAxis);
    
    double cos2 = Sqr(height) / (Sqr(height) + Sqr(radius));
    double sin2 = 1 - cos2;
    
    double a = cos2 * Dot(u, u) - sin2 * Sqr(p);
    double b = 2.0 * (cos2 * Dot(u, v) - sin2 * p * q);
    double c = cos2 * Dot(v, v) - sin2 * Sqr(q);
    
    if (!_SolveQuadratic(a, b, c, enterDistance, exitDistance)) {
        return false;
    }
    
    // Eliminate any solutions on the double cone
    bool enterValid = Dot(unitAxis, GetPoint(*enterDistance) - apex) <= 0.0;
    bool exitValid = Dot(unitAxis, GetPoint(*exitDistance) - apex) <= 0.0;
    
    if ((!enterValid) && (!exitValid)) {
        
        // Solutions lie only on double cone
        return false;
    }
    
    if (!enterValid) {
        *enterDistance = *exitDistance;
    }
    else if (!exitValid) {
        *exitDistance = *enterDistance;
    }
        
    return true;
}

bool
Ray::_SolveQuadratic(const double a, 
                       const double b,
                       const double c,
                       double *enterDistance, 
                       double *exitDistance) const
{
    if (IsClose(a, 0.0, tolerance)) {
        if (IsClose(b, 0.0, tolerance)) {
            
            // Degenerate solution
            return false;
        }
        
        double t = -c / b;
        
        if (t < 0.0) {
            return false;
        }
        
        if (enterDistance) {
            *enterDistance = t;
        }
        
        if (exitDistance) {
            *exitDistance = t;
        }
        
        return true;
    }
    
    // Discriminant
    double disc = Sqr(b) - 4.0 * a * c;
    
    if (IsClose(disc, 0.0, tolerance)) {
        
        // Tangent
        double t = -b / (2.0 * a);
        
        if (t < 0.0) {
            return false;
        }
        
        if (enterDistance) {
            *enterDistance = t;
        }
        
        if (exitDistance) {
            *exitDistance = t;
        }
        
        return true;
    }
    
    if (disc < 0.0) {

        // No intersection
        return false;
    }

    // Two intersection points
    double q = -0.5 * (b + copysign(1.0, b) * Sqrt(disc));
    double t0 = q / a;
    double t1 = c / q;

    if (t0 > t1) {
        std::swap(t0, t1);
    }
    
    if (t1 >= 0) {

        if (enterDistance) {
            *enterDistance = t0;
        }
            
        if (exitDistance) {
            *exitDistance  = t1;
        }

        return true;
    }
    
    return false;
}

std::ostream &
operator<<(std::ostream& out, const Ray& r)
{
    return out << '[' << _OstreamHelperP(r.GetStartPoint()) << " >> " 
               << _OstreamHelperP(r.GetDirection()) << ']';
}

PXR_NAMESPACE_CLOSE_SCOPE
