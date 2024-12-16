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

#include <deque>

template <typename T>
struct Deque : std::deque<T>
{
    auto PushBack(const T& v) -> Void
    {
        std::deque<T>::push_back(v);
    }

    auto PushFront(const T& v) -> Void
    {
        std::deque<T>::push_front(v);
    }

    auto PopFront() -> Void
    {
        std::deque<T>::pop_front();
    }

    template <typename... TArgs>
    auto EmplaceBack(TArgs&&... args) -> Void
    {
        std::deque<T>::template emplace_back<TArgs...>(Forward<TArgs>(args)...);
    }

    template <typename... TArgs>
    auto EmplaceFront(TArgs&&... args) -> Void
    {
        std::deque<T>::template emplace_front<TArgs...>(Forward<TArgs>(args)...);
    }

    auto GetSize() -> U64
    {
        return std::deque<T>::size();
    }

    auto GetFront() -> T&
    {
        return std::deque<T>::front();
    }

    auto GetFront() const -> const T&
    {
        return std::deque<T>::front();
    }
};
