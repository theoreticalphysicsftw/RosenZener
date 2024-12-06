// Modified from the original OpenUSD file.
// Copyright 2024 Mihail Mladenov

//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef PXR_BASE_GF_SIZE2_H
#define PXR_BASE_GF_SIZE2_H

/// \file gf/size2.h
/// \ingroup group_gf_LinearAlgebra

#include "pxr/pxr.h"
#include "pxr/base/arch/inttypes.h"
#include "pxr/base/gf/vec2i.h"
#include "pxr/base/gf/api.h"

#include <iosfwd>

PXR_NAMESPACE_OPEN_SCOPE

/// \class Size2
/// \ingroup group_gf_LinearAlgebra
///
/// Two-dimensional array of sizes
///
/// Size2 is used to represent pairs of counts.  It is based on the datatype
/// size_t, and thus can only represent non-negative values in each dimension.
/// If you need to represent negative numbers as well, use Vec2i.
///
/// Usage of Size2 is similar to that of Vec2i, except that all
/// mathematical operations are componentwise (including multiplication).
///
class Size2 {
public:
    /// Default constructor initializes components to zero.
    Size2() {
        Set(0, 0);
    }

    /// Copy constructor.
    Size2(const Size2& o) {
        *this = o;
    }

    /// Conversion from Vec2i.
    explicit Size2(const Vec2i&o) {
        Set(o[0], o[1]);
    }

    /// Construct from an array.
    Size2(const size_t v[2]) {
        Set(v);
    }

    /// Construct from two values.
    Size2(size_t v0, size_t v1) {
        Set(v0, v1);
    }

    /// Set to the values in a given array.
    Size2 & Set(const size_t v[2]) {
        _vec[0] = v[0]; 
        _vec[1] = v[1]; 
        return *this;
    }

    /// Set to values passed directly.
    Size2 & Set(size_t v0, size_t v1) {
        _vec[0] = v0; 
        _vec[1] = v1; 
        return *this;
    }

    /// Array operator.
    size_t & operator [](size_t i) {
        return _vec[i];
    }

    /// Const array operator.
    const size_t & operator [](size_t i) const {
        return _vec[i];
    }

    /// Component-wise equality.
    bool operator ==(const Size2 &v) const {
        return _vec[0] == v._vec[0] && _vec[1] == v._vec[1];
    }

    /// Component-wise inequality.
    bool operator !=(const Size2 &v) const {
        return ! (*this == v);
    }

    /// Component-wise in-place addition.
    Size2 & operator +=(const Size2 &v) {
        _vec[0] += v._vec[0]; 
        _vec[1] += v._vec[1]; 
        return *this;
    }

    /// Component-wise in-place subtraction.
    Size2 & operator -=(const Size2 &v) {
        _vec[0] -= v._vec[0]; 
        _vec[1] -= v._vec[1]; 
        return *this;
    }

    /// Component-wise in-place multiplication.
    Size2 & operator *=(Size2 const &v) {
        _vec[0] *= v._vec[0];
        _vec[1] *= v._vec[1];
        return *this;
    }

    /// Component-wise in-place multiplication by a scalar.
    Size2 & operator *=(int d) {
        _vec[0] = _vec[0] * d;
        _vec[1] = _vec[1] * d;
        return *this;
    }

    /// Component-wise in-place division by a scalar.
    Size2 & operator /=(int d) {
        _vec[0] = _vec[0] / d;
        _vec[1] = _vec[1] / d;
        return *this;
    }

    /// Component-wise addition.
    friend Size2 operator +(const Size2 &v1, const Size2 &v2) {
        return Size2(v1._vec[0]+v2._vec[0],
                       v1._vec[1]+v2._vec[1]);
    }

    /// Component-wise subtraction.
    friend Size2 operator -(const Size2 &v1, const Size2 &v2) {
        return Size2(v1._vec[0]-v2._vec[0],
                       v1._vec[1]-v2._vec[1]);
    }

    /// Component-wise multiplication.
    friend Size2 operator *(const Size2 &v1, const Size2 &v2) {
        return Size2(v1._vec[0]*v2._vec[0],
                       v1._vec[1]*v2._vec[1]);
    }

    /// Component-wise multiplication by a scalar.
    friend Size2 operator *(const Size2 &v1, int s) {
        return Size2(v1._vec[0]*s,
                       v1._vec[1]*s);
    }

    /// Component-wise multiplication by a scalar.
    friend Size2 operator *(int s, const Size2 &v1) {
        return Size2(v1._vec[0]*s,
                       v1._vec[1]*s);
    }

    /// Component-wise division by a scalar.
    friend Size2 operator /(const Size2 &v1, int s) {
        return Size2(v1._vec[0]/s,
                       v1._vec[1]/s);
    }

    /// Output operator.
    GF_API
    friend std::ostream &operator<<(std::ostream &o, Size2 const &v);

    /// Conversion to Vec2i.
    operator Vec2i() const {
        return Vec2i(_vec[0], _vec[1]);
    }
 private:
    size_t _vec[2];
};

// Friend functions must be declared
GF_API std::ostream &operator<<(std::ostream &o, Size2 const &v);

PXR_NAMESPACE_CLOSE_SCOPE

#endif // PXR_BASE_GF_SIZE2_H 
