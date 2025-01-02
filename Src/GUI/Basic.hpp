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

#include <Core.hpp>
#include <OS/Window.hpp>

#include <ImGUI/imgui.h>


class GUI
{
public:
    using CmdAccumulatorFunc = Function<Void()>;

	static auto Init() -> Bool;
	static auto Destroy() -> Void;
    static auto AddCmdAccumulator(CmdAccumulatorFunc&& f) -> U32;
    static auto RemoveCmdAccumulator(U32 idx) -> Void;

private:
	static auto ProcessInput(const Window::Event& e) -> Bool;
	static auto AccumulateGUICommands() -> Void;
	static auto PrepareRenderingData() -> Void;
	static auto Render() -> Void;
    
    inline static Array<CmdAccumulatorFunc> cmdAccumulators;

    friend class Window;
};

struct GUITexture
{
	ImTextureID id;
	U32 width;
	U32 height;

	GUITexture() : id(0), width(0), height(0) {}
	GUITexture(U32 width, U32 height) { Init(width, height); };

	~GUITexture();
	
	auto Init(U32 width, U32 height) -> Void;

	auto UpdateFromLebesgueRGBA8(const RawCPUImage& src) -> Void;
};
