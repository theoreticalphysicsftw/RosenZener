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
#include <Image/RawCPUImage.hpp>
#include <Image/Color.hpp>
#include <Math/Algebra/Vector.hpp>
#include <Math/Interpolation/Basic.hpp>

inline auto DrawPoint(RawCPUImage& dest, Vector2 normCoords, const Color4& color, F32 pixelSize) -> Void
{
    auto surfCoords = dest.ToSurfaceCoordinates(normCoords);
    auto radius = Ceil(pixelSize / 2) + 1;
    auto diameter = 2 * radius;

    ColorU32* surfData = (ColorU32*)dest.data.GetData();
    for (auto i = 0u; i < diameter; ++i)
    {
        auto pix = surfCoords + i - radius;
        auto uPix = Vector<U32, 2>(pix);
        if (dest.AreCoordsValid(uPix))
        {
            auto idx = LebesgueCurve(uPix[0], uPix[1]); 
            auto dist = Distance(surfCoords, pix);
            auto alpha = 1.f - SmoothStep(0.f, 2.f, dist);
            auto currentColor = color;
            currentColor[3] *= alpha;
            surfData[idx] = BlendColor(currentColor, surfData[idx]);
        }
    }
}
