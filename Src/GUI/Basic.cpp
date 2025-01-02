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


#include <GUI/Basic.hpp>
#include <OS/Window.hpp>
#include <OS/IO.hpp>

#include <ImGUI/imgui.h>
#include <ImGUI/backends/imgui_impl_sdl3.h>
#include <ImGUI/backends/imgui_impl_sdlrenderer3.h>
#include <ImGUI/misc/cpp/imgui_stdlib.h>


auto GUI::Init() -> Bool
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    auto& io = ImGui::GetIO();

    ImFontConfig fontCfg;
    fontCfg.SizePixels = 18;
    fontCfg.OversampleV = 4;
    fontCfg.OversampleH = 4;
    fontCfg.PixelSnapH = true;
    io.Fonts->AddFontDefault(&fontCfg);

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    
    return
        ImGui_ImplSDL3_InitForSDLRenderer(Window::window, Window::renderer) &&
        ImGui_ImplSDLRenderer3_Init(Window::renderer);
}


auto GUI::ProcessInput(const Window::Event& e) -> Bool
{
    return ImGui_ImplSDL3_ProcessEvent(&e);
}


auto GUI::AddCmdAccumulator(CmdAccumulatorFunc&& f) -> U32
{
    auto idx = cmdAccumulators.GetSize();
    cmdAccumulators.EmplaceBack(Move(f));
    return idx;
}


auto GUI::RemoveCmdAccumulator(U32 idx) -> Void
{
    cmdAccumulators.Remove(idx);
}


auto GUI::AccumulateGUICommands() -> Void
{
	ImGui_ImplSDLRenderer3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

    for (auto& accumulator : cmdAccumulators)
    {
        accumulator();
    }
}


auto GUI::PrepareRenderingData() -> Void
{
    ImGui::Render();
}


auto GUI::Render() -> Void
{
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), Window::renderer);
}


auto GUI::Destroy() -> Void
{
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

auto GUITexture::Init(U32 width, U32 height) -> Void
{
    this->width = width;
    this->height = height;
    id = As<ImTextureID>(SDL_CreateTexture(Window::renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, width, height));
}

GUITexture::~GUITexture()
{
    if (id)
    {
        SDL_DestroyTexture(As<SDL_Texture*>(id));
    }
}

auto GUITexture::UpdateFromLebesgueRGBA8(const RawCPUImage& src) -> Void
{
    auto nativeTex = As<SDL_Texture*>(id);
    SDL_Rect rect(0, 0, width, height);
    ColorU32* dst = nullptr;
    Int stride;
    SDL_LockTexture(nativeTex, &rect, As<Void**>(&dst), &stride);
    auto srcU32Ptr = reinterpret_cast<const ColorU32*>(src.data.GetData());
    Log(SDL_GetError());
    for (auto i = 0u; i < height; ++i)
    {
        for (auto j = 0u; j < width; ++j)
        {
            auto idx = LebesgueCurve(j, i);
            dst[i * stride / 4 + j] = srcU32Ptr[idx];
        }
    }
    SDL_UnlockTexture(nativeTex);
}
