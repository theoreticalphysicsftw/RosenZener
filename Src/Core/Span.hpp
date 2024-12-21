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

#include <Core/ExternAlias.hpp>
#include <Core/StaticArray.hpp>

#include <span>


template <typename T, U64 Size = std::dynamic_extent>
struct Span : std::span<T, Size>
{
    template <typename TIt>
    explicit constexpr Span(TIt first, U64 count) :
        std::span<T, Size>(first, count)
    {
    }

    explicit constexpr Span(InitializerList<T> iList) :
        std::span<T, Size>(iList)
    {
    }

    constexpr Span(StaticArray<T, Size>& arr) :
        std::span<T, Size>(arr.data, arr.GetSize())
    {
    }

    constexpr Span(const StaticArray<T, Size>& arr) :
        std::span<T, Size>(arr.data, arr.GetSize())
    {
    }
};


template <typename T>
struct Span<T, std::dynamic_extent> : std::span<T, std::dynamic_extent>
{
    template <typename TIt>
    constexpr Span(TIt first, U64 count) :
        std::span<T, std::dynamic_extent>(first, count)
    {
    }

    constexpr Span(InitializerList<T> iList) :
        std::span<T, std::dynamic_extent>(iList)
    {
    }

    template <U64 Size>
    constexpr Span(StaticArray<T, Size>& arr) :
        std::span<T, std::dynamic_extent>(arr.data, arr.GetSize())
    {
    }

    template <U64 Size>
    constexpr Span(const StaticArray<T, Size>& arr) :
        std::span<T, std::dynamic_extent>(arr.data, arr.GetSize())
    {
    }
};

template <typename T, U64 Size>
Span(StaticArray<T, Size>&) -> Span<T, Size>;

template <typename T, U64 Size>
Span(const StaticArray<T, Size>&) -> Span<const T, Size>;
