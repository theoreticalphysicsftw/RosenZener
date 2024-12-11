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

#include <Core/Primitives.hpp>
#include <Math/Discrete/Bijections.hpp>
#include <Math/Discrete/BitsAndIntegers.hpp>


template <typename T>
auto Min(const T& i0, const T& i1) -> T
{
    return i0 < i1? i0 : i1;
}


template <typename T>
auto Max(const T& i0, const T& i1) -> T
{
    return i0 < i1? i1 : i0;
}


template <typename T>
auto Clamp(const T& i, const T& r0, const T& r1) -> T
{
    return Max(Min(i, r0), r1);
}


template <typename T>
auto GetClampedU8(const T& i) -> U8 
{
    return Clamp(i, T(0), T(255));
}
