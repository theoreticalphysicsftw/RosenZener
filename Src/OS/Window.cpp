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


#include <OS/Window.hpp>
#include <GUI/Basic.hpp>
#include <OS/IO.hpp>

#include <SDL3/SDL_main.h>


auto Window::Init(const Char* name, U32 width, U32 height, Bool vSyncEnabled) -> Bool
{
    Window::width = width;
    Window::height = height;
    Window::isVSyncOn = vSyncEnabled;

    SDL_SetMainReady();
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        LogError("Error during SDL_Init(): ", SDL_GetError());
        return false;
    }

    auto windowFlags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    window = SDL_CreateWindow(name, width, height, windowFlags);
    if (window == nullptr)
    {
        LogError("Error during SDL_CreateWindow(): ", SDL_GetError());
        return false;
    }

    renderer = SDL_CreateRenderer(window, nullptr);

    if (renderer == nullptr)
    {
        LogError("Error during SDL_CreateRenderer(): ", SDL_GetError());
        return false;
    }

    ApplyVSyncSetting();
    isClosed = false;

    return true;
}


auto Window::ProcessInput() -> Void
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        GUI::ProcessInput(e);

        if (e.type == SDL_EVENT_QUIT || e.type == SDL_EVENT_WINDOW_DESTROYED)
        {
            isClosed = true;
        }

        if (e.window.type == SDL_EVENT_WINDOW_RESIZED)
        {
            Window::width = e.window.data1;
            Window::height = e.window.data2;
        }
    }
}


auto Window::Loop() -> Void
{
    while (!isClosed)
    {
        ProcessInput();

        SDL_RenderClear(renderer);
        DrawAfterClear();
        GUI::AccumulateGUICommands();
        GUI::PrepareRenderingData();
        GUI::Render();
        SDL_RenderPresent(renderer);
    }
}


auto Window::Destroy() -> Void
{
    SDL_DestroyWindow(window);
    SDL_Quit();
}


auto Window::DrawAfterClear() -> Void
{
    LockedTexture lockedTex;
    lockedTex.width = width;
    lockedTex.height = height;
    if (toDrawAfterClear)
    {
        SDL_LockTexture(screenTarget, nullptr, (Void**)&lockedTex.data, &lockedTex.stride);
        toDrawAfterClear->CopyToLockedTextureRGBA8(lockedTex);
        SDL_UnlockTexture(screenTarget);
        SDL_RenderTexture(renderer, screenTarget, nullptr, nullptr);
        toDrawAfterClear = nullptr;
    }
}


auto Window::AddToDrawAfterClear(const RawCPUImage& imgRGBA8) -> Void
{
    toDrawAfterClear = &imgRGBA8;
}


auto Window::ApplyVSyncSetting() -> Void
{
    SDL_SetRenderVSync(renderer, isVSyncOn);
}
