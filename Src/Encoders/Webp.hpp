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
#include <Image/RawCPUImage.hpp>

#include <LibWebp/src/webp/decode.h>
#include <LibWebp/src/webp/encode.h>

inline auto DecodeWebp(Span<const Byte> data) -> RawCPUImage
{
	RawCPUImage result;
	result.format = EFormat::RGBA8;
	I32 width, height;

	WebPGetInfo(data.data(), data.size(), &width, &height);
	result.width = width;
	result.height = height;
	result.data.resize(result.height * result.width * sizeof(U32));

	auto decodeResult =
		WebPDecodeRGBAInto(data.data(), data.size(), result.data.data(), result.data.size(), result.width * sizeof(U32));

	if (decodeResult == nullptr)
	{
		result.data.clear();
		result.width = 0;
		result.height = 0;
	}

	return result;
}

inline auto EncodeWebp(const RawCPUImage& img, F32 qf) -> Array<Byte>
{
	U8* out;
	auto size = WebPEncodeRGBA(img.data.data(), img.width, img.height, img.width * 4, qf, &out);
	// Unnecessary copy but whatever it's not performance critical function.
	Array<Byte> result(out, out + size);
	WebPFree(out);
	return result;
}

inline auto EncodeWebp(const Byte* data, U32 w, U32 h, U32 stride, F32 qf) -> Array<Byte>
{
	U8* out;
	auto size = WebPEncodeRGBA(data, w, h, stride, qf, &out);
	// Unnecessary copy but whatever it's not performance critical function.
	Array<Byte> result(out, out + size);
	WebPFree(out);
	return result;
}

inline auto EncodeWebpBGRA(const Byte* data, U32 w, U32 h, U32 stride, F32 qf) -> Array<Byte>
{
	U8* out;
	auto size = WebPEncodeBGRA(data, w, h, stride, qf, &out);
	// Unnecessary copy but whatever it's not performance critical function.
	Array<Byte> result(out, out + size);
	WebPFree(out);
	return result;
}