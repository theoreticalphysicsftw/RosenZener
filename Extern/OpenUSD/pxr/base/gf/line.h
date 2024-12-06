// Modified from the original OpenUSD file.
// Copyright 2024 Mihail Mladenov

// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef PXR_BASE_GF_LINE_H
#define PXR_BASE_GF_LINE_H

/// \file gf/line.h
/// \ingroup group_gf_BasicGeometry

#include "pxr/pxr.h"
#include "pxr/base/gf/vec3d.h"

#include <float.h>
#include <iosfwd>

PXR_NAMESPACE_OPEN_SCOPE

/// \class Line
/// \ingroup group_gf_BasicGeometry
///
/// Basic type: 3D line
///
/// This class represents a three-dimensional line in space.  Lines are
/// constructed from a point, \p p0, and a direction, dir.  The direction is
/// normalized in the constructor. 
///
/// The line is kept in a parametric represention, p = p0 + t * dir. 
///
class Line {

  public:

    /// The default constructor leaves line parameters undefined.
    Line() {
    }

    /// Construct a line from a point and a direction.
    Line(const Vec3d &p0, const Vec3d &dir ) {
        Set( p0, dir );
    }

    double Set(const Vec3d &p0, const Vec3d &dir ) {
        _p0 = p0;
        _dir = dir;
        return _dir.Normalize();
    }

    /// Return the point on the line at \p ( p0 + t * dir ).
    /// Remember dir has been normalized so t represents a unit distance.
    Vec3d GetPoint( double t ) const { return _p0 + _dir * t; }

    /// Return the normalized direction of the line.
    const Vec3d &GetDirection() const { return _dir; }

    /// Returns the point on the line that is closest to \p point. If \p t is
    /// not \c NULL, it will be set to the parametric distance along the line
    /// of the returned point.
    GF_API
    Vec3d FindClosestPoint(const Vec3d &point, double *t = NULL) const;

    /// Component-wise equality test. The starting points and directions,
    /// must match exactly for lines to be considered equal.
    bool		operator ==(const Line &l) const {
	return _p0 == l._p0 &&	_dir  == l._dir;
    }

    /// Component-wise inequality test. The starting points, and directions
    /// must match exactly for lines to be considered equal.
    bool		operator !=(const Line &r) const {
	return ! (*this == r);
    }

  private:
    GF_API
    friend bool FindClosestPoints( const Line &, const Line &,
                                     Vec3d *, Vec3d *,
                                     double *, double * );
    // Parametric description:
    //  l(t) = _p0 + t * _length * _dir;
    Vec3d             _p0;
    Vec3d             _dir;   
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
bool FindClosestPoints(const Line &l1, const Line &l2,
                         Vec3d *p1 = nullptr, Vec3d *p2 = nullptr,
                         double *t1 = nullptr, double *t2 = nullptr);

/// Output a Line.
/// \ingroup group_gf_DebuggingOutput
GF_API std::ostream &operator<<(std::ostream&, const Line&);

PXR_NAMESPACE_CLOSE_SCOPE

#endif // PXR_BASE_GF_LINE_H
