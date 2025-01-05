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

#include <string>
#include <cstring>
#include <format>


struct String : std::string
{
	constexpr String() = default;
	constexpr String(const Char* cstr) : std::string(cstr) {}
	constexpr String(const std::string& str) : std::string(str) {}
	constexpr String(const String& str) = default;
	constexpr String(String&& str) = default;

	auto operator=(const String&) -> String& = default;
	auto operator=(String&&) -> String& = default;

	auto operator=(const Char* cstr) -> String&
	{
		std::string::operator+=(cstr);
		return *this;
	}

	auto ToCStr() const -> const Char*
	{
		return std::string::c_str();
	}

	auto GetData() -> Char*
	{
		return std::string::data();
	}

	auto GetData() const -> const Char*
	{
		return std::string::data();
	}

	auto GetSize() const -> U32
	{
		return std::string::size();
	}

	constexpr auto operator+(const String& other) -> String
	{
		return As<String>(As<std::string>(*this) + As<std::string>(other));
	}
};

template <typename... TArgs>
using FormatStr = std::format_string<TArgs...>;

template <typename... TArgs>
constexpr auto Format(const Char* fmt, TArgs&&... args) -> String
{
	return std::vformat(fmt, std::make_format_args(args...));
}

template <typename... TArgs>
constexpr auto Format(const String& fmt, TArgs&&... args) -> String
{
	return std::vformat(fmt, std::make_format_args(args...));
}