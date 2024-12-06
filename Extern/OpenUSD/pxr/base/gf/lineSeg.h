// Modified from the original OpenUSD file.
// Copyright 2024 Mihail Mladenov

//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef PXR_BASE_GF_LINE_SEG_H
#define PXR_BASE_GF_LINE_SEG_H

/// \file gf/lineSeg.h
/// \ingroup group_gf_BasicGeometry

#include "pxr/pxr.h"
#include "pxr/base/gf/line.h"
#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/api.h"

#include <float.h>
#include <iosfwd>

PXR_NAMESPACE_OPEN_SCOPE

/// \class LineSeg
/// \ingroup group_gf_BasicGeometry
///
/// Basic type: 3D line segment
///
/// This class represents a three-dimensional line segment in space.
///
class LineSeg {

  public:

    /// The default constructor leaves line parameters undefined.
    LineSeg() {
    }

    /// Construct a line segment that spans two points.
    LineSeg(const Vec3d &p0, const Vec3d &p1 ) {
        _length = _line.Set( p0, p1 - p0 );
    }

    /// Return the point on the segment specified by the parameter t. 
    /// p = p0 + t * (p1 - p0)
    Vec3d GetPoint( double t ) const {return _line.GetPoint( t * _length );}

    /// Return the normalized direction of the line.
    const Vec3d &GetDirection() const { return _line.GetDirection(); }

    /// Return the length of the line
    double GetLength() const { return _length; }

    /// Returns the point on the line that is closest to \p point. If
    /// \p t is not \c NULL, it will be set to the parametric
    /// distance along the line of the closest point.
    GF_API
    Vec3d FindClosestPoint(const Vec3d &point, double *t = NULL) const;

    /// Component-wise equality test. The starting points and directions,
    /// must match exactly for lines to be considered equal.
    bool		operator ==(const LineSeg &l) const {
	return (_line == l._line && _length  == l._length);
    }

    /// Component-wise inequality test. The starting points,
    /// and directions must match exactly for lines to be
    /// considered equal.
    bool		operator !=(const LineSeg &r) const {
	return ! (*this == r);
    }

  private:
    GF_API
    friend bool FindClosestPoints( const Line &, const LineSeg &,
                                     Vec3d *, Vec3d *,
                                     double *, double * );
    GF_API
    friend bool FindClosestPoints( const LineSeg &, const LineSeg &,
                                     Vec3d *, Vec3d *,
                                     double *, double * );

    Line              _line;
    double              _length;   // distance from p0 to p1
};

/// Computes the closets points on \p line and \p seg.
///
/// The two points are returned in \p p1 and \p p2. The parametric distances
/// of \p p1 and \p p2 along the line and segment are returned in \p t1 and \p
/// t2.
///
/// This returns \c false if the lines were close enough to parallel that no
/// points could be computed; in this case, the other return values are
/// undefined.
GF_API
bool FindClosestPoints( const Line &line, const LineSeg &seg,
                          Vec3d *p1 = nullptr, Vec3d *p2 = nullptr,
                          double *t1 = nullptr, double *t2 = nullptr );

/// Computes the closets points on two line segments, \p seg1 and \p seg2. The
/// two points are returned in \p p1 and \p p2. The parametric distances of \p
/// p1 and \p p2 along the segments are returned in \p t1 and \p t2.
///
/// This returns \c false if the lines were close enough to parallel that no
/// points could be computed; in this case, the other return values are
/// undefined.
GF_API
bool FindClosestPoints( const LineSeg &seg1, const LineSeg &seg2,
                          Vec3d *p1 = nullptr, Vec3d *p2 = nullptr,
                          double *t1 = nullptr, double *t2 = nullptr );

/// Output a LineSeg.
/// \ingroup group_gf_DebuggingOutput
GF_API std::ostream &operator<<(std::ostream&, const LineSeg&);

PXR_NAMESPACE_CLOSE_SCOPE

#endif // PXR_BASE_GF_LINE_SEG_H
