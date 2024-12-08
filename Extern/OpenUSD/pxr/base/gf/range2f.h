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
// range.template.h file to make changes.

#ifndef PXR_BASE_GF_RANGE2F_H
#define PXR_BASE_GF_RANGE2F_H

/// \file gf/range2f.h
/// \ingroup group_gf_BasicGeometry

#include "pxr/pxr.h"

#include "pxr/base/gf/api.h"
#include "pxr/base/gf/vec2d.h"
#include "pxr/base/gf/vec2f.h"
#include "pxr/base/gf/traits.h"
#include "pxr/base/tf/hash.h"

#include <cfloat>
#include <cstddef>
#include <iosfwd>

PXR_NAMESPACE_OPEN_SCOPE

class Range2d;
class Range2f;

template <>
struct IsRange<class Range2f> { static const bool value = true; };

/// \class Range2f
/// \ingroup group_gf_BasicGeometry
///
/// Basic type: 2-dimensional floating point range.
///
/// This class represents a 2-dimensional range (or interval) All
/// operations are component-wise and conform to interval mathematics. An
/// empty range is one where max < min.
/// The default empty is [FLT_MAX,-FLT_MAX]
class Range2f
{
public:

    /// Helper typedef.
    typedef Vec2f MinMaxType;

    static const size_t dimension = Vec2f::dimension;
    typedef Vec2f::ScalarType ScalarType;

    /// Sets the range to an empty interval
    // TODO check whether this can be deprecated.
    void inline SetEmpty() {
        _min[0] = _min[1] =  FLT_MAX;
        _max[0] = _max[1] = -FLT_MAX;
    }

    /// The default constructor creates an empty range.
    Range2f() {
        SetEmpty();
    }

    /// This constructor initializes the minimum and maximum points.
    Range2f(const Vec2f &min, const Vec2f &max)
        : _min(min), _max(max)
    {
    }


    /// Construct from Range2d.
    GF_API
    explicit Range2f(class Range2d const &other);

    /// Returns the minimum value of the range.
    const Vec2f &GetMin() const { return _min; }

    /// Returns the maximum value of the range.
    const Vec2f &GetMax() const { return _max; }

    /// Returns the size of the range.
    Vec2f GetSize() const { return _max - _min; }

    /// Returns the midpoint of the range, that is, 0.5*(min+max).
    /// Note: this returns zero in the case of default-constructed ranges,
    /// or ranges set via SetEmpty().
    Vec2f GetMidpoint() const {
        return static_cast<ScalarType>(0.5) * _min
               + static_cast<ScalarType>(0.5) * _max;
    }

    /// Sets the minimum value of the range.
    void SetMin(const Vec2f &min) { _min = min; }

    /// Sets the maximum value of the range.
    void SetMax(const Vec2f &max) { _max = max; }

    /// Returns whether the range is empty (max < min).
    bool IsEmpty() const {
        return _min[0] > _max[0] || _min[1] > _max[1];
    }

    /// Modifies the range if necessary to surround the given value.
    /// \deprecated Use UnionWith() instead.
    void ExtendBy(const Vec2f &point) { UnionWith(point); }

    /// Modifies the range if necessary to surround the given range.
    /// \deprecated Use UnionWith() instead.
    void ExtendBy(const Range2f &range) { UnionWith(range); }

    /// Returns true if the \p point is located inside the range. As with all
    /// operations of this type, the range is assumed to include its extrema.
    bool Contains(const Vec2f &point) const {
        return (point[0] >= _min[0] && point[0] <= _max[0]
             && point[1] >= _min[1] && point[1] <= _max[1]);
    }

    /// Returns true if the \p range is located entirely inside the range. As
    /// with all operations of this type, the ranges are assumed to include
    /// their extrema.
    bool Contains(const Range2f &range) const {
        return Contains(range._min) && Contains(range._max);
    }

    /// Returns true if the \p point is located inside the range. As with all
    /// operations of this type, the range is assumed to include its extrema.
    /// \deprecated Use Contains() instead.
    bool IsInside(const Vec2f &point) const {
        return Contains(point);
    }

    /// Returns true if the \p range is located entirely inside the range. As
    /// with all operations of this type, the ranges are assumed to include
    /// their extrema.
    /// \deprecated Use Contains() instead.
    bool IsInside(const Range2f &range) const {
        return Contains(range);
    }

    /// Returns true if the \p range is located entirely outside the range. As
    /// with all operations of this type, the ranges are assumed to include
    /// their extrema.
    bool IsOutside(const Range2f &range) const {
        return ((range._max[0] < _min[0] || range._min[0] > _max[0])
             || (range._max[1] < _min[1] || range._min[1] > _max[1]));
    }

    /// Returns the smallest \c Range2f which contains both \p a and \p b.
    static Range2f GetUnion(const Range2f &a, const Range2f &b) {
        Range2f res = a;
        _FindMin(res._min,b._min);
        _FindMax(res._max,b._max);
        return res;
    }

    /// Extend \p this to include \p b.
    const Range2f &UnionWith(const Range2f &b) {
        _FindMin(_min,b._min);
        _FindMax(_max,b._max);
        return *this;
    }

    /// Extend \p this to include \p b.
    const Range2f &UnionWith(const Vec2f &b) {
        _FindMin(_min,b);
        _FindMax(_max,b);
        return *this;
    }

    /// Returns the smallest \c Range2f which contains both \p a and \p b
    /// \deprecated Use GetUnion() instead.
    static Range2f Union(const Range2f &a, const Range2f &b) {
        return GetUnion(a, b);
    }

    /// Extend \p this to include \p b.
    /// \deprecated Use UnionWith() instead.
    const Range2f &Union(const Range2f &b) {
        return UnionWith(b);
    }

    /// Extend \p this to include \p b.
    /// \deprecated Use UnionWith() instead.
    const Range2f &Union(const Vec2f &b) {
        return UnionWith(b);
    }

    /// Returns a \c Range2f that describes the intersection of \p a and \p b.
    static Range2f GetIntersection(const Range2f &a, const Range2f &b) {
        Range2f res = a;
        _FindMax(res._min,b._min);
        _FindMin(res._max,b._max);
        return res;
    }

    /// Returns a \c Range2f that describes the intersection of \p a and \p b.
    /// \deprecated Use GetIntersection() instead.
    static Range2f Intersection(const Range2f &a, const Range2f &b) {
        return GetIntersection(a, b);
    }

    /// Modifies this range to hold its intersection with \p b and returns the
    /// result
    const Range2f &IntersectWith(const Range2f &b) {
        _FindMax(_min,b._min);
        _FindMin(_max,b._max);
        return *this;
    }

    /// Modifies this range to hold its intersection with \p b and returns the
    /// result.
    /// \deprecated Use IntersectWith() instead.
    const Range2f &Intersection(const Range2f &b) {
        return IntersectWith(b);
    }

    /// unary sum.
    Range2f &operator +=(const Range2f &b) {
        _min += b._min;
        _max += b._max;
        return *this;
    }

    /// unary difference.
    Range2f &operator -=(const Range2f &b) {
        _min -= b._max;
        _max -= b._min;
        return *this;
    }

    /// unary multiply.
    Range2f &operator *=(double m) {
        if (m > 0) {
            _min *= m;
            _max *= m;
        } else {
            Vec2f tmp = _min;
            _min = _max * m;
            _max = tmp * m;
        }
        return *this;
    }

    /// unary division.
    Range2f &operator /=(double m) {
        return *this *= (1.0 / m);
    }

    /// binary sum.
    Range2f operator +(const Range2f &b) const {
        return Range2f(_min + b._min, _max + b._max);
    }


    /// binary difference.
    Range2f operator -(const Range2f &b) const {
        return Range2f(_min - b._max, _max - b._min);
    }

    /// scalar multiply.
    friend Range2f operator *(double m, const Range2f &r) {
        return (m > 0 ? 
            Range2f(r._min*m, r._max*m) : 
            Range2f(r._max*m, r._min*m));
    }

    /// scalar multiply.
    friend Range2f operator *(const Range2f &r, double m) {
        return (m > 0 ? 
            Range2f(r._min*m, r._max*m) : 
            Range2f(r._max*m, r._min*m));
    }

    /// scalar divide.
    friend Range2f operator /(const Range2f &r, double m) {
        return r * (1.0 / m);
    }

    /// hash.
    friend inline size_t hash_value(const Range2f &r) {
        return TfHash::Combine(r._min, r._max);
    }

    /// The min and max points must match exactly for equality.
    bool operator ==(const Range2f &b) const {
        return (_min == b._min && _max == b._max);
    }

    bool operator !=(const Range2f &b) const {
        return !(*this == b);
    }

    /// Compare this range to a Range2d.
    ///
    /// The values must match exactly and it does exactly what you might
    /// expect when comparing float and double values.
    GF_API inline bool operator ==(const Range2d& other) const;
    GF_API inline bool operator !=(const Range2d& other) const;

    /// Compute the squared distance from a point to the range.
    GF_API
    double GetDistanceSquared(const Vec2f &p) const;

    /// Returns the ith corner of the range, in the following order:
    /// SW, SE, NW, NE.
    GF_API
    Vec2f GetCorner(size_t i) const;

    /// Returns the ith quadrant of the range, in the following order:
    /// SW, SE, NW, NE.
    GF_API
    Range2f GetQuadrant(size_t i) const;

    /// The unit square.
    GF_API
    static const Range2f UnitSquare;

  private:
    /// Minimum and maximum points.
    Vec2f _min, _max;

    /// Extends minimum point if necessary to contain given point.
    static void _FindMin(Vec2f &dest, const Vec2f &point) {
        if (point[0] < dest[0]) dest[0] = point[0];
        if (point[1] < dest[1]) dest[1] = point[1];
    }

    /// Extends maximum point if necessary to contain given point.
    static void _FindMax(Vec2f &dest, const Vec2f &point) {
        if (point[0] > dest[0]) dest[0] = point[0];
        if (point[1] > dest[1]) dest[1] = point[1];
    }
};

/// Output a Range2f.
/// \ingroup group_gf_DebuggingOutput
GF_API std::ostream& operator<<(std::ostream &, Range2f const &);

PXR_NAMESPACE_CLOSE_SCOPE
#include "pxr/base/gf/range2d.h"
PXR_NAMESPACE_OPEN_SCOPE

inline bool
Range2f::operator ==(const Range2d& other) const {
    return _min == Vec2f(other.GetMin()) &&
           _max == Vec2f(other.GetMax());
}

inline bool
Range2f::operator !=(const Range2d& other) const {
    return !(*this == other);
}


PXR_NAMESPACE_CLOSE_SCOPE

#endif // PXR_BASE_GF_RANGE2F_H
