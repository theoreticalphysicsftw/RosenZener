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
#include <Math/Algebra/Vector.hpp>
#include <Math/Discrete.hpp>


using Color3 = Vector3;
using Color4 = Vector4;

union ColorU32
{
    U32 packed;
    struct
    {
        U8 r;
        U8 g;
        U8 b;
        U8 a;
    };
    struct
    {
        U8 y;
        U8 cb;
        U8 cr;
        U8 a0;
    };
    struct
    {
        U8 y0;
        U8 u;
        U8 v;
        U8 a1;
    };

    constexpr ColorU32(U32 raw = 0);
    constexpr ColorU32(U8 r, U8 g, U8 b, U8 a);
    constexpr ColorU32(const Color4& c4);

    constexpr operator Color4() const;

    constexpr operator U32() const
    {
        return packed;
    }
};


inline auto YCbCrAToRGBA(ColorU32) -> ColorU32;
inline auto RGBAToYCbCrA(ColorU32) -> ColorU32;
inline auto RGBAToYCbCrABT601(ColorU32) -> ColorU32;
inline auto YUVAToRGBA(ColorU32) -> ColorU32;
inline auto RGBAToYUVA(ColorU32) -> ColorU32;
inline auto RGBAToGrayscale(ColorU32) -> U8;


inline constexpr ColorU32::ColorU32(U32 raw) :
    packed(raw)
{
}


inline constexpr ColorU32::ColorU32(U8 r, U8 g, U8 b, U8 a) :
    r(r), g(g), b(b), a(a)
{
}


inline constexpr ColorU32::ColorU32(const Color4& c4)
{
    r = U8(c4[0] * 255);
    g = U8(c4[1] * 255);
    b = U8(c4[2] * 255);
    a = U8(c4[3] * 255);
}


inline constexpr ColorU32::operator Color4() const
{
    return Color4(r / 255.f, g / 255.f, b / 255.f, a / 255.f);
}


inline auto YCbCrAToRGBA(ColorU32 ycbcra)-> ColorU32
{
    ColorU32 rgba;
    rgba.r = GetClampedU8(ycbcra.y + 1.402f * (ycbcra.cr - 128.f));
    rgba.g = GetClampedU8(ycbcra.y - 0.34414f * (ycbcra.cb - 128.f) - 0.71414f * (ycbcra.cr - 128.f));
    rgba.b = GetClampedU8(ycbcra.y + 1.772f * (ycbcra.cb - 128.f));
    rgba.a = ycbcra.a;
    return rgba;
}


inline auto RGBAToYCbCrA(ColorU32 rgba) -> ColorU32
{
    ColorU32 ycbcra;
    ycbcra.y = GetClampedU8(0.299f * rgba.r + 0.587f * rgba.g + 0.114f * rgba.b);
    ycbcra.cr = GetClampedU8(128.f - 0.1687f * rgba.r - 0.3313f * rgba.g + 0.5f * rgba.b);
    ycbcra.cb = GetClampedU8(128.f + 0.5f * rgba.r - 0.4187f * rgba.g - 0.0813f * rgba.b);
    ycbcra.a = rgba.a;
    return ycbcra;
}


inline auto RGBAToYCbCrABT601(ColorU32 rgba) -> ColorU32
{
    ColorU32 ycbcra;
    ycbcra.y = U8(Clamp(16.f + 0.2567f * rgba.r + 0.5041f * rgba.g + 0.0980f * rgba.b, 16.f, 235.f));
    ycbcra.cr = U8(Clamp(128.f - 0.1482f * rgba.r - 0.2909f * rgba.g + 0.4392f * rgba.b, 16.f, 240.f));
    ycbcra.cb = U8(Clamp(128.f + 0.4392f * rgba.r - 0.3677f * rgba.g - 0.0714f * rgba.b, 16.f, 240.f));
    ycbcra.a = rgba.a;
    return ycbcra;
}


inline auto YUVAToRGBA(ColorU32 yuva) -> ColorU32
{
    ColorU32 rgba;
    rgba.r = GetClampedU8(yuva.y + 1.1398f * yuva.v);
    rgba.g = GetClampedU8(yuva.y - 0.3947f * yuva.u - 0.5806f * yuva.v);
    rgba.b = GetClampedU8(yuva.y + 2.0321f * yuva.u);
    rgba.a = yuva.a;
    return rgba;
}


inline auto RGBAToYUVA(ColorU32 rgba) -> ColorU32
{
    ColorU32 yuva;
    yuva.y = GetClampedU8(0.299f * rgba.r + 0.587f * rgba.g + 0.114f * rgba.b);
    yuva.u = GetClampedU8(-0.1471f * rgba.r - 0.2889f * rgba.g + 0.436f * rgba.b);
    yuva.v = GetClampedU8(0.615f * rgba.r - 0.515f * rgba.g - 0.1f * rgba.b);
    yuva.a = rgba.a;
    return yuva;
}


inline auto RGBAToGrayscale(ColorU32 c) -> U8
{
    return GetClampedU8(0.299f * c.r + 0.587 * c.g + 0.114 * c.b) * (c.a / 255.f);
}


inline auto BlendColor(const Color4& fg, const Color4& bg) -> Color4
{
    auto alpha = fg[3];
    auto result = bg * (1 - alpha) + fg * alpha;
    result[3] = 1;
    return result;
}
