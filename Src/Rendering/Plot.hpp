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

#include <Core.hpp>
#include <Math/Algebra.hpp>
#include <Image/Color.hpp>
#include <Image/RawCPUImage.hpp>
#include <Rendering/Point.hpp>

template <typename T>
inline auto DiscretePointPlot2D
(
    RawCPUImage& dest,
    const Vector<T, 2>& drawRangeMin,
    const Vector<T, 2>& drawRangeMax,
    const Vector<T, 2>& fRangeMin,
    const Vector<T, 2>& fRangeMax,
    T fVal,
    T argVal,
    const Color4& color,
    T radiusPixel
)
{
    Vector<T, 2> f(argVal, fVal);
    auto projected = RemapToRange(f, drawRangeMin, drawRangeMax, fRangeMin, fRangeMax);
    DrawPoint(dest, projected, color, radiusPixel);
}
