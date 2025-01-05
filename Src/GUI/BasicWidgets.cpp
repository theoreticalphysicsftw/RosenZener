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

#include <GUI/BasicWidgets.hpp>
#include <Core/TypeTraits.hpp>


auto PlotEuclideanCoordinateFrame2D
(
	const Vector2& minBounds,
	const Vector2& maxBounds,
	const Vector2& minDrawBounds,
	const Vector2& maxDrawBounds,
	const Vector2& center,
	const String& xUnitsConst,
	const String& yUnitsConst,
	const String& xTerm,
	const String& yTerm,
	U32 unitsDrawnX,
	U32 unitsDrawnY,
	ColorU32 axisColor,
	F32 axisWidth
) -> Void
{
	auto bgDrawList = ImGui::GetBackgroundDrawList();
	auto drawCenter = RemapToRange(center, minBounds, maxBounds, minDrawBounds, maxDrawBounds);
	auto x0 = Vector2(minDrawBounds[0], drawCenter[1]);
	auto x1 = Vector2(maxDrawBounds[0], drawCenter[1]);
	auto y0 = Vector2(drawCenter[0], minDrawBounds[1]);
	auto y1 = Vector2(drawCenter[0], maxDrawBounds[1]);
	bgDrawList->AddLine(As<ImVec2>(x0), As<ImVec2>(x1), axisColor, axisWidth);
	bgDrawList->AddLine(As<ImVec2>(y0), As<ImVec2>(y1), axisColor, axisWidth);
	
	bgDrawList->AddText(ImVec2(x1[0] - 10, x1[1] + 2), axisColor, xTerm.ToCStr());
	bgDrawList->AddText(ImVec2(y1[0] + 7, y1[1] - 2), axisColor, yTerm.ToCStr());

	for (auto i = 1u; i < unitsDrawnX; ++i)
	{
		auto x = x0 + (x1 - x0) * F32(i) / F32(unitsDrawnX);
		auto val = RemapToRange(x, minDrawBounds, maxDrawBounds, minBounds, maxBounds);
		bgDrawList->AddLine(ImVec2(x[0], x[1] - 4), ImVec2(x[0], x[1] + 4), axisColor, axisWidth);
		if (!AreClose(x[0], drawCenter[0]))
		{
			bgDrawList->AddText(ImVec2(x[0] + 2, x[1] + 2), axisColor, (Format("{:.2f}", val[0]) + xUnitsConst).ToCStr());
		}
	}

	for (auto i = 1u; i < unitsDrawnY; ++i)
	{
		auto y = y0 + (y1 - y0) * F32(i) / F32(unitsDrawnY);
		auto val = RemapToRange(y, minDrawBounds, maxDrawBounds, minBounds, maxBounds);
		bgDrawList->AddLine(ImVec2(y[0] - 4, y[1]), ImVec2(y[0] + 4, y[1]), axisColor, axisWidth);
		if (!AreClose(y[1], drawCenter[1]))
		{
			bgDrawList->AddText(ImVec2(y[0] + 7, y[1] - 10), axisColor, (Format("{:.2f}", val[1]) + yUnitsConst).ToCStr());
		}
	}
}