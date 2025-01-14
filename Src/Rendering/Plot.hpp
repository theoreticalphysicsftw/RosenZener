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
#include <OS/Time.hpp>
#include <OS/IO.hpp>


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
    static constexpr U32 consecutiveSubdivsTreshold = 16;
    static constexpr T flatnessThreshold = 0.998;
    
    U32 initialTesselation = dest.width;

    auto ts = GetTimeStampUS<F64>();

    Deque<Vector<T, 2>> initValues;
    auto fRange = fRangeMax - fRangeMin;

    auto Project = [&](const Vector<T, 2>& v) -> Vector<T, 2>
    {
        return RemapToRange
        (
            v,
            fRangeMin,
            fRangeMax,
            drawRangeMin,
            drawRangeMax
        );
    };

    for (auto i = 0u; i < initialTesselation; ++i)
    {
        auto t = fRangeMin[0] + i / T(initialTesselation - 1) * fRange[0];
        auto ft = func(t);
        initValues.EmplaceBack(Vector<T, 2>(t, ft));
    }

    Array<Vector<T, 2>> values;
    values.EmplaceBack(initValues.GetFront());
    initValues.PopFront();

    U32 consecutiveSubdivs = 0;
    
    // Keep tessellating or push the current tree points.
    auto SplitOrPush = [&](auto& p0, auto& p1, auto& p2, Bool pushBoth)
    {
        auto dir0 = Normalized(p1 - p0);
        auto dir1 = Normalized(p2 - p0);
        auto cosA = dir0.Dot(dir1);
        
        // The angle is not close enough to 0 and we've not reached
        // the max subdivisions.
        if (cosA < flatnessThreshold && consecutiveSubdivs < consecutiveSubdivsTreshold)
        {
            auto p15t = (p1[0] + p2[0]) / 2;
            initValues.EmplaceFront(p2);
            initValues.EmplaceFront(p15t, func(p15t));
            if (pushBoth)
            {
                initValues.EmplaceFront(p1);
                auto p05t = (p0[0] + p1[0]) / 2;
                initValues.EmplaceFront(p05t, func(p05t));
            }
            consecutiveSubdivs++;
        }
        else
        {
            if (pushBoth)
            {
                values.EmplaceBack(p1);
            }
            values.EmplaceBack(p2);
            consecutiveSubdivs = 0;
        }
    };
    
    while (!initValues.empty())
    {
        if (values.GetSize() < 2)
        {
            auto p0 = values.GetBack();
            auto p1 = initValues.GetFront();
            initValues.PopFront();
            auto p2 = initValues.GetFront();
            initValues.PopFront();
            SplitOrPush(p0, p1, p2, true);
        }
        else
        {
            auto p0 = values.GetBeforeBack();
            auto p1 = values.GetBack();
            auto p2 = initValues.GetFront();
            initValues.PopFront();
            SplitOrPush(p0, p1, p2, false);
        }
    }

    for (auto i = 0u; i < values.GetSize(); ++i)
    {
        values[i] = Project(values[i]);
    }

    auto te = GetTimeStampUS<F64>();
    DebugLog(__func__, ": tessellation of ", values.GetSize(), " done in ",te - ts, "us"); 
    
    ts = GetTimeStampUS<F64>();
    for (auto i = 0u; i < values.GetSize() - 1; ++i)
    {
        DrawLine(dest, *reinterpret_cast<Line<T, 2>*>(values.GetData() + i), color, widthPixel, true);
    }
    te = GetTimeStampUS<F64>();
    DebugLog(__func__, ": rendering done in ",te - ts, "us"); 
}


template <typename T, typename TFunc>
inline auto SmoothParametricPlot2D
(
    RawCPUImage& dest,
    const Vector<T, 2>& drawRangeMin,
    const Vector<T, 2>& drawRangeMax,
    const Vector<T, 3>& fRangeMin,
    const Vector<T, 3>& fRangeMax,
    const TFunc&& func,
    const Color4& color,
    T widthPixel
) -> Void
{
    static constexpr U32 consecutiveSubdivsTreshold = 32;
    static constexpr T flatnessThreshold = 0.99998;
    
    U32 initialTesselation = dest.width;

    auto ts = GetTimeStampUS<F64>();

    Deque<Vector<T, 3>> initValues;
    auto fRange = fRangeMax - fRangeMin;

    auto Project = [&](const Vector<T, 3>& v) -> Vector<T, 2>
    {
        return RemapToRange
        (
            v.template GetSubSpaceProjection<1, 2>(),
            fRangeMin.template GetSubSpaceProjection<1, 2>(),
            fRangeMax.template GetSubSpaceProjection<1, 2>(),
            drawRangeMin,
            drawRangeMax
        );
    };

    for (auto i = 0u; i < initialTesselation; ++i)
    {
        auto t = fRangeMin[0] + i / T(initialTesselation - 1) * fRange[0];
        auto ft = func(t);
        initValues.EmplaceBack(Vector<T, 3>(t, ft[0], ft[1]));
    }

    Array<Vector<T, 3>> values;
    values.EmplaceBack(initValues.GetFront());
    initValues.PopFront();

    U32 consecutiveSubdivs = 0;
    
    // Keep tessellating or push the current tree points.
    auto SplitOrPush = [&](auto& p0, auto& p1, auto& p2, Bool pushBoth)
    {
        auto pp0 = p0.template GetSubSpaceProjection<1, 2>();
        auto pp1 = p1.template GetSubSpaceProjection<1, 2>();
        auto pp2 = p2.template GetSubSpaceProjection<1, 2>();
        auto dir0 = Normalized(pp1 - pp0);
        auto dir1 = Normalized(pp2 - pp0);
        auto cosA = dir0.Dot(dir1);
        
        // The angle is not close enough to 0 and we've not reached
        // the max subdivisions.
        if (cosA < flatnessThreshold && consecutiveSubdivs < consecutiveSubdivsTreshold)
        {
            auto p15t = (p1[0] + p2[0]) / 2;
            initValues.EmplaceFront(p2);
            auto fp15 = func(p15t);
            initValues.EmplaceFront(p15t, fp15[0], fp15[1]);
            if (pushBoth)
            {
                initValues.EmplaceFront(p1);
                auto p05t = (p0[0] + p1[0]) / 2;
                auto fp05 = func(p05t);
                initValues.EmplaceFront(p05t, fp05[0], fp05[1]);
            }
            consecutiveSubdivs++;
        }
        else
        {
            if (pushBoth)
            {
                values.EmplaceBack(p1);
            }
            values.EmplaceBack(p2);
            consecutiveSubdivs = 0;
        }
    };
    
    while (!initValues.empty())
    {
        if (values.GetSize() < 2)
        {
            auto p0 = values.GetBack();
            auto p1 = initValues.GetFront();
            initValues.PopFront();
            auto p2 = initValues.GetFront();
            initValues.PopFront();
            SplitOrPush(p0, p1, p2, true);
        }
        else
        {
            auto p0 = values.GetBeforeBack();
            auto p1 = values.GetBack();
            auto p2 = initValues.GetFront();
            initValues.PopFront();
            SplitOrPush(p0, p1, p2, false);
        }
    }

    Array<Vector<T, 2>> projectedValues(values.GetSize());
    for (auto i = 0u; i < values.GetSize(); ++i)
    {
        projectedValues.EmplaceBack(Project(values[i]));
    }

    auto te = GetTimeStampUS<F64>();
    DebugLog(__func__, ": tessellation of ", values.GetSize(), " done in ",te - ts, "us"); 
    
    ts = GetTimeStampUS<F64>();
    for (auto i = 0u; i < values.GetSize() - 1; ++i)
    {
        DrawLine(dest, *reinterpret_cast<Line<T, 2>*>(projectedValues.GetData() + i), color, widthPixel, true);
    }
    te = GetTimeStampUS<F64>();
    DebugLog(__func__, ": rendering done in ",te - ts, "us"); 
}
