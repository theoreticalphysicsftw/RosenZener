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
#include <Rendering/Line.hpp>


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
    auto projected = RemapToRange(f, fRangeMin, fRangeMax, drawRangeMin, drawRangeMax);
    DrawPoint(dest, projected, color, radiusPixel);
}


template <typename T, typename TFunc>
inline auto SmoothPlot2D
(
    RawCPUImage& dest,
    const Vector<T, 2>& drawRangeMin,
    const Vector<T, 2>& drawRangeMax,
    const Vector<T, 2>& fRangeMin,
    const Vector<T, 2>& fRangeMax,
    const TFunc&& func,
    const Color4& color,
    T widthPixel
) -> Void
{

    static constexpr U32 initialTesselation = 256;
    static constexpr U32 consecutiveSubdivsTreshold = 32;
    static constexpr F32 flatnessThreshold = 0.998f;

    Deque<Vector<T, 2>> initValues;
    auto fRange = fRangeMax - fRangeMin;
    
    for (auto i = 0u; i < initialTesselation; ++i)
    {
        auto t = fRangeMin[0] + i / T(initialTesselation) * fRange[0];
        auto ft = func(t);
        auto projected = RemapToRange
        (
            Vector<T, 2>(t, ft),
            fRangeMin,
            fRangeMax,
            drawRangeMin,
            drawRangeMax
        );
        initValues.EmplaceBack(projected);
    }

    Array<Vector<T, 2>> values;
    values.EmplaceBack(initValues.GetFront());
    initValues.PopFront();

    U32 consecutiveSubdivs = 0;
    
    while (!initValues.empty())
    {
        if (initValues.GetSize() < 2)
        {
            values.EmplaceBack(initValues.GetFront());
            initValues.PopFront();
        }
        else
        {
            auto& p0 = values.GetBack();
            auto p1 = initValues.GetFront();
            initValues.PopFront();
            auto p2 = initValues.GetFront();
            initValues.PopFront();

            auto dir0 = Normalized(p1 - p0);
            auto dir1 = Normalized(p2 - p0);
            auto cosA = dir0.Dot(dir1);
            
            // The angle is not close enough to 0 and we've not reached
            // the max subdivisions.
            if (cosA < flatnessThreshold && consecutiveSubdivs < consecutiveSubdivsTreshold)
            {
                auto p05t = (p0[0] + p1[0]) / 2;
                auto p15t = (p1[0] + p2[0]) / 2;
                initValues.EmplaceFront(p2);
                initValues.EmplaceFront(p15t, func(p15t));
                initValues.EmplaceFront(p1);
                initValues.EmplaceFront(p05t, func(p05t));
                consecutiveSubdivs++;
            }
            else
            {
                values.EmplaceBack(p1);
                values.EmplaceBack(p2);
                consecutiveSubdivs = 0;
            }
        }
    }
    
    for (auto i = 0u; i < values.GetSize() - 1; ++i)
    {
        DrawLine(dest, *reinterpret_cast<Line<T, 2>*>(values.GetData() + i), color, widthPixel);
    }
}
