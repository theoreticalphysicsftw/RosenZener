// Modified from the original OpenUSD file.
// Copyright 2024 Mihail Mladenov

//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef PXR_BASE_GF_LINE2D_H
#define PXR_BASE_GF_LINE2D_H

/// \file gf/line2d.h
/// \ingroup group_gf_BasicGeometry

#include "pxr/pxr.h"
#include "pxr/base/gf/vec2d.h"
#include "pxr/base/gf/api.h"

#include <float.h>

PXR_NAMESPACE_OPEN_SCOPE

/// \class Line2d
/// \ingroup group_gf_BasicGeometry
///
/// Basic type: 2D line
///
/// This class represents a two-dimensional line in space.  Lines are
/// constructed from a point, \p p0, and a direction, dir.  The direction is
/// normalized in the constructor. 
///
/// The line is kept in a parametric represention, p = p0 + t * dir. 
///
class Line2d {

  public:

    /// The default constructor leaves line parameters undefined.
    Line2d() {
    }

    /// Construct a line from a point and a direction.
    Line2d(const Vec2d &p0, const Vec2d &dir ) {
        Set( p0, dir );
    }

    double Set(const Vec2d &p0, const Vec2d &dir ) {
        _p0 = p0;
        _dir = dir;
        return _dir.Normalize();
    }

    /// Return the point on the line at \p ( p0 + t * dir ).
    /// Remember dir has been normalized so t represents a unit distance.
    Vec2d GetPoint( double t ) const { return _p0 + _dir * t; }

    /// Return the normalized direction of the line.
    const Vec2d &GetDirection() const { return _dir; }

    /// Returns the point on the line that is closest to \p point. If \p t is
    /// not \c NULL, it will be set to the parametric distance along the line
    /// of the returned point.
    GF_API
    Vec2d FindClosestPoint(const Vec2d &point, double *t = NULL) const;

    /// Component-wise equality test. The starting points and directions, must
    /// match exactly for lines to be considered equal.
    bool		operator ==(const Line2d &l) const {
	return _p0 == l._p0 &&	_dir  == l._dir;
    }

    /// Component-wise inequality test. The starting points, and directions
    /// must match exactly for lines to be considered equal.
    bool		operator !=(const Line2d &r) const {
	return ! (*this == r);
    }

  private:
    GF_API
    friend bool FindClosestPoints( const Line2d &, const Line2d &,
                                     Vec2d *, Vec2d *,
                                     double *, double *);

    // Parametric description:
    //  l(t) = _p0 + t * _length * _dir;
    Vec2d             _p0;
    Vec2d             _dir;   
};

/// Computes the closets points between two lines.
///
/// The two points are returned in \p p1 and \p p2.  The parametric distance
/// of each point on the lines is returned in \p t1 and \p t2.
///
/// This returns \c false if the lines were close enough to parallel that no
/// points could be computed; in this case, the other return values are
/// undefined.
GF_API
bool FindClosestPoints(const Line2d &l1, const Line2d &l2,
                         Vec2d *p1 = nullptr, Vec2d *p2 = nullptr,
                         double *t1 = nullptr, double *t2 = nullptr);

PXR_NAMESPACE_CLOSE_SCOPE

#endif // PXR_BASE_GF_LINE2D_H
