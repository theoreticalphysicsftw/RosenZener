// Modified from the original OpenUSD file.
// Copyright 2024 Mihail Mladenov

//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
////////////////////////////////////////////////////////////////////////
// This file is generated by a script.  Do not edit directly.  Edit the
// vec.template.cpp file to make changes.

#include "pxr/base/gf/vec3f.h"

#include "pxr/pxr.h"
#include "pxr/base/gf/math.h"
#include "pxr/base/gf/ostreamHelpers.h"
#include "pxr/base/tf/type.h"

// Include headers for other vec types to support wrapping conversions and
// operators.
#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/vec3h.h"
#include "pxr/base/gf/vec3i.h"

#include <vector>
#include <ostream>

PXR_NAMESPACE_OPEN_SCOPE

TF_REGISTRY_FUNCTION(TfType) {
    TfType::Define<Vec3f>();
}

std::ostream& 
operator<<(std::ostream &out, Vec3f const &v)
{
    return out << '(' 
        << _OstreamHelperP(v[0]) << ", " 
        << _OstreamHelperP(v[1]) << ", " 
        << _OstreamHelperP(v[2]) << ')';
}


bool
Vec3f::operator==(Vec3d const &other) const
{
    return _data[0] == other[0] &&
           _data[1] == other[1] &&
           _data[2] == other[2];
}
bool
Vec3f::operator==(Vec3h const &other) const
{
    return _data[0] == other[0] &&
           _data[1] == other[1] &&
           _data[2] == other[2];
}
bool
Vec3f::operator==(Vec3i const &other) const
{
    return _data[0] == other[0] &&
           _data[1] == other[1] &&
           _data[2] == other[2];
}


bool
Vec3f::OrthogonalizeBasis(
    Vec3f *tx, Vec3f *ty, Vec3f *tz,
    const bool normalize, double eps)
{
    return OrthogonalizeBasis(tx, ty, tz, normalize, eps);
}

void
Vec3f::BuildOrthonormalFrame(
    Vec3f *v1, Vec3f *v2, float eps) const
{
    return ::BuildOrthonormalFrame(*this, v1, v2, eps);
}

/*
 * Given 3 basis vectors *tx, *ty, *tz, orthogonalize and optionally normalize
 * them.
 *
 * This uses an iterative method that is very stable even when the vectors
 * are far from orthogonal (close to colinear).  The number of iterations
 * and thus the computation time does increase as the vectors become
 * close to colinear, however.
 *
 * If the iteration fails to converge, returns false with vectors as close to
 * orthogonal as possible.
 */
bool
OrthogonalizeBasis(Vec3f *tx, Vec3f *ty, Vec3f *tz,
                     bool normalize, double eps)
{
    Vec3f ax,bx,cx,ay,by,cy,az,bz,cz;

    if (normalize) {
	Normalize(tx);
	Normalize(ty);
	Normalize(tz);
	ax = *tx;
	ay = *ty;
	az = *tz;
    } else {
	ax = *tx;
	ay = *ty;
	az = *tz;
	ax.Normalize();
	ay.Normalize();
	az.Normalize();
    }

    /* Check for colinear vectors. This is not only a quick-out: the
     * error computation below will evaluate to zero if there's no change
     * after an iteration, which can happen either because we have a good
     * solution or because the vectors are colinear.   So we have to check
     * the colinear case beforehand, or we'll get fooled in the error
     * computation.
     */
    if (IsClose(ax,ay,eps) || IsClose(ax,az,eps) || IsClose(ay,az,eps)) {
	return false;
    }

    const int MAX_ITERS = 20;
    int iter;
    for (iter = 0; iter < MAX_ITERS; ++iter) {
	bx = *tx;
	by = *ty;
	bz = *tz;

	bx -= Dot(ay,bx) * ay;
	bx -= Dot(az,bx) * az;

	by -= Dot(ax,by) * ax;
	by -= Dot(az,by) * az;

	bz -= Dot(ax,bz) * ax;
	bz -= Dot(ay,bz) * ay;

	cx = 0.5*(*tx + bx);
	cy = 0.5*(*ty + by);
	cz = 0.5*(*tz + bz);

	if (normalize) {
            cx.Normalize();
            cy.Normalize();
            cz.Normalize();
	}

	Vec3f xDiff = *tx - cx;
	Vec3f yDiff = *ty - cy;
	Vec3f zDiff = *tz - cz;

	double error =
            Dot(xDiff,xDiff) + Dot(yDiff,yDiff) + Dot(zDiff,zDiff);

	// error is squared, so compare to squared tolerance
	if (error < Sqr(eps))
	    break;

	*tx = cx;
	*ty = cy;
	*tz = cz;

	ax = *tx;
	ay = *ty;
	az = *tz;

	if (!normalize) {
            ax.Normalize();
            ay.Normalize();
            az.Normalize();
	}
    }

    return iter < MAX_ITERS;
}

/*
 * BuildOrthonormalFrame constructs two unit vectors *v1 and *v2,
 * with *v1 and *v2 perpendicular to each other and (*this).
 * We arbitrarily cross *this with the X axis to form *v1,
 * and if the result is degenerate, we set *v1 = (Y axis) X *this.
 * If L = length(*this) < eps, we shrink v1 and v2 to be of
 * length L/eps.
 */
void
BuildOrthonormalFrame(Vec3f const &v0,
                        Vec3f* v1,
                        Vec3f* v2, float eps)
{
    float len = v0.GetLength();

    if (len == 0.) {
	*v1 = *v2 = Vec3f(0);
    }
    else {
	Vec3f unitDir = v0 / len;
	*v1 = Vec3f::XAxis() ^ unitDir;

	if (Sqr(*v1) < Sqr(1e-4))
	    *v1 = Vec3f::YAxis() ^ unitDir;

        Normalize(v1);
	*v2 = unitDir ^ *v1;	// this is of unit length

	if (len < eps) {
	    double  desiredLen = len / eps;
	    *v1 *= desiredLen;
	    *v2 *= desiredLen;
	}
    }
}

Vec3f
Slerp(double alpha, const Vec3f &v0, const Vec3f &v1)
{
    // determine the angle between the two lines going from the center of
    // the sphere to v0 and v1.  the projection (dot prod) of one onto the
    // other gives us the arc cosine of the angle between them.
    double angle = acos(Clamp((double)Dot(v0, v1), -1.0, 1.0));

    // Check for very small angle between the vectors, and if so, just lerp them.
    // XXX: This value for epsilon is somewhat arbitrary, and if
    // someone can derive a more meaningful value, that would be fine.
    if ( fabs(angle) < 0.001 ) {
        return Lerp(alpha, v0, v1);
    }

    // compute the sin of the angle, we need it a couple of places
    double sinAngle = sin(angle);

    // Check if the vectors are nearly opposing, and if so,
    // compute an arbitrary orthogonal vector to interpolate across.
    // XXX: Another somewhat arbitrary test for epsilon, but trying to stay
    // within reasonable float precision.
    if ( fabs(sinAngle) < 0.00001 ) {
        Vec3f vX, vY;
        v0.BuildOrthonormalFrame(&vX, &vY);
        Vec3f v = v0 * cos(alpha*M_PI) + vX * sin(alpha*M_PI);
        return v;
    }

    // interpolate
    double oneOverSinAngle = 1.0 / sinAngle;

    return
        v0 * (sin((1.0-alpha)*angle) * oneOverSinAngle) +
        v1 * (sin(     alpha *angle) * oneOverSinAngle);
}


PXR_NAMESPACE_CLOSE_SCOPE
