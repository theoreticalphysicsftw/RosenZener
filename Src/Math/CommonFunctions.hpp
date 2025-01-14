// MIT License
// 
// Copyright (c) 2024 Mihail Mladenov
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


#pragma once

#include <Math/Algebra/Complex.hpp>
#include <Math/Constants.hpp>

#include <cmath>


template <typename T>
inline auto Sin(const T& x) -> T
{
    return std::sin(x);
}

template <typename T>
inline auto Cos(const T& x) -> T
{
    return std::cos(x);
}

template <typename T>
inline auto Sech(const T& x) -> T
{
    return T(1) / std::cosh(x);
}

template <typename T>
inline auto Tanh(const T& x) -> T
{
    return std::tanh(x);
}

template <typename T>
inline auto Ceil(const T& x) -> T
{
    return std::ceil(x);
}

template <typename T>
inline auto Floor(const T & x) -> T
{
    return std::floor(x);
}

template <typename T>
inline auto Abs(const T& x) -> T
{
    return std::abs(x);
}

template <typename T>
inline auto Sqrt(const T& x) -> T
{
    return std::sqrt(x);
}

template <typename T>
inline auto Pow(const T& x, T n) -> T
{
    return std::pow(x, n);
}

template <typename T>
inline auto Exp(const T& x) -> T
{
    return std::exp(x);
}

template <typename T>
inline auto Ln(const T& x) -> T
{
    return std::log(x);
}

template <typename T>
inline auto AreClose(T a, T b, T eps = 0.000001) -> Bool
{
    return Abs(a - b) < eps;
}

template <typename T>
inline auto LogGamma(const Complex<T>& z) -> Complex<T>
{
    static constexpr U32 maxIterations = 16;
    auto r = -Complex<T>(Constants<T>::EulerMascheroni) * z - Ln(z);

    for (auto i = 1u; i <= maxIterations; ++i)
    {
        auto zi = z / Complex<T>(i);
        r += zi - Ln(Complex<T>(1) + zi);
    }

    return r;
}

template <typename T>
inline auto LogPochhammer(const Complex<T>& z, const Complex<T>& n) -> Complex<T>
{
    return LogGamma(z + n) - LogGamma(z);
}

template <typename T>
inline auto GaussHypergeometric(const Complex<T>& a, const Complex<T>& b, const Complex<T>& c, const Complex<T>& z) -> Complex<T>
{
    static constexpr U32 maxIterations = 64;
    auto term = a * b / c * z;
    auto sum = Complex<T>(1) + term;

    for (auto i = 2; i <= maxIterations; ++i)
    {
        auto cim1 = Complex<T>(i - 1);
        term *= (a + cim1) * (b + cim1) / (c + cim1) * z / Complex<T>(i);
        sum += term;
    }

    return sum;
}
