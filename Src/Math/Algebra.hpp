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

#include <Math/Algebra/Vector.hpp>
#include <Math/Algebra/Matrix.hpp>
#include <Math/Algebra/Complex.hpp>

#include <Core/TypeTraits.hpp>
#include <Core/Concepts.hpp>


template <typename T>
struct VectorTraits {};

template <typename T>
	requires CIsFloatingPoint<T>
struct VectorTraits<T>
{
	using Scalar = T;
};

template <typename T>
	requires CIsFloatingPoint<T>
struct VectorTraits<Complex<T>>
{
	using Scalar = Complex<T>;
};

template <typename T, U32 N>
struct VectorTraits<Vector<T, N>>
{
	using Scalar = T;
};

template <typename T, U32 R, U32 C>
struct VectorTraits<Matrix<T, R, C>>
{
	using Scalar = T;
};
