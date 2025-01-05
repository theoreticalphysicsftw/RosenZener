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
#include <Core/Concepts.hpp>

#include <chrono>


template <typename T>
    requires CIsArithmetic<T>
inline auto GetTimeStampUS() -> T
{
    static auto staticTimePoint = std::chrono::steady_clock::now();
    auto timePoint = std::chrono::steady_clock::now();
    return T(std::chrono::duration_cast<std::chrono::nanoseconds>(timePoint - staticTimePoint).count() / T(1000));
}

template <typename T>
    requires CIsArithmetic<T>
inline auto UsToS(T us) -> T
{
    return us / T(1000000);
}

using StdTimePoint = std::chrono::system_clock::time_point;

inline auto GetStdTimePoint() -> StdTimePoint
{
    return std::chrono::system_clock::now();
}