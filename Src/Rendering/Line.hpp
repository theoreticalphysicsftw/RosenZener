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

#include <Math/Geometry/Line.hpp>
#include <Image/RawCPUImage.hpp>
#include <Image/Color.hpp>

template <typename TF>
inline auto DrawLine(RawCPUImage& dest, Line<TF, 2> line, const Color4& color, F32 widthPixel) -> Void
{
	static constexpr TF feather = 2.0;
	dest.ToSurfaceCoordinates(Span<Vector<TF, 2>>(line.points.data, 2));
	auto halfWidth = widthPixel / TF(2);
	auto bBox = line.GetBBox();

	auto xMin = Min(U32(Max(TF(0), Floor(bBox.lower[0] - halfWidth))), dest.width - 1);
	auto xMax = Min(U32(Max(TF(0), Ceil(bBox.upper[0] + halfWidth))), dest.width - 1);
	auto yMin = Min(U32(Max(TF(0), Floor(bBox.lower[1] - halfWidth))), dest.height - 1);
	auto yMax = Min(U32(Max(TF(0), Ceil(bBox.upper[1]) + halfWidth)), dest.height - 1);

	ColorU32* surfData = (ColorU32*)dest.data.GetData();
	for (auto i = yMin; i <= yMax; ++i)
	{
		for (auto j = xMin; j <= xMax; ++j)
		{
			auto idx = LebesgueCurve(j, i);
			auto pixelCenter = Vector2(j, i) + TF(0.5);

			auto dist = line.GetDistanceFrom(pixelCenter) - halfWidth;
			auto alpha = TF(1) - SmoothStep(TF(0), feather, dist);
			auto currentColor = color;
			currentColor[3] *= alpha;
			surfData[idx] = BlendColor(currentColor, surfData[idx]);
		}
	}
}
