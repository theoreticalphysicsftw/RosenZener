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

#ifndef PXR_BASE_GF_RANGE1D_H
#define PXR_BASE_GF_RANGE1D_H

/// \file gf/range1d.h
/// \ingroup group_gf_BasicGeometry

#include "pxr/pxr.h"

#include "pxr/base/gf/api.h"
#include "pxr/base/gf/traits.h"
#include "pxr/base/tf/hash.h"

#include <cfloat>
#include <cstddef>
#include <iosfwd>

PXR_NAMESPACE_OPEN_SCOPE

class Range1d;
class Range1f;

template <>
struct IsRange<class Range1d> { static const bool value = true; };

/// \class Range1d
/// \ingroup group_gf_BasicGeometry
///
/// Basic type: 1-dimensional floating point range.
///
/// This class represents a 1-dimensional range (or interval) All
/// operations are component-wise and conform to interval mathematics. An
/// empty range is one where max < min.
/// The default empty is [FLT_MAX,-FLT_MAX]
class Range1d
{
public:

    /// Helper typedef.
    typedef double MinMaxType;

    static const size_t dimension = 1;
    typedef MinMaxType ScalarType;

    /// Sets the range to an empty interval
    // TODO check whether this can be deprecated.
    void inline SetEmpty() {
	_min =  FLT_MAX;
	_max = -FLT_MAX;
    }

    /// The default constructor creates an empty range.
    Range1d() {
        SetEmpty();
    }

    /// This constructor initializes the minimum and maximum points.
    Range1d(double min, double max)
        : _min(min), _max(max)
    {
    }


    /// Implicitly convert from Range1f.
    GF_API
    Range1d(class Range1f const &other);

    /// Returns the minimum value of the range.
    double GetMin() const { return _min; }

    /// Returns the maximum value of the range.
    double GetMax() const { return _max; }

    /// Returns the size of the range.
    double GetSize() const { return _max - _min; }

    /// Returns the midpoint of the range, that is, 0.5*(min+max).
    /// Note: this returns zero in the case of default-constructed ranges,
    /// or ranges set via SetEmpty().
    double GetMidpoint() const {
        return static_cast<ScalarType>(0.5) * _min
               + static_cast<ScalarType>(0.5) * _max;
    }

    /// Sets the minimum value of the range.
    void SetMin(double min) { _min = min; }

    /// Sets the maximum value of the range.
    void SetMax(double max) { _max = max; }

    /// Returns whether the range is empty (max < min).
    bool IsEmpty() const {
        return _min > _max;
    }

    /// Modifies the range if necessary to surround the given value.
    /// \deprecated Use UnionWith() instead.
    void ExtendBy(double point) { UnionWith(point); }

    /// Modifies the range if necessary to surround the given range.
    /// \deprecated Use UnionWith() instead.
    void ExtendBy(const Range1d &range) { UnionWith(range); }

    /// Returns true if the \p point is located inside the range. As with all
    /// operations of this type, the range is assumed to include its extrema.
    bool Contains(double point) const {
        return (point >= _min && point <= _max);
    }

    /// Returns true if the \p range is located entirely inside the range. As
    /// with all operations of this type, the ranges are assumed to include
    /// their extrema.
    bool Contains(const Range1d &range) const {
        return Contains(range._min) && Contains(range._max);
    }

    /// Returns true if the \p point is located inside the range. As with all
    /// operations of this type, the range is assumed to include its extrema.
    /// \deprecated Use Contains() instead.
    bool IsInside(double point) const {
        return Contains(point);
    }

    /// Returns true if the \p range is located entirely inside the range. As
    /// with all operations of this type, the ranges are assumed to include
    /// their extrema.
    /// \deprecated Use Contains() instead.
    bool IsInside(const Range1d &range) const {
        return Contains(range);
    }

    /// Returns true if the \p range is located entirely outside the range. As
    /// with all operations of this type, the ranges are assumed to include
    /// their extrema.
    bool IsOutside(const Range1d &range) const {
        return (range._max < _min || range._min > _max);
    }

    /// Returns the smallest \c Range1d which contains both \p a and \p b.
    static Range1d GetUnion(const Range1d &a, const Range1d &b) {
        Range1d res = a;
        _FindMin(res._min,b._min);
        _FindMax(res._max,b._max);
        return res;
    }

    /// Extend \p this to include \p b.
    const Range1d &UnionWith(const Range1d &b) {
        _FindMin(_min,b._min);
        _FindMax(_max,b._max);
        return *this;
    }

    /// Extend \p this to include \p b.
    const Range1d &UnionWith(double b) {
        _FindMin(_min,b);
        _FindMax(_max,b);
        return *this;
    }

    /// Returns the smallest \c Range1d which contains both \p a and \p b
    /// \deprecated Use GetUnion() instead.
    static Range1d Union(const Range1d &a, const Range1d &b) {
        return GetUnion(a, b);
    }

    /// Extend \p this to include \p b.
    /// \deprecated Use UnionWith() instead.
    const Range1d &Union(const Range1d &b) {
        return UnionWith(b);
    }

    /// Extend \p this to include \p b.
    /// \deprecated Use UnionWith() instead.
    const Range1d &Union(double b) {
        return UnionWith(b);
    }

    /// Returns a \c Range1d that describes the intersection of \p a and \p b.
    static Range1d GetIntersection(const Range1d &a, const Range1d &b) {
        Range1d res = a;
        _FindMax(res._min,b._min);
        _FindMin(res._max,b._max);
        return res;
    }

    /// Returns a \c Range1d that describes the intersection of \p a and \p b.
    /// \deprecated Use GetIntersection() instead.
    static Range1d Intersection(const Range1d &a, const Range1d &b) {
        return GetIntersection(a, b);
    }

    /// Modifies this range to hold its intersection with \p b and returns the
    /// result
    const Range1d &IntersectWith(const Range1d &b) {
        _FindMax(_min,b._min);
        _FindMin(_max,b._max);
        return *this;
    }

    /// Modifies this range to hold its intersection with \p b and returns the
    /// result.
    /// \deprecated Use IntersectWith() instead.
    const Range1d &Intersection(const Range1d &b) {
        return IntersectWith(b);
    }

    /// unary sum.
    Range1d &operator +=(const Range1d &b) {
        _min += b._min;
        _max += b._max;
        return *this;
    }

    /// unary difference.
    Range1d &operator -=(const Range1d &b) {
        _min -= b._max;
        _max -= b._min;
        return *this;
    }

    /// unary multiply.
    Range1d &operator *=(double m) {
        if (m > 0) {
            _min *= m;
            _max *= m;
        } else {
            double tmp = _min;
            _min = _max * m;
            _max = tmp * m;
        }
        return *this;
    }

    /// unary division.
    Range1d &operator /=(double m) {
        return *this *= (1.0 / m);
    }

    /// binary sum.
    Range1d operator +(const Range1d &b) const {
        return Range1d(_min + b._min, _max + b._max);
    }


    /// binary difference.
    Range1d operator -(const Range1d &b) const {
        return Range1d(_min - b._max, _max - b._min);
    }

    /// scalar multiply.
    friend Range1d operator *(double m, const Range1d &r) {
        return (m > 0 ? 
            Range1d(r._min*m, r._max*m) : 
            Range1d(r._max*m, r._min*m));
    }

    /// scalar multiply.
    friend Range1d operator *(const Range1d &r, double m) {
        return (m > 0 ? 
            Range1d(r._min*m, r._max*m) : 
            Range1d(r._max*m, r._min*m));
    }

    /// scalar divide.
    friend Range1d operator /(const Range1d &r, double m) {
        return r * (1.0 / m);
    }

    /// hash.
    friend inline size_t hash_value(const Range1d &r) {
        return TfHash::Combine(r._min, r._max);
    }

    /// The min and max points must match exactly for equality.
    bool operator ==(const Range1d &b) const {
        return (_min == b._min && _max == b._max);
    }

    bool operator !=(const Range1d &b) const {
        return !(*this == b);
    }

    /// Compare this range to a Range1f.
    ///
    /// The values must match exactly and it does exactly what you might
    /// expect when comparing float and double values.
    GF_API inline bool operator ==(const Range1f& other) const;
    GF_API inline bool operator !=(const Range1f& other) const;

    /// Compute the squared distance from a point to the range.
    GF_API
    double GetDistanceSquared(double p) const;


  private:
    /// Minimum and maximum points.
    double _min, _max;

    /// Extends minimum point if necessary to contain given point.
    static void _FindMin(double &dest, double point) {
        if (point < dest) dest = point;
    }

    /// Extends maximum point if necessary to contain given point.
    static void _FindMax(double &dest, double point) {
        if (point > dest) dest = point;
    }
};

/// Output a Range1d.
/// \ingroup group_gf_DebuggingOutput
GF_API std::ostream& operator<<(std::ostream &, Range1d const &);

PXR_NAMESPACE_CLOSE_SCOPE
#include "pxr/base/gf/range1f.h"
PXR_NAMESPACE_OPEN_SCOPE

inline bool
Range1d::operator ==(const Range1f& other) const {
    return _min == double(other.GetMin()) &&
           _max == double(other.GetMax());
}

inline bool
Range1d::operator !=(const Range1f& other) const {
    return !(*this == other);
}


PXR_NAMESPACE_CLOSE_SCOPE

#endif // PXR_BASE_GF_RANGE1D_H