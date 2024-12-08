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
// matrix3.template.cpp file to make changes.


#include "pxr/pxr.h"
#include "pxr/base/gf/matrix3d.h"
#include "pxr/base/gf/matrix3f.h"

#include "pxr/base/gf/math.h"
#include "pxr/base/gf/ostreamHelpers.h"
#include "pxr/base/tf/type.h"

#include "pxr/base/gf/quatf.h"
#include "pxr/base/gf/rotation.h"
#include <float.h>
#include <ostream>

PXR_NAMESPACE_OPEN_SCOPE

TF_REGISTRY_FUNCTION(TfType) {
    TfType::Define<Matrix3f>();
}

std::ostream&
operator<<(std::ostream& out, const Matrix3f& m)
{
    return out
        << "( ("
        << _OstreamHelperP(m[0][0]) << ", "
        << _OstreamHelperP(m[0][1]) << ", "
        << _OstreamHelperP(m[0][2])
        << "), ("
        << _OstreamHelperP(m[1][0]) << ", "
        << _OstreamHelperP(m[1][1]) << ", "
        << _OstreamHelperP(m[1][2])
        << "), ("
        << _OstreamHelperP(m[2][0]) << ", "
        << _OstreamHelperP(m[2][1]) << ", "
        << _OstreamHelperP(m[2][2])
        << ") )";
}

Matrix3f::Matrix3f(const Matrix3d& m)
{
    Set(m[0][0], m[0][1], m[0][2], 
        m[1][0], m[1][1], m[1][2], 
        m[2][0], m[2][1], m[2][2]);
}

Matrix3f::Matrix3f(const std::vector< std::vector<double> >& v)
{
    float m[3][3] = {{1.0, 0.0, 0.0},
                      {0.0, 1.0, 0.0},
                      {0.0, 0.0, 1.0}};
    for(size_t row = 0; row < 3 && row < v.size(); ++row) {
        for (size_t col = 0; col < 3 && col < v[row].size(); ++col) {
            m[row][col] = v[row][col];
        }
    }
    Set(m);
}

Matrix3f::Matrix3f(const std::vector< std::vector<float> >& v)
{
    float m[3][3] = {{1.0, 0.0, 0.0},
                      {0.0, 1.0, 0.0},
                      {0.0, 0.0, 1.0}};
    for(size_t row = 0; row < 3 && row < v.size(); ++row) {
        for (size_t col = 0; col < 3 && col < v[row].size(); ++col) {
            m[row][col] = v[row][col];
        }
    }
    Set(m);
}

Matrix3f::Matrix3f(const Rotation &rot)
{
    SetRotate(rot);
}

Matrix3f::Matrix3f(const Quatf &rot)
{
    SetRotate(rot);
}

Matrix3f &
Matrix3f::SetDiagonal(float s)
{
    _mtx[0][0] = s;
    _mtx[0][1] = 0.0;
    _mtx[0][2] = 0.0;
    _mtx[1][0] = 0.0;
    _mtx[1][1] = s;
    _mtx[1][2] = 0.0;
    _mtx[2][0] = 0.0;
    _mtx[2][1] = 0.0;
    _mtx[2][2] = s;
    return *this;
}

Matrix3f &
Matrix3f::SetDiagonal(const Vec3f& v)
{
    _mtx[0][0] = v[0]; _mtx[0][1] = 0.0; _mtx[0][2] = 0.0; 
    _mtx[1][0] = 0.0; _mtx[1][1] = v[1]; _mtx[1][2] = 0.0; 
    _mtx[2][0] = 0.0; _mtx[2][1] = 0.0; _mtx[2][2] = v[2];
    return *this;
}

float *
Matrix3f::Get(float m[3][3]) const
{
    m[0][0] = _mtx[0][0];
    m[0][1] = _mtx[0][1];
    m[0][2] = _mtx[0][2];
    m[1][0] = _mtx[1][0];
    m[1][1] = _mtx[1][1];
    m[1][2] = _mtx[1][2];
    m[2][0] = _mtx[2][0];
    m[2][1] = _mtx[2][1];
    m[2][2] = _mtx[2][2];
    return &m[0][0];
}

bool
Matrix3f::operator ==(const Matrix3d &m) const
{
    return (_mtx[0][0] == m._mtx[0][0] &&
            _mtx[0][1] == m._mtx[0][1] &&
            _mtx[0][2] == m._mtx[0][2] &&
            _mtx[1][0] == m._mtx[1][0] &&
            _mtx[1][1] == m._mtx[1][1] &&
            _mtx[1][2] == m._mtx[1][2] &&
            _mtx[2][0] == m._mtx[2][0] &&
            _mtx[2][1] == m._mtx[2][1] &&
            _mtx[2][2] == m._mtx[2][2]);
}

bool
Matrix3f::operator ==(const Matrix3f &m) const
{
    return (_mtx[0][0] == m._mtx[0][0] &&
            _mtx[0][1] == m._mtx[0][1] &&
            _mtx[0][2] == m._mtx[0][2] &&
            _mtx[1][0] == m._mtx[1][0] &&
            _mtx[1][1] == m._mtx[1][1] &&
            _mtx[1][2] == m._mtx[1][2] &&
            _mtx[2][0] == m._mtx[2][0] &&
            _mtx[2][1] == m._mtx[2][1] &&
            _mtx[2][2] == m._mtx[2][2]);
}


Matrix3f
Matrix3f::GetTranspose() const
{
    Matrix3f transpose;
    transpose._mtx[0][0] = _mtx[0][0];
    transpose._mtx[1][0] = _mtx[0][1];
    transpose._mtx[2][0] = _mtx[0][2];
    transpose._mtx[0][1] = _mtx[1][0];
    transpose._mtx[1][1] = _mtx[1][1];
    transpose._mtx[2][1] = _mtx[1][2];
    transpose._mtx[0][2] = _mtx[2][0];
    transpose._mtx[1][2] = _mtx[2][1];
    transpose._mtx[2][2] = _mtx[2][2];

    return transpose;
}

Matrix3f
Matrix3f::GetInverse(double *detPtr, double eps) const
{
    double a00,a01,a02,a10,a11,a12,a20,a21,a22;
    double det, rcp;

    a00 = _mtx[0][0];
    a01 = _mtx[0][1];
    a02 = _mtx[0][2];
    a10 = _mtx[1][0];
    a11 = _mtx[1][1];
    a12 = _mtx[1][2];
    a20 = _mtx[2][0];
    a21 = _mtx[2][1];
    a22 = _mtx[2][2];
    det = -(a02*a11*a20) + a01*a12*a20 + a02*a10*a21 - 
	  a00*a12*a21 - a01*a10*a22 + a00*a11*a22;

    if (detPtr) {
	*detPtr = det;
    }

    Matrix3f inverse;

    if (Abs(det) > eps) {
	rcp = 1.0 / det;
        inverse._mtx[0][0] = static_cast<float>((-(a12*a21) + a11*a22)*rcp);
        inverse._mtx[0][1] = static_cast<float>((a02*a21 - a01*a22)*rcp);
        inverse._mtx[0][2] = static_cast<float>((-(a02*a11) + a01*a12)*rcp);
        inverse._mtx[1][0] = static_cast<float>((a12*a20 - a10*a22)*rcp);
        inverse._mtx[1][1] = static_cast<float>((-(a02*a20) + a00*a22)*rcp);
        inverse._mtx[1][2] = static_cast<float>((a02*a10 - a00*a12)*rcp);
        inverse._mtx[2][0] = static_cast<float>((-(a11*a20) + a10*a21)*rcp);
        inverse._mtx[2][1] = static_cast<float>((a01*a20 - a00*a21)*rcp);
        inverse._mtx[2][2] = static_cast<float>((-(a01*a10) + a00*a11)*rcp);

    }
    else {
       	inverse.SetScale(FLT_MAX);
    }

    return inverse;
}

double
Matrix3f::GetDeterminant() const
{
    return (_mtx[0][0] * _mtx[1][1] * _mtx[2][2] +
	    _mtx[0][1] * _mtx[1][2] * _mtx[2][0] +
	    _mtx[0][2] * _mtx[1][0] * _mtx[2][1] -
	    _mtx[0][0] * _mtx[1][2] * _mtx[2][1] -
	    _mtx[0][1] * _mtx[1][0] * _mtx[2][2] -
	    _mtx[0][2] * _mtx[1][1] * _mtx[2][0]);
}

double
Matrix3f::GetHandedness() const
{
    // Note: This can be computed with fewer arithmetic operations using a
    //       cross and dot product, but it is more important that the result
    //       is consistent with the way the determinant is computed.
    return Sgn(GetDeterminant());
}

/* Make the matrix orthonormal in place using an iterative method.
 * It is potentially slower if the matrix is far from orthonormal (i.e. if
 * the row basis vectors are close to colinear) but in the common case
 * of near-orthonormality it should be just as fast. */
bool
Matrix3f::Orthonormalize(bool issueWarning)
{
    // orthogonalize and normalize row vectors
    Vec3d r0(_mtx[0][0],_mtx[0][1],_mtx[0][2]);
    Vec3d r1(_mtx[1][0],_mtx[1][1],_mtx[1][2]);
    Vec3d r2(_mtx[2][0],_mtx[2][1],_mtx[2][2]);
    bool result = Vec3d::OrthogonalizeBasis(&r0, &r1, &r2, true);
    _mtx[0][0] = r0[0];
    _mtx[0][1] = r0[1];
    _mtx[0][2] = r0[2];
    _mtx[1][0] = r1[0];
    _mtx[1][1] = r1[1];
    _mtx[1][2] = r1[2];
    _mtx[2][0] = r2[0];
    _mtx[2][1] = r2[1];
    _mtx[2][2] = r2[2];
    if (!result && issueWarning)
	TF_WARN("OrthogonalizeBasis did not converge, matrix may not be "
                "orthonormal.");
    return result;
}

Matrix3f
Matrix3f::GetOrthonormalized(bool issueWarning) const
{
    Matrix3f result = *this;
    result.Orthonormalize(issueWarning);
    return result;
}

/*
** Scaling
*/
Matrix3f&
Matrix3f::operator*=(double d)
{
    _mtx[0][0] *= d; _mtx[0][1] *= d; _mtx[0][2] *= d; 
    _mtx[1][0] *= d; _mtx[1][1] *= d; _mtx[1][2] *= d; 
    _mtx[2][0] *= d; _mtx[2][1] *= d; _mtx[2][2] *= d;
    return *this;
}

/*
** Addition
*/
Matrix3f &
Matrix3f::operator+=(const Matrix3f &m)
{
    _mtx[0][0] += m._mtx[0][0];
    _mtx[0][1] += m._mtx[0][1];
    _mtx[0][2] += m._mtx[0][2];
    _mtx[1][0] += m._mtx[1][0];
    _mtx[1][1] += m._mtx[1][1];
    _mtx[1][2] += m._mtx[1][2];
    _mtx[2][0] += m._mtx[2][0];
    _mtx[2][1] += m._mtx[2][1];
    _mtx[2][2] += m._mtx[2][2];
    return *this;
}

/*
** Subtraction
*/
Matrix3f &
Matrix3f::operator-=(const Matrix3f &m)
{
    _mtx[0][0] -= m._mtx[0][0];
    _mtx[0][1] -= m._mtx[0][1];
    _mtx[0][2] -= m._mtx[0][2];
    _mtx[1][0] -= m._mtx[1][0];
    _mtx[1][1] -= m._mtx[1][1];
    _mtx[1][2] -= m._mtx[1][2];
    _mtx[2][0] -= m._mtx[2][0];
    _mtx[2][1] -= m._mtx[2][1];
    _mtx[2][2] -= m._mtx[2][2];
    return *this;
}

/*
** Negation
*/
Matrix3f
operator -(const Matrix3f& m)
{
    return
        Matrix3f(-m._mtx[0][0], -m._mtx[0][1], -m._mtx[0][2], 
                   -m._mtx[1][0], -m._mtx[1][1], -m._mtx[1][2], 
                   -m._mtx[2][0], -m._mtx[2][1], -m._mtx[2][2]);
}

Matrix3f &
Matrix3f::operator*=(const Matrix3f &m)
{
    // Save current values before they are overwritten
    Matrix3f tmp = *this;

    _mtx[0][0] = tmp._mtx[0][0] * m._mtx[0][0] +
                 tmp._mtx[0][1] * m._mtx[1][0] +
                 tmp._mtx[0][2] * m._mtx[2][0];

    _mtx[0][1] = tmp._mtx[0][0] * m._mtx[0][1] +
                 tmp._mtx[0][1] * m._mtx[1][1] +
                 tmp._mtx[0][2] * m._mtx[2][1];

    _mtx[0][2] = tmp._mtx[0][0] * m._mtx[0][2] +
                 tmp._mtx[0][1] * m._mtx[1][2] +
                 tmp._mtx[0][2] * m._mtx[2][2];

    _mtx[1][0] = tmp._mtx[1][0] * m._mtx[0][0] +
                 tmp._mtx[1][1] * m._mtx[1][0] +
                 tmp._mtx[1][2] * m._mtx[2][0];

    _mtx[1][1] = tmp._mtx[1][0] * m._mtx[0][1] +
                 tmp._mtx[1][1] * m._mtx[1][1] +
                 tmp._mtx[1][2] * m._mtx[2][1];

    _mtx[1][2] = tmp._mtx[1][0] * m._mtx[0][2] +
                 tmp._mtx[1][1] * m._mtx[1][2] +
                 tmp._mtx[1][2] * m._mtx[2][2];

    _mtx[2][0] = tmp._mtx[2][0] * m._mtx[0][0] +
                 tmp._mtx[2][1] * m._mtx[1][0] +
                 tmp._mtx[2][2] * m._mtx[2][0];

    _mtx[2][1] = tmp._mtx[2][0] * m._mtx[0][1] +
                 tmp._mtx[2][1] * m._mtx[1][1] +
                 tmp._mtx[2][2] * m._mtx[2][1];

    _mtx[2][2] = tmp._mtx[2][0] * m._mtx[0][2] +
                 tmp._mtx[2][1] * m._mtx[1][2] +
                 tmp._mtx[2][2] * m._mtx[2][2];

    return *this;
}
Matrix3f &
Matrix3f::SetScale(float s)
{
    _mtx[0][0] = s;   _mtx[0][1] = 0.0; _mtx[0][2] = 0.0;
    _mtx[1][0] = 0.0; _mtx[1][1] = s;   _mtx[1][2] = 0.0;
    _mtx[2][0] = 0.0; _mtx[2][1] = 0.0; _mtx[2][2] = s;

    return *this;
}

Matrix3f &
Matrix3f::SetRotate(const Quatf &rot)
{
    _SetRotateFromQuat(rot.GetReal(), rot.GetImaginary());
    return *this;
}

Matrix3f &
Matrix3f::SetRotate(const Rotation &rot)
{
    Quaternion quat = rot.GetQuaternion();
    _SetRotateFromQuat(quat.GetReal(), Vec3f(quat.GetImaginary()));
    return *this;
}

void
Matrix3f::_SetRotateFromQuat(float r, const Vec3f& i)
{
    _mtx[0][0] = 1.0 - 2.0 * (i[1] * i[1] + i[2] * i[2]);
    _mtx[0][1] =       2.0 * (i[0] * i[1] + i[2] *    r);
    _mtx[0][2] =       2.0 * (i[2] * i[0] - i[1] *    r);

    _mtx[1][0] =       2.0 * (i[0] * i[1] - i[2] *    r);
    _mtx[1][1] = 1.0 - 2.0 * (i[2] * i[2] + i[0] * i[0]);
    _mtx[1][2] =       2.0 * (i[1] * i[2] + i[0] *    r);

    _mtx[2][0] =       2.0 * (i[2] * i[0] + i[1] *    r);
    _mtx[2][1] =       2.0 * (i[1] * i[2] - i[0] *    r);
    _mtx[2][2] = 1.0 - 2.0 * (i[1] * i[1] + i[0] * i[0]);
}
                            

Matrix3f &
Matrix3f::SetScale(const Vec3f &s)
{
    _mtx[0][0] = s[0]; _mtx[0][1] = 0.0;  _mtx[0][2] = 0.0;
    _mtx[1][0] = 0.0;  _mtx[1][1] = s[1]; _mtx[1][2] = 0.0;
    _mtx[2][0] = 0.0;  _mtx[2][1] = 0.0;  _mtx[2][2] = s[2];

    return *this;
}

Quaternion
Matrix3f::ExtractRotationQuaternion() const
{
    // This was adapted from the (open source) Open Inventor
    // SbRotation::SetValue(const SbMatrix &m)

    int i;

    // First, find largest diagonal in matrix:
    if (_mtx[0][0] > _mtx[1][1])
	i = (_mtx[0][0] > _mtx[2][2] ? 0 : 2);
    else
	i = (_mtx[1][1] > _mtx[2][2] ? 1 : 2);

    Vec3d im;
    double  r;

    if (_mtx[0][0] + _mtx[1][1] + _mtx[2][2] > _mtx[i][i]) {
	r = 0.5 * sqrt(_mtx[0][0] + _mtx[1][1] +
		       _mtx[2][2] + 1);
	im.Set((_mtx[1][2] - _mtx[2][1]) / (4.0 * r),
	       (_mtx[2][0] - _mtx[0][2]) / (4.0 * r),
	       (_mtx[0][1] - _mtx[1][0]) / (4.0 * r));
    }
    else {
	int j = (i + 1) % 3;
	int k = (i + 2) % 3;
	double q = 0.5 * sqrt(_mtx[i][i] - _mtx[j][j] -
			      _mtx[k][k] + 1); 

	im[i] = q;
	im[j] = (_mtx[i][j] + _mtx[j][i]) / (4 * q);
	im[k] = (_mtx[k][i] + _mtx[i][k]) / (4 * q);
	r     = (_mtx[j][k] - _mtx[k][j]) / (4 * q);
    }

    return Quaternion(Clamp(r, -1.0, 1.0), im);
}

Rotation
Matrix3f::ExtractRotation() const
{
    return Rotation( ExtractRotationQuaternion() );
}

Vec3f
Matrix3f::DecomposeRotation(const Vec3f &axis0,
                             const Vec3f &axis1,
                             const Vec3f &axis2) const
{
    return Vec3f(ExtractRotation().Decompose(axis0, axis1, axis2));
}


bool
IsClose(Matrix3f const &m1, Matrix3f const &m2, double tolerance)
{
    for(size_t row = 0; row < 3; ++row) {
        for(size_t col = 0; col < 3; ++col) {
            if(!IsClose(m1[row][col], m2[row][col], tolerance))
                return false;
        }
    }
    return true;
}


PXR_NAMESPACE_CLOSE_SCOPE
