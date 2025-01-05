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
#include <Core/TypeTraits.hpp>

#include <vector>

template <typename T>
struct Array : std::vector<T>
{
    Array() = default;

    Array(U64 reserved)
    {
        std::vector<T>::reserve(reserved);
    }

    template <typename TIt>
    Array(const TIt& it0, const TIt it1) :
        std::vector<T>(it0, it1)
    {
    }

    Array(const Array& other) = default;
    Array(Array&& other) = default;

    Array& operator=(const Array& other) = default;
    Array& operator=(Array&& other) = default;

    constexpr auto GetSize() const -> U64
    {
        return std::vector<T>::size();
    }

    constexpr auto GetBack() const -> const T&
    {
        return std::vector<T>::back();
    }

    constexpr auto GetBack() -> T&
    {
        return std::vector<T>::back();
    }

    constexpr auto GetBeforeBack() const -> const T&
    {
        return std::vector<T>::operator[](GetSize() - 2);
    }

    constexpr auto GetBeforeBack() -> T&
    {
        return std::vector<T>::operator[](GetSize() - 2);
    }

    constexpr auto GetData() const -> const T*
    {
        return std::vector<T>::data();
    }

    constexpr auto GetData() -> T*
    {
        return std::vector<T>::data();
    }

    constexpr auto operator[](U64 i) -> T&
    {
        return std::vector<T>::operator[](i);
    }

    constexpr auto operator[](U64 i) const -> const T&
    {
        return std::vector<T>::operator[](i);
    }


    constexpr auto Remove(U64 i) -> Void
    {
        std::vector<T>::erase(std::vector<T>::begin() + i);
    }

    template <typename... TArgs>
    constexpr auto EmplaceBack(TArgs&&... args) -> Void
    {
        std::vector<T>::template emplace_back<TArgs...>(Forward<TArgs>(args)...);
    };
};
