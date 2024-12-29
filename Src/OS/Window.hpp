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

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <Core.hpp>
#include <OS/Thread.hpp>
#include <Image/RawCPUImage.hpp>


class Window
{
public:
	using Event = SDL_Event;

	static auto Init(const Char* name, U32 width, U32 height, Bool vSyncEnabled) -> Bool;
	static auto Loop() -> Void;
	static auto Destroy() -> Void;
    
    static auto AddToDrawAfterClear(const RawCPUImage& imgRGBA8) -> Void;

	inline static U32 width = 0;
	inline static U32 height = 0;
	inline static Atomic<Bool> isClosed = true;
private:
	static auto ProcessInput() -> Void;
	static auto ApplyVSyncSetting() -> Void;
    static auto DrawAfterClear() -> Void;

	inline static SDL_Window* window = nullptr;
    inline static SDL_Renderer* renderer = nullptr;
    inline static SDL_Texture* screenTarget = nullptr;

    inline static const RawCPUImage* toDrawAfterClear = nullptr;

	inline static Bool isVSyncOn = true;

	friend class GUI;
};
