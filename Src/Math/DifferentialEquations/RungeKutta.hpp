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
#include <Core/Array.hpp>
#include <Math/Algebra.hpp>

template <typename TArg, typename TRange, typename TFunc>
    requires CIsFloatingPoint<TArg>
auto SolveRungeKutta
(
    const TArg& initialConditionArg,
    const TRange& initialConditionRange,
    TFunc&& mapArgAndFuncToDerivative,
    const TArg& step,
    U64 totalIterations
) -> Array<Pair<TArg, TRange>>
{
    using Scalar = VectorTraits<TRange>::Scalar;

    Array<Pair<TArg, TRange>> solution(totalIterations);
    solution.EmplaceBack(initialConditionArg, initialConditionRange);

    for (auto i = 0ull; i < totalIterations; ++i)
    {
        auto& prev = solution.GetBack();
        auto halfStep = step / TArg(2);

        auto k0 = mapArgAndFuncToDerivative(prev.first, prev.second);
        auto k1 = mapArgAndFuncToDerivative
        (
            prev.first + halfStep,
            prev.second + Scalar(halfStep) * k0
        );
        auto k2 = mapArgAndFuncToDerivative
        (
            prev.first + halfStep,
            prev.second + Scalar(halfStep) * k1
        );
        auto k3 = mapArgAndFuncToDerivative
        (
            prev.first + step,
            prev.second + Scalar(step) * k2
        );

        auto newArg = prev.first + step;
        auto newRangeVal = prev.second + Scalar(step / TArg(6)) * (k0 + Scalar(TArg(2)) * k1 + Scalar(TArg(2)) * k2 + k3);

        solution.EmplaceBack(newArg, newRangeVal);
    }

    return solution;
}

