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

inline auto RoundToPowerOfTwo(U32 x) -> U32
{
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x++;
    return x;
}

auto InterleaveBits(U16 n) -> U32
{
    U32 n32 = n;
    n32 = (n32 | (n32 << 8)) & 0b00000000111111110000000011111111u;
    n32 = (n32 | (n32 << 4)) & 0b00001111000011110000111100001111u;
    n32 = (n32 | (n32 << 2)) & 0b00110011001100110011001100110011u;
    n32 = (n32 | (n32 << 1)) & 0b01010101010101010101010101010101u;
    return n32;
}

auto DeinterleaveBits(U32 n) -> U16
{
    n = (n | (n >> 1)) & 0b00110011001100110011001100110011u;
    n = (n | (n >> 2)) & 0b00001111000011110000111100001111u;
    n = (n | (n >> 4)) & 0b00000000111111110000000011111111u;
    n = (n | (n >> 8)) & 0b00000000000000001111111111111111u;
    return U16(n);
}
