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
// matrix2.template.h file to make changes.

#ifndef PXR_BASE_GF_MATRIX2F_H
#define PXR_BASE_GF_MATRIX2F_H

/// \file gf/matrix2f.h
/// \ingroup group_gf_LinearAlgebra

#include "pxr/pxr.h"
#include "pxr/base/gf/api.h"
#include "pxr/base/gf/declare.h"
#include "pxr/base/gf/matrixData.h"
#include "pxr/base/gf/vec2f.h"
#include "pxr/base/gf/traits.h"
#include "pxr/base/tf/hash.h"

#include <iosfwd>
#include <vector>

PXR_NAMESPACE_OPEN_SCOPE

template <>
struct IsMatrix<class Matrix2f> { static const bool value = true; };

class Matrix2d;
class Matrix2f;

/// \class Matrix2f
/// \ingroup group_gf_LinearAlgebra
///
/// Stores a 2x2 matrix of \c float elements. A basic type.
///
/// Matrices are defined to be in row-major order, so <c>matrix[i][j]</c>
/// indexes the element in the \e i th row and the \e j th column.
///
class Matrix2f
{
public:
    typedef float ScalarType;

    static const size_t numRows = 2;
    static const size_t numColumns = 2;

    /// Default constructor. Leaves the matrix component values undefined.
    Matrix2f() = default;

    /// Constructor. Initializes the matrix from 4 independent
    /// \c float values, specified in row-major order. For example,
    /// parameter \e m10 specifies the value in row 1 and column 0.
    Matrix2f(float m00, float m01, 
               float m10, float m11) {
        Set(m00, m01, 
            m10, m11);
    }

    /// Constructor. Initializes the matrix from a 2x2 array
    /// of \c float values, specified in row-major order.
    Matrix2f(const float m[2][2]) {
        Set(m);
    }

    /// Constructor. Explicitly initializes the matrix to \e s times the
    /// identity matrix.
    explicit Matrix2f(float s) {
        SetDiagonal(s);
    }

    /// This explicit constructor initializes the matrix to \p s times
    /// the identity matrix.
    explicit Matrix2f(int s) {
        SetDiagonal(s);
    }

    /// Constructor. Explicitly initializes the matrix to diagonal form,
    /// with the \e i th element on the diagonal set to <c>v[i]</c>.
    explicit Matrix2f(const Vec2f& v) {
        SetDiagonal(v);
    }

    /// Constructor.  Initialize the matrix from a vector of vectors of
    /// double. The vector is expected to be 2x2. If it is
    /// too big, only the first 2 rows and/or columns will be used.
    /// If it is too small, uninitialized elements will be filled in with
    /// the corresponding elements from an identity matrix.
    ///
    GF_API
    explicit Matrix2f(const std::vector< std::vector<double> >& v);

    /// Constructor.  Initialize the matrix from a vector of vectors of
    /// float. The vector is expected to be 2x2. If it is
    /// too big, only the first 2 rows and/or columns will be used.
    /// If it is too small, uninitialized elements will be filled in with
    /// the corresponding elements from an identity matrix.
    ///
    GF_API
    explicit Matrix2f(const std::vector< std::vector<float> >& v);

    /// This explicit constructor converts a "double" matrix to a "float" matrix.
    GF_API
    explicit Matrix2f(const class Matrix2d& m);

    /// Sets a row of the matrix from a Vec2.
    void SetRow(int i, const Vec2f & v) {
        _mtx[i][0] = v[0];
        _mtx[i][1] = v[1];
    }

    /// Sets a column of the matrix from a Vec2.
    void SetColumn(int i, const Vec2f & v) {
        _mtx[0][i] = v[0];
        _mtx[1][i] = v[1];
    }

    /// Gets a row of the matrix as a Vec2.
    Vec2f GetRow(int i) const {
        return Vec2f(_mtx[i][0], _mtx[i][1]);
    }

    /// Gets a column of the matrix as a Vec2.
    Vec2f GetColumn(int i) const {
        return Vec2f(_mtx[0][i], _mtx[1][i]);
    }

    /// Sets the matrix from 4 independent \c float values,
    /// specified in row-major order. For example, parameter \e m10 specifies
    /// the value in row 1 and column 0.
    Matrix2f& Set(float m00, float m01, 
                    float m10, float m11) {
        _mtx[0][0] = m00; _mtx[0][1] = m01; 
        _mtx[1][0] = m10; _mtx[1][1] = m11;
        return *this;
    }

    /// Sets the matrix from a 2x2 array of \c float
    /// values, specified in row-major order.
    Matrix2f& Set(const float m[2][2]) {
        _mtx[0][0] = m[0][0];
        _mtx[0][1] = m[0][1];
        _mtx[1][0] = m[1][0];
        _mtx[1][1] = m[1][1];
        return *this;
    }

    /// Sets the matrix to the identity matrix.
    Matrix2f& SetIdentity() {
        return SetDiagonal(1);
    }

    /// Sets the matrix to zero.
    Matrix2f& SetZero() {
        return SetDiagonal(0);
    }

    /// Sets the matrix to \e s times the identity matrix.
    GF_API
    Matrix2f& SetDiagonal(float s);

    /// Sets the matrix to have diagonal (<c>v[0], v[1]</c>).
    GF_API
    Matrix2f& SetDiagonal(const Vec2f&);

    /// Fills a 2x2 array of \c float values with the values in
    /// the matrix, specified in row-major order.
    GF_API
    float* Get(float m[2][2]) const;

    /// Returns raw access to components of matrix as an array of
    /// \c float values.  Components are in row-major order.
    float* data() {
        return _mtx.GetData();
    }

    /// Returns const raw access to components of matrix as an array of
    /// \c float values.  Components are in row-major order.
    const float* data() const {
        return _mtx.GetData();
    }

    /// Returns vector components as an array of \c float values.
    float* GetArray()  {
        return _mtx.GetData();
    }

    /// Returns vector components as a const array of \c float values.
    const float* GetArray() const {
        return _mtx.GetData();
    }

    /// Accesses an indexed row \e i of the matrix as an array of 2 \c
    /// float values so that standard indexing (such as <c>m[0][1]</c>)
    /// works correctly.
    float* operator [](int i) { return _mtx[i]; }

    /// Accesses an indexed row \e i of the matrix as an array of 2 \c
    /// float values so that standard indexing (such as <c>m[0][1]</c>)
    /// works correctly.
    const float* operator [](int i) const { return _mtx[i]; }

    /// Hash.
    friend inline size_t hash_value(Matrix2f const &m) {
        return TfHash::Combine(
            m._mtx[0][0],
            m._mtx[0][1],
            m._mtx[1][0],
            m._mtx[1][1]
        );
    }

    /// Tests for element-wise matrix equality. All elements must match
    /// exactly for matrices to be considered equal.
    GF_API
    bool operator ==(const Matrix2d& m) const;

    /// Tests for element-wise matrix equality. All elements must match
    /// exactly for matrices to be considered equal.
    GF_API
    bool operator ==(const Matrix2f& m) const;

    /// Tests for element-wise matrix inequality. All elements must match
    /// exactly for matrices to be considered equal.
    bool operator !=(const Matrix2d& m) const {
        return !(*this == m);
    }

    /// Tests for element-wise matrix inequality. All elements must match
    /// exactly for matrices to be considered equal.
    bool operator !=(const Matrix2f& m) const {
        return !(*this == m);
    }

    /// Returns the transpose of the matrix.
    GF_API
    Matrix2f GetTranspose() const;

    /// Returns the inverse of the matrix, or FLT_MAX * SetIdentity() if the
    /// matrix is singular. (FLT_MAX is the largest value a \c float can have,
    /// as defined by the system.) The matrix is considered singular if the
    /// determinant is less than or equal to the optional parameter \e eps. If
    /// \e det is non-null, <c>*det</c> is set to the determinant.
    GF_API
    Matrix2f GetInverse(double* det = NULL, double eps = 0) const;

    /// Returns the determinant of the matrix.
    GF_API
    double GetDeterminant() const;


    /// Post-multiplies matrix \e m into this matrix.
    GF_API
    Matrix2f& operator *=(const Matrix2f& m);

    /// Multiplies the matrix by a float.
    GF_API
    Matrix2f& operator *=(double);

    /// Returns the product of a matrix and a float.
    friend Matrix2f operator *(const Matrix2f& m1, double d)
    {
        Matrix2f m = m1;
        return m *= d;
    }

    ///
    // Returns the product of a matrix and a float.
    friend Matrix2f operator *(double d, const Matrix2f& m)
    {
        return m * d;
    }

    /// Adds matrix \e m to this matrix.
    GF_API
    Matrix2f& operator +=(const Matrix2f& m);

    /// Subtracts matrix \e m from this matrix.
    GF_API
    Matrix2f& operator -=(const Matrix2f& m);

    /// Returns the unary negation of matrix \e m.
    GF_API
    friend Matrix2f operator -(const Matrix2f& m);

    /// Adds matrix \e m2 to \e m1
    friend Matrix2f operator +(const Matrix2f& m1, const Matrix2f& m2)
    {
        Matrix2f tmp(m1);
        tmp += m2;
        return tmp;
    }

    /// Subtracts matrix \e m2 from \e m1.
    friend Matrix2f operator -(const Matrix2f& m1, const Matrix2f& m2)
    {
        Matrix2f tmp(m1);
        tmp -= m2;
        return tmp;
    }

    /// Multiplies matrix \e m1 by \e m2.
    friend Matrix2f operator *(const Matrix2f& m1, const Matrix2f& m2)
    {
        Matrix2f tmp(m1);
        tmp *= m2;
        return tmp;
    }

    /// Divides matrix \e m1 by \e m2 (that is, <c>m1 * inv(m2)</c>).
    friend Matrix2f operator /(const Matrix2f& m1, const Matrix2f& m2)
    {
        return(m1 * m2.GetInverse());
    }

    /// Returns the product of a matrix \e m and a column vector \e vec.
    friend inline Vec2f operator *(const Matrix2f& m, const Vec2f& vec) {
        return Vec2f(vec[0] * m._mtx[0][0] + vec[1] * m._mtx[0][1],
                       vec[0] * m._mtx[1][0] + vec[1] * m._mtx[1][1]);
    }

    /// Returns the product of row vector \e vec and a matrix \e m.
    friend inline Vec2f operator *(const Vec2f &vec, const Matrix2f& m) {
        return Vec2f(vec[0] * m._mtx[0][0] + vec[1] * m._mtx[1][0],
                       vec[0] * m._mtx[0][1] + vec[1] * m._mtx[1][1]);
    }


private:
    /// Matrix storage, in row-major order.
    MatrixData<float, 2, 2> _mtx;

    // Friend declarations
    friend class Matrix2d;
};


/// Tests for equality within a given tolerance, returning \c true if the
/// difference between each component of the matrix is less than or equal
/// to \p tolerance, or false otherwise.
GF_API 
bool IsClose(Matrix2f const &m1, Matrix2f const &m2, double tolerance);

/// Output a Matrix2f
/// \ingroup group_gf_DebuggingOutput
GF_API std::ostream& operator<<(std::ostream &, Matrix2f const &);

PXR_NAMESPACE_CLOSE_SCOPE

#endif // PXR_BASE_GF_MATRIX2F_H
