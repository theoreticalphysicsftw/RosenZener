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
// matrix3.template.h file to make changes.

#ifndef PXR_BASE_GF_MATRIX3D_H
#define PXR_BASE_GF_MATRIX3D_H

/// \file gf/matrix3d.h
/// \ingroup group_gf_LinearAlgebra

#include "pxr/pxr.h"
#include "pxr/base/gf/api.h"
#include "pxr/base/gf/declare.h"
#include "pxr/base/gf/matrixData.h"
#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/traits.h"
#include "pxr/base/tf/hash.h"

#include <iosfwd>
#include <vector>

PXR_NAMESPACE_OPEN_SCOPE

template <>
struct IsMatrix<class Matrix3d> { static const bool value = true; };

class Matrix3d;
class Matrix3f;
class Rotation;
class Quaternion;
class Quatd;

/// \class Matrix3d
/// \ingroup group_gf_LinearAlgebra
///
/// Stores a 3x3 matrix of \c double elements. A basic type.
///
/// Matrices are defined to be in row-major order, so <c>matrix[i][j]</c>
/// indexes the element in the \e i th row and the \e j th column.
///
/// <h3>3D Transformations</h3>
///
/// Three methods, SetRotate(), SetScale(), and ExtractRotation(), interpret
/// a Matrix3d as a 3D transformation. By convention, vectors are treated
/// primarily as row vectors, implying the following:
///
/// \li Transformation matrices are organized to deal with row
///        vectors, not column vectors.
/// \li Each of the Set() methods in this class completely rewrites the
///        matrix; for example, SetRotate() yields a matrix
///        which does nothing but rotate.
/// \li When multiplying two transformation matrices, the matrix
///        on the left applies a more local transformation to a row
///        vector. For example, if R represents a rotation
///        matrix and S represents a scale matrix, the
///        product R*S  will rotate a row vector, then scale
///        it.
class Matrix3d
{
public:
    typedef double ScalarType;

    static const size_t numRows = 3;
    static const size_t numColumns = 3;

    /// Default constructor. Leaves the matrix component values undefined.
    Matrix3d() = default;

    /// Constructor. Initializes the matrix from 9 independent
    /// \c double values, specified in row-major order. For example,
    /// parameter \e m10 specifies the value in row 1 and column 0.
    Matrix3d(double m00, double m01, double m02, 
               double m10, double m11, double m12, 
               double m20, double m21, double m22) {
        Set(m00, m01, m02, 
            m10, m11, m12, 
            m20, m21, m22);
    }

    /// Constructor. Initializes the matrix from a 3x3 array
    /// of \c double values, specified in row-major order.
    Matrix3d(const double m[3][3]) {
        Set(m);
    }

    /// Constructor. Explicitly initializes the matrix to \e s times the
    /// identity matrix.
    explicit Matrix3d(double s) {
        SetDiagonal(s);
    }

    /// This explicit constructor initializes the matrix to \p s times
    /// the identity matrix.
    explicit Matrix3d(int s) {
        SetDiagonal(s);
    }

    /// Constructor. Explicitly initializes the matrix to diagonal form,
    /// with the \e i th element on the diagonal set to <c>v[i]</c>.
    explicit Matrix3d(const Vec3d& v) {
        SetDiagonal(v);
    }

    /// Constructor.  Initialize the matrix from a vector of vectors of
    /// double. The vector is expected to be 3x3. If it is
    /// too big, only the first 3 rows and/or columns will be used.
    /// If it is too small, uninitialized elements will be filled in with
    /// the corresponding elements from an identity matrix.
    ///
    GF_API
    explicit Matrix3d(const std::vector< std::vector<double> >& v);

    /// Constructor.  Initialize the matrix from a vector of vectors of
    /// float. The vector is expected to be 3x3. If it is
    /// too big, only the first 3 rows and/or columns will be used.
    /// If it is too small, uninitialized elements will be filled in with
    /// the corresponding elements from an identity matrix.
    ///
    GF_API
    explicit Matrix3d(const std::vector< std::vector<float> >& v);

    /// Constructor. Initialize matrix from rotation.
    GF_API
    Matrix3d(const Rotation& rot);

    /// Constructor. Initialize matrix from a quaternion.
    GF_API
    explicit Matrix3d(const Quatd& rot);

    /// This explicit constructor converts a "float" matrix to a "double" matrix.
    GF_API
    explicit Matrix3d(const class Matrix3f& m);

    /// Sets a row of the matrix from a Vec3.
    void SetRow(int i, const Vec3d & v) {
        _mtx[i][0] = v[0];
        _mtx[i][1] = v[1];
        _mtx[i][2] = v[2];
    }

    /// Sets a column of the matrix from a Vec3.
    void SetColumn(int i, const Vec3d & v) {
        _mtx[0][i] = v[0];
        _mtx[1][i] = v[1];
        _mtx[2][i] = v[2];
    }

    /// Gets a row of the matrix as a Vec3.
    Vec3d GetRow(int i) const {
        return Vec3d(_mtx[i][0], _mtx[i][1], _mtx[i][2]);
    }

    /// Gets a column of the matrix as a Vec3.
    Vec3d GetColumn(int i) const {
        return Vec3d(_mtx[0][i], _mtx[1][i], _mtx[2][i]);
    }

    /// Sets the matrix from 9 independent \c double values,
    /// specified in row-major order. For example, parameter \e m10 specifies
    /// the value in row 1 and column 0.
    Matrix3d& Set(double m00, double m01, double m02, 
                    double m10, double m11, double m12, 
                    double m20, double m21, double m22) {
        _mtx[0][0] = m00; _mtx[0][1] = m01; _mtx[0][2] = m02; 
        _mtx[1][0] = m10; _mtx[1][1] = m11; _mtx[1][2] = m12; 
        _mtx[2][0] = m20; _mtx[2][1] = m21; _mtx[2][2] = m22;
        return *this;
    }

    /// Sets the matrix from a 3x3 array of \c double
    /// values, specified in row-major order.
    Matrix3d& Set(const double m[3][3]) {
        _mtx[0][0] = m[0][0];
        _mtx[0][1] = m[0][1];
        _mtx[0][2] = m[0][2];
        _mtx[1][0] = m[1][0];
        _mtx[1][1] = m[1][1];
        _mtx[1][2] = m[1][2];
        _mtx[2][0] = m[2][0];
        _mtx[2][1] = m[2][1];
        _mtx[2][2] = m[2][2];
        return *this;
    }

    /// Sets the matrix to the identity matrix.
    Matrix3d& SetIdentity() {
        return SetDiagonal(1);
    }

    /// Sets the matrix to zero.
    Matrix3d& SetZero() {
        return SetDiagonal(0);
    }

    /// Sets the matrix to \e s times the identity matrix.
    GF_API
    Matrix3d& SetDiagonal(double s);

    /// Sets the matrix to have diagonal (<c>v[0], v[1], v[2]</c>).
    GF_API
    Matrix3d& SetDiagonal(const Vec3d&);

    /// Fills a 3x3 array of \c double values with the values in
    /// the matrix, specified in row-major order.
    GF_API
    double* Get(double m[3][3]) const;

    /// Returns raw access to components of matrix as an array of
    /// \c double values.  Components are in row-major order.
    double* data() {
        return _mtx.GetData();
    }

    /// Returns const raw access to components of matrix as an array of
    /// \c double values.  Components are in row-major order.
    const double* data() const {
        return _mtx.GetData();
    }

    /// Returns vector components as an array of \c double values.
    double* GetArray()  {
        return _mtx.GetData();
    }

    /// Returns vector components as a const array of \c double values.
    const double* GetArray() const {
        return _mtx.GetData();
    }

    /// Accesses an indexed row \e i of the matrix as an array of 3 \c
    /// double values so that standard indexing (such as <c>m[0][1]</c>)
    /// works correctly.
    double* operator [](int i) { return _mtx[i]; }

    /// Accesses an indexed row \e i of the matrix as an array of 3 \c
    /// double values so that standard indexing (such as <c>m[0][1]</c>)
    /// works correctly.
    const double* operator [](int i) const { return _mtx[i]; }

    /// Hash.
    friend inline size_t hash_value(Matrix3d const &m) {
        return TfHash::Combine(
            m._mtx[0][0],
            m._mtx[0][1],
            m._mtx[0][2],
            m._mtx[1][0],
            m._mtx[1][1],
            m._mtx[1][2],
            m._mtx[2][0],
            m._mtx[2][1],
            m._mtx[2][2]
        );
    }

    /// Tests for element-wise matrix equality. All elements must match
    /// exactly for matrices to be considered equal.
    GF_API
    bool operator ==(const Matrix3d& m) const;

    /// Tests for element-wise matrix equality. All elements must match
    /// exactly for matrices to be considered equal.
    GF_API
    bool operator ==(const Matrix3f& m) const;

    /// Tests for element-wise matrix inequality. All elements must match
    /// exactly for matrices to be considered equal.
    bool operator !=(const Matrix3d& m) const {
        return !(*this == m);
    }

    /// Tests for element-wise matrix inequality. All elements must match
    /// exactly for matrices to be considered equal.
    bool operator !=(const Matrix3f& m) const {
        return !(*this == m);
    }

    /// Returns the transpose of the matrix.
    GF_API
    Matrix3d GetTranspose() const;

    /// Returns the inverse of the matrix, or FLT_MAX * SetIdentity() if the
    /// matrix is singular. (FLT_MAX is the largest value a \c float can have,
    /// as defined by the system.) The matrix is considered singular if the
    /// determinant is less than or equal to the optional parameter \e eps. If
    /// \e det is non-null, <c>*det</c> is set to the determinant.
    GF_API
    Matrix3d GetInverse(double* det = NULL, double eps = 0) const;

    /// Returns the determinant of the matrix.
    GF_API
    double GetDeterminant() const;

    /// Makes the matrix orthonormal in place. This is an iterative method that
    /// is much more stable than the previous cross/cross method.  If the
    /// iterative method does not converge, a warning is issued.
    ///
    /// Returns true if the iteration converged, false otherwise.  Leaves any
    /// translation part of the matrix unchanged.  If \a issueWarning is true,
    /// this method will issue a warning if the iteration does not converge,
    /// otherwise it will be silent.
    GF_API
    bool Orthonormalize(bool issueWarning=true);

    /// Returns an orthonormalized copy of the matrix.
    GF_API
    Matrix3d GetOrthonormalized(bool issueWarning=true) const;

    /// Returns the sign of the determinant of the matrix, i.e. 1 for a
    /// right-handed matrix, -1 for a left-handed matrix, and 0 for a
    /// singular matrix.
    GF_API
    double GetHandedness() const;

    /// Returns true if the vectors in the matrix form a right-handed
    /// coordinate system.
    bool IsRightHanded() const {
        return GetHandedness() == 1.0;
    }

    /// Returns true if the vectors in matrix form a left-handed
    /// coordinate system.
    bool IsLeftHanded() const {
        return GetHandedness() == -1.0;
    }

    /// Post-multiplies matrix \e m into this matrix.
    GF_API
    Matrix3d& operator *=(const Matrix3d& m);

    /// Multiplies the matrix by a double.
    GF_API
    Matrix3d& operator *=(double);

    /// Returns the product of a matrix and a double.
    friend Matrix3d operator *(const Matrix3d& m1, double d)
    {
        Matrix3d m = m1;
        return m *= d;
    }

    ///
    // Returns the product of a matrix and a double.
    friend Matrix3d operator *(double d, const Matrix3d& m)
    {
        return m * d;
    }

    /// Adds matrix \e m to this matrix.
    GF_API
    Matrix3d& operator +=(const Matrix3d& m);

    /// Subtracts matrix \e m from this matrix.
    GF_API
    Matrix3d& operator -=(const Matrix3d& m);

    /// Returns the unary negation of matrix \e m.
    GF_API
    friend Matrix3d operator -(const Matrix3d& m);

    /// Adds matrix \e m2 to \e m1
    friend Matrix3d operator +(const Matrix3d& m1, const Matrix3d& m2)
    {
        Matrix3d tmp(m1);
        tmp += m2;
        return tmp;
    }

    /// Subtracts matrix \e m2 from \e m1.
    friend Matrix3d operator -(const Matrix3d& m1, const Matrix3d& m2)
    {
        Matrix3d tmp(m1);
        tmp -= m2;
        return tmp;
    }

    /// Multiplies matrix \e m1 by \e m2.
    friend Matrix3d operator *(const Matrix3d& m1, const Matrix3d& m2)
    {
        Matrix3d tmp(m1);
        tmp *= m2;
        return tmp;
    }

    /// Divides matrix \e m1 by \e m2 (that is, <c>m1 * inv(m2)</c>).
    friend Matrix3d operator /(const Matrix3d& m1, const Matrix3d& m2)
    {
        return(m1 * m2.GetInverse());
    }

    /// Returns the product of a matrix \e m and a column vector \e vec.
    friend inline Vec3d operator *(const Matrix3d& m, const Vec3d& vec) {
        return Vec3d(vec[0] * m._mtx[0][0] + vec[1] * m._mtx[0][1] + vec[2] * m._mtx[0][2],
                       vec[0] * m._mtx[1][0] + vec[1] * m._mtx[1][1] + vec[2] * m._mtx[1][2],
                       vec[0] * m._mtx[2][0] + vec[1] * m._mtx[2][1] + vec[2] * m._mtx[2][2]);
    }

    /// Returns the product of row vector \e vec and a matrix \e m.
    friend inline Vec3d operator *(const Vec3d &vec, const Matrix3d& m) {
        return Vec3d(vec[0] * m._mtx[0][0] + vec[1] * m._mtx[1][0] + vec[2] * m._mtx[2][0],
                       vec[0] * m._mtx[0][1] + vec[1] * m._mtx[1][1] + vec[2] * m._mtx[2][1],
                       vec[0] * m._mtx[0][2] + vec[1] * m._mtx[1][2] + vec[2] * m._mtx[2][2]);
    }

    /// Sets matrix to specify a uniform scaling by \e scaleFactor.
    GF_API
    Matrix3d& SetScale(double scaleFactor);

    /// \name 3D Transformation Utilities
    /// @{

    /// Sets the matrix to specify a rotation equivalent to \e rot.
    GF_API
    Matrix3d& SetRotate(const Quatd &rot);

    /// Sets the matrix to specify a rotation equivalent to \e rot.
    GF_API
    Matrix3d& SetRotate(const Rotation &rot);

    /// Sets the matrix to specify a nonuniform scaling in x, y, and z by
    /// the factors in vector \e scaleFactors.
    GF_API
    Matrix3d& SetScale(const Vec3d &scaleFactors);

    /// Returns the rotation corresponding to this matrix. This works
    /// well only if the matrix represents a rotation.
    ///
    /// For good results, consider calling Orthonormalize() before calling
    /// this method.
    GF_API
    Rotation ExtractRotation() const;

    /// Decompose the rotation corresponding to this matrix about 3
    /// orthogonal axes.  If the axes are not orthogonal, warnings
    /// will be spewed.
    ///
    /// This is a convenience method that is equivalent to calling
    /// ExtractRotation().Decompose().
    GF_API
    Vec3d DecomposeRotation(const Vec3d &axis0,
                              const Vec3d &axis1,
                              const Vec3d &axis2 ) const;

    /// Returns the quaternion corresponding to this matrix. This works
    /// well only if the matrix represents a rotation.
    ///
    /// For good results, consider calling Orthonormalize() before calling
    /// this method.
    GF_API
    Quaternion ExtractRotationQuaternion() const;

    /// @}

private:
    /// Set the matrix to the rotation given by a quaternion,
    /// defined by the real component \p r and imaginary components \p i.
    void _SetRotateFromQuat(double r, const Vec3d& i);


private:
    /// Matrix storage, in row-major order.
    MatrixData<double, 3, 3> _mtx;

    // Friend declarations
    friend class Matrix3f;
};


/// Tests for equality within a given tolerance, returning \c true if the
/// difference between each component of the matrix is less than or equal
/// to \p tolerance, or false otherwise.
GF_API 
bool IsClose(Matrix3d const &m1, Matrix3d const &m2, double tolerance);

/// Output a Matrix3d
/// \ingroup group_gf_DebuggingOutput
GF_API std::ostream& operator<<(std::ostream &, Matrix3d const &);

PXR_NAMESPACE_CLOSE_SCOPE

#endif // PXR_BASE_GF_MATRIX3D_H
