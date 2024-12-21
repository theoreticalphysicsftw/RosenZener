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

#include <Math/Constants.hpp>
#include <Math/Discrete/BoundingBox.hpp>


template <typename TF, U32 Dim>
struct Line
{
	using Scalar = TF;
	using Vec = Vector<TF, Dim>;
	using BBox = BBox<TF, Dim>;

	static constexpr U32 dimension = Dim;

	union
	{
		struct
		{
			Vec p0;
			Vec p1;
		};
		StaticArray<Vec, 2> points;
	};

	auto operator==(const Line& other) -> Bool;

	Line(const Vec& p0 = Vec(), const Vec& p1 = Vec()) : p0(p0), p1(p1) {}

	auto GetPolynomialCoefficients() const -> StaticArray<Vec, 2>;
	auto GetBBox() const -> BBox;
	auto Intersects(const BBox& bBox) const -> Bool;
	auto EvaluateAt(Scalar t) const -> Vec;
	auto GetCentroid() const -> Vec;
	auto GetSquaredDistanceFrom(const Vec& p, Bool excludeEndpoints = false) const -> TF;
	auto GetDistanceFrom(const Vec& p, Bool excludeEndpoints = false) const -> TF;

	auto GetDirection() const  -> Vec
	{
		return p1 - p0;
	}
};



template<typename TF, U32 Dim>
inline auto Line<TF, Dim>::operator==(const Line& other) -> Bool
{
	return points == other.points;
}

template<typename TF, U32 Dim>
inline auto Line<TF, Dim>::GetPolynomialCoefficients() const -> StaticArray<Vec, 2>
{
	StaticArray<Vec, 2> result =
	{
		p1 - p0,
		p0
	};
	return result;
}


template<typename TF, U32 Dim>
inline auto Line<TF, Dim>::GetBBox() const -> BBox
{
	return BBox(Span<const Vec>(points.data, 2));
}


template<typename TF, U32 Dim>
inline auto Line<TF, Dim>::Intersects(const BBox& bBox) const -> Bool
{
	return GetBBox().Intersects(bBox);
}


template<typename TF, U32 Dim>
inline auto Line<TF, Dim>::EvaluateAt(Scalar t) const -> Vec
{
	return (Scalar(1) - t) * p0 + t * p1;
}


template<typename TF, U32 Dim>
inline auto Line<TF, Dim>::GetCentroid() const -> Vec
{
	return (p0 + p1) / 2.f;
}


template<typename TF, U32 Dim>
inline auto Line<TF, Dim>::GetSquaredDistanceFrom(const Vec& p, Bool excludeEndpoints) const -> TF
{
	// Polinomial coefficients of the direction from the point
	auto coefficients = GetPolynomialCoefficients();
	coefficients[1] = coefficients[1] - p;

	auto t = -coefficients[1].Dot(coefficients[0]) / coefficients[0].Dot(coefficients[0]);

	if (t > 0 && t < 1)
	{
		return SquaredDistance(p, EvaluateAt(t));
	}
	else if (!excludeEndpoints)
	{
		auto dist0 = SquaredDistance(p, p0);
		auto dist1 = SquaredDistance(p, p1);
		return Min(dist0, dist1);
	}
    else
    {
        return Constants<TF>::Infinity;
    }
}


template<typename TF, U32 Dim>
inline auto Line<TF, Dim>::GetDistanceFrom(const Vec& p, Bool excludeEndpoints) const -> TF
{
	return Sqrt(GetSquaredDistanceFrom(p, excludeEndpoints));
}
