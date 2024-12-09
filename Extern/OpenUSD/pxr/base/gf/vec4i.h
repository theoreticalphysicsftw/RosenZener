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

#ifndef PXR_BASE_GF_VEC4I_H
#define PXR_BASE_GF_VEC4I_H

/// \file gf/vec4i.h
/// \ingroup group_gf_LinearAlgebra

#include "pxr/pxr.h"
#include "pxr/base/tf/diagnostic.h"
#include "pxr/base/gf/api.h"
#include "pxr/base/gf/limits.h"
#include "pxr/base/gf/traits.h"
#include "pxr/base/tf/hash.h"

#include <cstddef>

#include <iosfwd>

PXR_NAMESPACE_OPEN_SCOPE

class Vec4i;

template <>
struct IsVec<class Vec4i> { static const bool value = true; };

/// \class Vec4i
/// \ingroup group_gf_LinearAlgebra
///
/// Basic type for a vector of 4 int components.
///
/// Represents a vector of 4 components of type \c int.
/// It is intended to be fast and simple.
///
class Vec4i
{
public:
    /// Scalar element type and dimension.
    typedef int ScalarType;
    static const size_t dimension = 4;

    /// Default constructor does no initialization.
    Vec4i() = default;

    /// Initialize all elements to a single value.
    constexpr explicit Vec4i(int value)
        : _data{ value, value, value, value }
    {
    }

    /// Initialize all elements with explicit arguments.
    constexpr Vec4i(int s0, int s1, int s2, int s3)
        : _data{ s0, s1, s2, s3 }
    {
    }

    /// Construct with pointer to values.
    template <class Scl>
    constexpr explicit Vec4i(Scl const *p)
        : _data{ p[0], p[1], p[2], p[3] }
    {
    }
 
    /// Create a unit vector along the X-axis.
    static Vec4i XAxis() {
        Vec4i result(0);
        result[0] = 1;
        return result;
    }
    /// Create a unit vector along the Y-axis.
    static Vec4i YAxis() {
        Vec4i result(0);
        result[1] = 1;
        return result;
    }
    /// Create a unit vector along the Z-axis.
    static Vec4i ZAxis() {
        Vec4i result(0);
        result[2] = 1;
        return result;
    }
    /// Create a unit vector along the W-axis.
    static Vec4i WAxis() {
        Vec4i result(0);
        result[3] = 1;
        return result;
    }

    /// Create a unit vector along the i-th axis, zero-based.  Return the zero
    /// vector if \p i is greater than or equal to 4.
    static Vec4i Axis(size_t i) {
        Vec4i result(0);
        if (i < 4)
            result[i] = 1;
        return result;
    }

    /// Set all elements with passed arguments.
    Vec4i &Set(int s0, int s1, int s2, int s3) {
        _data[0] = s0;
        _data[1] = s1;
        _data[2] = s2;
        _data[3] = s3;
        return *this;
    }

    /// Set all elements with a pointer to data.
    Vec4i &Set(int const *a) {
        return Set(a[0], a[1], a[2], a[3]);
    }

    /// Direct data access.
    int const *data() const { return _data; }
    int *data() { return _data; }
    int const *GetArray() const { return data(); }

    /// Indexing.
    int const &operator[](size_t i) const { return _data[i]; }
    int &operator[](size_t i) { return _data[i]; }

    /// Hash.
    friend inline size_t hash_value(Vec4i const &vec) {
        return TfHash::Combine(vec[0], vec[1], vec[2], vec[3]);
    }

    /// Equality comparison.
    bool operator==(Vec4i const &other) const {
        return _data[0] == other[0] &&
               _data[1] == other[1] &&
               _data[2] == other[2] &&
               _data[3] == other[3];
    }
    bool operator!=(Vec4i const &other) const {
        return !(*this == other);
    }

    // TODO Add inequality for other vec types...
    /// Equality comparison.
    GF_API
    bool operator==(class Vec4d const &other) const;
    /// Equality comparison.
    GF_API
    bool operator==(class Vec4f const &other) const;
    /// Equality comparison.
    GF_API
    bool operator==(class Vec4h const &other) const;
    
    /// Create a vec with negated elements.
    Vec4i operator-() const {
        return Vec4i(-_data[0], -_data[1], -_data[2], -_data[3]);
    }

    /// Addition.
    Vec4i &operator+=(Vec4i const &other) {
        _data[0] += other[0];
        _data[1] += other[1];
        _data[2] += other[2];
        _data[3] += other[3];
        return *this;
    }
    friend Vec4i operator+(Vec4i const &l, Vec4i const &r) {
        return Vec4i(l) += r;
    }

    /// Subtraction.
    Vec4i &operator-=(Vec4i const &other) {
        _data[0] -= other[0];
        _data[1] -= other[1];
        _data[2] -= other[2];
        _data[3] -= other[3];
        return *this;
    }
    friend Vec4i operator-(Vec4i const &l, Vec4i const &r) {
        return Vec4i(l) -= r;
    }

    /// Multiplication by scalar.
    Vec4i &operator*=(double s) {
        _data[0] *= s;
        _data[1] *= s;
        _data[2] *= s;
        _data[3] *= s;
        return *this;
    }
    Vec4i operator*(double s) const {
        return Vec4i(*this) *= s;
    }
    friend Vec4i operator*(double s, Vec4i const &v) {
        return v * s;
    }

        /// Division by scalar.
    Vec4i &operator/=(int s) {
        _data[0] /= s;
        _data[1] /= s;
        _data[2] /= s;
        _data[3] /= s;
        return *this;
    }
    Vec4i operator/(int s) const {
        return Vec4i(*this) /= s;
    }
    
    /// See Dot().
    int operator*(Vec4i const &v) const {
        return _data[0] * v[0] + _data[1] * v[1] + _data[2] * v[2] + _data[3] * v[3];
    }

    /// Returns the projection of \p this onto \p v. That is:
    /// \code
    /// v * (*this * v)
    /// \endcode
    Vec4i GetProjection(Vec4i const &v) const {
        return v * (*this * v);
    }

    /// Returns the orthogonal complement of \p this->GetProjection(b).
    /// That is:
    /// \code
    ///  *this - this->GetProjection(b)
    /// \endcode
    Vec4i GetComplement(Vec4i const &b) const {
        return *this - this->GetProjection(b);
    }

    /// Squared length.
    int GetLengthSq() const {
        return *this * *this;
    }

 
private:
    int _data[4];
};

/// Output a Vec4i.
/// \ingroup group_gf_DebuggingOutput
GF_API std::ostream& operator<<(std::ostream &, Vec4i const &);


/// Returns component-wise multiplication of vectors \p v1 and \p v2.
inline Vec4i
CompMult(Vec4i const &v1, Vec4i const &v2) {
    return Vec4i(
        v1[0] * v2[0],
        v1[1] * v2[1],
        v1[2] * v2[2],
        v1[3] * v2[3]
        );
}

/// Returns component-wise quotient of vectors \p v1 and \p v2.
inline Vec4i
CompDiv(Vec4i const &v1, Vec4i const &v2) {
    return Vec4i(
        v1[0] / v2[0],
        v1[1] / v2[1],
        v1[2] / v2[2],
        v1[3] / v2[3]
        );
}

/// Returns the dot (inner) product of two vectors.
inline int
Dot(Vec4i const &v1, Vec4i const &v2) {
    return v1 * v2;
}

 
PXR_NAMESPACE_CLOSE_SCOPE

#endif // PXR_BASE_GF_VEC4I_H