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

#include <GUI/Basic.hpp>
#include <Core/String.hpp>
#include <Image/Color.hpp>


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
) -> Void;