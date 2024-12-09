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
// vec.template.h file to make changes.

#ifndef PXR_BASE_GF_VEC4D_H
#define PXR_BASE_GF_VEC4D_H

/// \file gf/vec4d.h
/// \ingroup group_gf_LinearAlgebra

#include "pxr/pxr.h"
#include "pxr/base/tf/diagnostic.h"
#include "pxr/base/gf/api.h"
#include "pxr/base/gf/limits.h"
#include "pxr/base/gf/traits.h"
#include "pxr/base/gf/math.h"
#include "pxr/base/tf/hash.h"

#include <cstddef>
#include <cmath>

#include <iosfwd>

PXR_NAMESPACE_OPEN_SCOPE

class Vec4d;

template <>
struct IsVec<class Vec4d> { static const bool value = true; };

/// \class Vec4d
/// \ingroup group_gf_LinearAlgebra
///
/// Basic type for a vector of 4 double components.
///
/// Represents a vector of 4 components of type \c double.
/// It is intended to be fast and simple.
///
class Vec4d
{
public:
    /// Scalar element type and dimension.
    typedef double ScalarType;
    static const size_t dimension = 4;

    /// Default constructor does no initialization.
    Vec4d() = default;

    /// Initialize all elements to a single value.
    constexpr explicit Vec4d(double value)
        : _data{ value, value, value, value }
    {
    }

    /// Initialize all elements with explicit arguments.
    constexpr Vec4d(double s0, double s1, double s2, double s3)
        : _data{ s0, s1, s2, s3 }
    {
    }

    /// Construct with pointer to values.
    template <class Scl>
    constexpr explicit Vec4d(Scl const *p)
        : _data{ p[0], p[1], p[2], p[3] }
    {
    }

    /// Implicitly convert from Vec4f.
    Vec4d(class Vec4f const &other);

    /// Implicitly convert from Vec4h.
    Vec4d(class Vec4h const &other);

    /// Implicitly convert from Vec4i.
    Vec4d(class Vec4i const &other);
 
    /// Create a unit vector along the X-axis.
    static Vec4d XAxis() {
        Vec4d result(0);
        result[0] = 1;
        return result;
    }
    /// Create a unit vector along the Y-axis.
    static Vec4d YAxis() {
        Vec4d result(0);
        result[1] = 1;
        return result;
    }
    /// Create a unit vector along the Z-axis.
    static Vec4d ZAxis() {
        Vec4d result(0);
        result[2] = 1;
        return result;
    }
    /// Create a unit vector along the W-axis.
    static Vec4d WAxis() {
        Vec4d result(0);
        result[3] = 1;
        return result;
    }

    /// Create a unit vector along the i-th axis, zero-based.  Return the zero
    /// vector if \p i is greater than or equal to 4.
    static Vec4d Axis(size_t i) {
        Vec4d result(0);
        if (i < 4)
            result[i] = 1;
        return result;
    }

    /// Set all elements with passed arguments.
    Vec4d &Set(double s0, double s1, double s2, double s3) {
        _data[0] = s0;
        _data[1] = s1;
        _data[2] = s2;
        _data[3] = s3;
        return *this;
    }

    /// Set all elements with a pointer to data.
    Vec4d &Set(double const *a) {
        return Set(a[0], a[1], a[2], a[3]);
    }

    /// Direct data access.
    double const *data() const { return _data; }
    double *data() { return _data; }
    double const *GetArray() const { return data(); }

    /// Indexing.
    double const &operator[](size_t i) const { return _data[i]; }
    double &operator[](size_t i) { return _data[i]; }

    /// Hash.
    friend inline size_t hash_value(Vec4d const &vec) {
        return TfHash::Combine(vec[0], vec[1], vec[2], vec[3]);
    }

    /// Equality comparison.
    bool operator==(Vec4d const &other) const {
        return _data[0] == other[0] &&
               _data[1] == other[1] &&
               _data[2] == other[2] &&
               _data[3] == other[3];
    }
    bool operator!=(Vec4d const &other) const {
        return !(*this == other);
    }

    // TODO Add inequality for other vec types...
    /// Equality comparison.
    GF_API
    bool operator==(class Vec4f const &other) const;
    /// Equality comparison.
    GF_API
    bool operator==(class Vec4h const &other) const;
    /// Equality comparison.
    GF_API
    bool operator==(class Vec4i const &other) const;
    
    /// Create a vec with negated elements.
    Vec4d operator-() const {
        return Vec4d(-_data[0], -_data[1], -_data[2], -_data[3]);
    }

    /// Addition.
    Vec4d &operator+=(Vec4d const &other) {
        _data[0] += other[0];
        _data[1] += other[1];
        _data[2] += other[2];
        _data[3] += other[3];
        return *this;
    }
    friend Vec4d operator+(Vec4d const &l, Vec4d const &r) {
        return Vec4d(l) += r;
    }

    /// Subtraction.
    Vec4d &operator-=(Vec4d const &other) {
        _data[0] -= other[0];
        _data[1] -= other[1];
        _data[2] -= other[2];
        _data[3] -= other[3];
        return *this;
    }
    friend Vec4d operator-(Vec4d const &l, Vec4d const &r) {
        return Vec4d(l) -= r;
    }

    /// Multiplication by scalar.
    Vec4d &operator*=(double s) {
        _data[0] *= s;
        _data[1] *= s;
        _data[2] *= s;
        _data[3] *= s;
        return *this;
    }
    Vec4d operator*(double s) const {
        return Vec4d(*this) *= s;
    }
    friend Vec4d operator*(double s, Vec4d const &v) {
        return v * s;
    }

        /// Division by scalar.
    // TODO should divide by the scalar type.
    Vec4d &operator/=(double s) {
        // TODO This should not multiply by 1/s, it should do the division.
        // Doing the division is more numerically stable when s is close to
        // zero.
        return *this *= (1.0 / s);
    }
    Vec4d operator/(double s) const {
        return *this * (1.0 / s);
    }
    
    /// See Dot().
    double operator*(Vec4d const &v) const {
        return _data[0] * v[0] + _data[1] * v[1] + _data[2] * v[2] + _data[3] * v[3];
    }

    /// Returns the projection of \p this onto \p v. That is:
    /// \code
    /// v * (*this * v)
    /// \endcode
    Vec4d GetProjection(Vec4d const &v) const {
        return v * (*this * v);
    }

    /// Returns the orthogonal complement of \p this->GetProjection(b).
    /// That is:
    /// \code
    ///  *this - this->GetProjection(b)
    /// \endcode
    Vec4d GetComplement(Vec4d const &b) const {
        return *this - this->GetProjection(b);
    }

    /// Squared length.
    double GetLengthSq() const {
        return *this * *this;
    }

    /// Length
    double GetLength() const {
        return Sqrt(GetLengthSq());
    }

    /// Normalizes the vector in place to unit length, returning the
    /// length before normalization. If the length of the vector is
    /// smaller than \p eps, then the vector is set to vector/\c eps.
    /// The original length of the vector is returned. See also Normalize().
    ///
    /// \todo This was fixed for bug 67777. This is a gcc64 optimizer bug.
    /// By tickling the code, it no longer tries to write into
    /// an illegal memory address (in the code section of memory).
    double Normalize(double eps = GF_MIN_VECTOR_LENGTH) {
        // TODO this seems suspect...  suggest dividing by length so long as
        // length is not zero.
        double length = GetLength();
        *this /= (length > eps) ? length : eps;
        return length;
    }

    Vec4d GetNormalized(double eps = GF_MIN_VECTOR_LENGTH) const {
        Vec4d normalized(*this);
        normalized.Normalize(eps);
        return normalized;
    }

  
private:
    double _data[4];
};

/// Output a Vec4d.
/// \ingroup group_gf_DebuggingOutput
GF_API std::ostream& operator<<(std::ostream &, Vec4d const &);


PXR_NAMESPACE_CLOSE_SCOPE

#include "pxr/base/gf/vec4f.h"
#include "pxr/base/gf/vec4h.h"
#include "pxr/base/gf/vec4i.h"

PXR_NAMESPACE_OPEN_SCOPE

inline
Vec4d::Vec4d(class Vec4f const &other)
{
    _data[0] = other[0];
    _data[1] = other[1];
    _data[2] = other[2];
    _data[3] = other[3];
}
inline
Vec4d::Vec4d(class Vec4h const &other)
{
    _data[0] = other[0];
    _data[1] = other[1];
    _data[2] = other[2];
    _data[3] = other[3];
}
inline
Vec4d::Vec4d(class Vec4i const &other)
{
    _data[0] = other[0];
    _data[1] = other[1];
    _data[2] = other[2];
    _data[3] = other[3];
}

/// Returns component-wise multiplication of vectors \p v1 and \p v2.
inline Vec4d
CompMult(Vec4d const &v1, Vec4d const &v2) {
    return Vec4d(
        v1[0] * v2[0],
        v1[1] * v2[1],
        v1[2] * v2[2],
        v1[3] * v2[3]
        );
}

/// Returns component-wise quotient of vectors \p v1 and \p v2.
inline Vec4d
CompDiv(Vec4d const &v1, Vec4d const &v2) {
    return Vec4d(
        v1[0] / v2[0],
        v1[1] / v2[1],
        v1[2] / v2[2],
        v1[3] / v2[3]
        );
}

/// Returns the dot (inner) product of two vectors.
inline double
Dot(Vec4d const &v1, Vec4d const &v2) {
    return v1 * v2;
}


/// Returns the geometric length of \c v.
inline double
GetLength(Vec4d const &v)
{
    return v.GetLength();
}

/// Normalizes \c *v in place to unit length, returning the length before
/// normalization. If the length of \c *v is smaller than \p eps then \c *v is
/// set to \c *v/eps.  The original length of \c *v is returned.
inline double
Normalize(Vec4d *v, double eps = GF_MIN_VECTOR_LENGTH)
{
    return v->Normalize(eps);
}

/// Returns a normalized (unit-length) vector with the same direction as \p v.
/// If the length of this vector is smaller than \p eps, the vector divided by
/// \p eps is returned.
inline Vec4d
GetNormalized(Vec4d const &v, double eps = GF_MIN_VECTOR_LENGTH)
{
    return v.GetNormalized(eps);
}

/// Returns the projection of \p a onto \p b. That is:
/// \code
/// b * (a * b)
/// \endcode
inline Vec4d
GetProjection(Vec4d const &a, Vec4d const &b)
{
    return a.GetProjection(b);
}

/// Returns the orthogonal complement of \p a.GetProjection(b). That is:
/// \code
///  a - a.GetProjection(b)
/// \endcode
inline Vec4d
GetComplement(Vec4d const &a, Vec4d const &b)
{
    return a.GetComplement(b);
}

/// Tests for equality within a given tolerance, returning \c true if the
/// length of the difference vector is less than or equal to \p tolerance.
inline bool
IsClose(Vec4d const &v1, Vec4d const &v2, double tolerance)
{
    Vec4d delta = v1 - v2;
    return delta.GetLengthSq() <= tolerance * tolerance;
}

 
 
PXR_NAMESPACE_CLOSE_SCOPE

#endif // PXR_BASE_GF_VEC4D_H