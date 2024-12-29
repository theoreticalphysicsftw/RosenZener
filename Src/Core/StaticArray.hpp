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

template <typename T, U64 TSize>
struct StaticArray
{
    StaticArray() = default;

    template <typename... TArgs>
        requires CAllAreConstructibleFrom<T, TArgs...>
    StaticArray(TArgs... args) :
        data{static_cast<T>(args)...}
    {
    }

    auto operator==(const StaticArray& other) const -> Bool = default;

    constexpr auto GetSize() const -> U64
    {
        return TSize;
    }

    constexpr auto operator[](U64 i) -> T&
    {
        return data[i];
    }

    constexpr auto operator[](U64 i) const -> const T&
    {
        return data[i];
    }

    T data[TSize];
};
