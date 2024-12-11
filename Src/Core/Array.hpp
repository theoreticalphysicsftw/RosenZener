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

    constexpr U32 GetSize() const
    {
        return std::vector<T>::size();
    }

    constexpr const T* GetData() const
    {
        return std::vector<T>::data();
    }

    constexpr T* GetData()
    {
        return std::vector<T>::data();
    }

    constexpr T& operator[](U32 i)
    {
        return std::vector<T>::operator[](i);
    }

    constexpr const T& operator[](U32 i) const
    {
        return std::vector<T>::operator[](i);
    }

    template <typename... TArgs>
    void EmplaceBack(TArgs&&... args)
    {
        std::vector<T>::template emplace_back<TArgs...>(Forward<TArgs>(args)...);
    };
};
