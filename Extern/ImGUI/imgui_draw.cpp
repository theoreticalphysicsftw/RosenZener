// Modified from original -> default font changed.

// dear imgui, v1.91.6 WIP
// (drawing and font code)

/*

Index of this file:

// [SECTION] STB libraries implementation
// [SECTION] Style functions
// [SECTION] ImDrawList
// [SECTION] ImTriangulator, ImDrawList concave polygon fill
// [SECTION] ImDrawListSplitter
// [SECTION] ImDrawData
// [SECTION] Helpers ShadeVertsXXX functions
// [SECTION] ImFontConfig
// [SECTION] ImFontAtlas
// [SECTION] ImFontAtlas: glyph ranges helpers
// [SECTION] ImFontGlyphRangesBuilder
// [SECTION] ImFont
// [SECTION] ImGui Internal Render Helpers
// [SECTION] Decompression code
// [SECTION] Default font data (ProggyClean.ttf)

*/

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include "imgui.h"
#ifndef IMGUI_DISABLE
#include "imgui_internal.h"
#ifdef IMGUI_ENABLE_FREETYPE
#include "misc/freetype/imgui_freetype.h"
#endif

#include <stdio.h>      // vsnprintf, sscanf, printf

// Visual Studio warnings
#ifdef _MSC_VER
#pragma warning (disable: 4127)     // condition expression is constant
#pragma warning (disable: 4505)     // unreferenced local function has been removed (stb stuff)
#pragma warning (disable: 4996)     // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#pragma warning (disable: 26451)    // [Static Analyzer] Arithmetic overflow : Using operator 'xxx' on a 4 byte value and then casting the result to a 8 byte value. Cast the value to the wider type before calling operator 'xxx' to avoid overflow(io.2).
#pragma warning (disable: 26812)    // [Static Analyzer] The enum type 'xxx' is unscoped. Prefer 'enum class' over 'enum' (Enum.3). [MSVC Static Analyzer)
#endif

// Clang/GCC warnings with -Weverything
#if defined(__clang__)
#if __has_warning("-Wunknown-warning-option")
#pragma clang diagnostic ignored "-Wunknown-warning-option"         // warning: unknown warning group 'xxx'                      // not all warnings are known by all Clang versions and they tend to be rename-happy.. so ignoring warnings triggers new warnings on some configuration. Great!
#endif
#pragma clang diagnostic ignored "-Wunknown-pragmas"                // warning: unknown warning group 'xxx'
#pragma clang diagnostic ignored "-Wold-style-cast"                 // warning: use of old-style cast                            // yes, they are more terse.
#pragma clang diagnostic ignored "-Wfloat-equal"                    // warning: comparing floating point with == or != is unsafe // storing and comparing against same constants ok.
#pragma clang diagnostic ignored "-Wglobal-constructors"            // warning: declaration requires a global destructor         // similar to above, not sure what the exact difference is.
#pragma clang diagnostic ignored "-Wsign-conversion"                // warning: implicit conversion changes signedness
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"  // warning: zero as null pointer constant                    // some standard header variations use #define NULL 0
#pragma clang diagnostic ignored "-Wcomma"                          // warning: possible misuse of comma operator here
#pragma clang diagnostic ignored "-Wreserved-id-macro"              // warning: macro name is a reserved identifier
#pragma clang diagnostic ignored "-Wdouble-promotion"               // warning: implicit conversion from 'float' to 'double' when passing argument to function  // using printf() is a misery with this as C++ va_arg ellipsis changes float to double.
#pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"  // warning: implicit conversion from 'xxx' to 'float' may lose precision
#pragma clang diagnostic ignored "-Wreserved-identifier"            // warning: identifier '_Xxx' is reserved because it starts with '_' followed by a capital letter
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"            // warning: 'xxx' is an unsafe pointer used for buffer access
#pragma clang diagnostic ignored "-Wnontrivial-memaccess"           // warning: first argument in call to 'memset' is a pointer to non-trivially copyable type
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wpragmas"                          // warning: unknown option after '#pragma GCC diagnostic' kind
#pragma GCC diagnostic ignored "-Wunused-function"                  // warning: 'xxxx' defined but not used
#pragma GCC diagnostic ignored "-Wdouble-promotion"                 // warning: implicit conversion from 'float' to 'double' when passing argument to function
#pragma GCC diagnostic ignored "-Wconversion"                       // warning: conversion to 'xxxx' from 'xxxx' may alter its value
#pragma GCC diagnostic ignored "-Wstack-protector"                  // warning: stack protector not protecting local variables: variable length buffer
#pragma GCC diagnostic ignored "-Wclass-memaccess"                  // [__GNUC__ >= 8] warning: 'memset/memcpy' clearing/writing an object of type 'xxxx' with no trivial copy-assignment; use assignment or value-initialization instead
#endif

//-------------------------------------------------------------------------
// [SECTION] STB libraries implementation (for stb_truetype and stb_rect_pack)
//-------------------------------------------------------------------------

// Compile time options:
//#define IMGUI_STB_NAMESPACE           ImStb
//#define IMGUI_STB_TRUETYPE_FILENAME   "my_folder/stb_truetype.h"
//#define IMGUI_STB_RECT_PACK_FILENAME  "my_folder/stb_rect_pack.h"
//#define IMGUI_DISABLE_STB_TRUETYPE_IMPLEMENTATION
//#define IMGUI_DISABLE_STB_RECT_PACK_IMPLEMENTATION

#ifdef IMGUI_STB_NAMESPACE
namespace IMGUI_STB_NAMESPACE
{
#endif

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4456)                             // declaration of 'xx' hides previous local declaration
#pragma warning (disable: 6011)                             // (stb_rectpack) Dereferencing NULL pointer 'cur->next'.
#pragma warning (disable: 6385)                             // (stb_truetype) Reading invalid data from 'buffer':  the readable size is '_Old_3`kernel_width' bytes, but '3' bytes may be read.
#pragma warning (disable: 28182)                            // (stb_rectpack) Dereferencing NULL pointer. 'cur' contains the same NULL value as 'cur->next' did.
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"        // warning: 'xxxx' defined but not used
#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wimplicit-fallthrough"
#pragma clang diagnostic ignored "-Wcast-qual"              // warning: cast from 'const xxxx *' to 'xxx *' drops const qualifier
#endif

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"              // warning: comparison is always true due to limited range of data type [-Wtype-limits]
#pragma GCC diagnostic ignored "-Wcast-qual"                // warning: cast from type 'const xxxx *' to type 'xxxx *' casts away qualifiers
#endif

#ifndef STB_RECT_PACK_IMPLEMENTATION                        // in case the user already have an implementation in the _same_ compilation unit (e.g. unity builds)
#ifndef IMGUI_DISABLE_STB_RECT_PACK_IMPLEMENTATION          // in case the user already have an implementation in another compilation unit
#define STBRP_STATIC
#define STBRP_ASSERT(x)     do { IM_ASSERT(x); } while (0)
#define STBRP_SORT          ImQsort
#define STB_RECT_PACK_IMPLEMENTATION
#endif
#ifdef IMGUI_STB_RECT_PACK_FILENAME
#include IMGUI_STB_RECT_PACK_FILENAME
#else
#include "imstb_rectpack.h"
#endif
#endif

#ifdef  IMGUI_ENABLE_STB_TRUETYPE
#ifndef STB_TRUETYPE_IMPLEMENTATION                         // in case the user already have an implementation in the _same_ compilation unit (e.g. unity builds)
#ifndef IMGUI_DISABLE_STB_TRUETYPE_IMPLEMENTATION           // in case the user already have an implementation in another compilation unit
#define STBTT_malloc(x,u)   ((void)(u), IM_ALLOC(x))
#define STBTT_free(x,u)     ((void)(u), IM_FREE(x))
#define STBTT_assert(x)     do { IM_ASSERT(x); } while(0)
#define STBTT_fmod(x,y)     ImFmod(x,y)
#define STBTT_sqrt(x)       ImSqrt(x)
#define STBTT_pow(x,y)      ImPow(x,y)
#define STBTT_fabs(x)       ImFabs(x)
#define STBTT_ifloor(x)     ((int)ImFloor(x))
#define STBTT_iceil(x)      ((int)ImCeil(x))
#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#else
#define STBTT_DEF extern
#endif
#ifdef IMGUI_STB_TRUETYPE_FILENAME
#include IMGUI_STB_TRUETYPE_FILENAME
#else
#include "imstb_truetype.h"
#endif
#endif
#endif // IMGUI_ENABLE_STB_TRUETYPE

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#if defined(_MSC_VER)
#pragma warning (pop)
#endif

#ifdef IMGUI_STB_NAMESPACE
} // namespace ImStb
using namespace IMGUI_STB_NAMESPACE;
#endif

//-----------------------------------------------------------------------------
// [SECTION] Style functions
//-----------------------------------------------------------------------------

void ImGui::StyleColorsDark(ImGuiStyle* dst)
{
    ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border]                 = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Button]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator]              = colors[ImGuiCol_Border];
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_TabHovered]             = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_Tab]                    = ImLerp(colors[ImGuiCol_Header],       colors[ImGuiCol_TitleBgActive], 0.80f);
    colors[ImGuiCol_TabSelected]            = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
    colors[ImGuiCol_TabSelectedOverline]    = colors[ImGuiCol_HeaderActive];
    colors[ImGuiCol_TabDimmed]              = ImLerp(colors[ImGuiCol_Tab],          colors[ImGuiCol_TitleBg], 0.80f);
    colors[ImGuiCol_TabDimmedSelected]      = ImLerp(colors[ImGuiCol_TabSelected],  colors[ImGuiCol_TitleBg], 0.40f);
    colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.50f, 0.50f, 0.50f, 0.00f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);   // Prefer using Alpha=1.0 here
    colors[ImGuiCol_TableBorderLight]       = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);   // Prefer using Alpha=1.0 here
    colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextLink]               = colors[ImGuiCol_HeaderActive];
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavCursor]              = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

void ImGui::StyleColorsClassic(ImGuiStyle* dst)
{
    ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Text]                   = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.85f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.11f, 0.11f, 0.14f, 0.92f);
    colors[ImGuiCol_Border]                 = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.43f, 0.43f, 0.43f, 0.39f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.47f, 0.47f, 0.69f, 0.40f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.42f, 0.41f, 0.64f, 0.69f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.27f, 0.27f, 0.54f, 0.83f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.32f, 0.32f, 0.63f, 0.87f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.40f, 0.40f, 0.80f, 0.20f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.40f, 0.40f, 0.55f, 0.80f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.20f, 0.25f, 0.30f, 0.60f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.40f, 0.40f, 0.80f, 0.30f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.40f, 0.40f, 0.80f, 0.40f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.41f, 0.39f, 0.80f, 0.60f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.90f, 0.90f, 0.90f, 0.50f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.41f, 0.39f, 0.80f, 0.60f);
    colors[ImGuiCol_Button]                 = ImVec4(0.35f, 0.40f, 0.61f, 0.62f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.40f, 0.48f, 0.71f, 0.79f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.46f, 0.54f, 0.80f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.40f, 0.40f, 0.90f, 0.45f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.45f, 0.45f, 0.90f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.53f, 0.53f, 0.87f, 0.80f);
    colors[ImGuiCol_Separator]              = ImVec4(0.50f, 0.50f, 0.50f, 0.60f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.60f, 0.60f, 0.70f, 1.00f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.70f, 0.70f, 0.90f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.78f, 0.82f, 1.00f, 0.60f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.78f, 0.82f, 1.00f, 0.90f);
    colors[ImGuiCol_TabHovered]             = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_Tab]                    = ImLerp(colors[ImGuiCol_Header],       colors[ImGuiCol_TitleBgActive], 0.80f);
    colors[ImGuiCol_TabSelected]            = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
    colors[ImGuiCol_TabSelectedOverline]    = colors[ImGuiCol_HeaderActive];
    colors[ImGuiCol_TabDimmed]              = ImLerp(colors[ImGuiCol_Tab],          colors[ImGuiCol_TitleBg], 0.80f);
    colors[ImGuiCol_TabDimmedSelected]      = ImLerp(colors[ImGuiCol_TabSelected],  colors[ImGuiCol_TitleBg], 0.40f);
    colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.53f, 0.53f, 0.87f, 0.00f);
    colors[ImGuiCol_PlotLines]              = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.27f, 0.27f, 0.38f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.31f, 0.31f, 0.45f, 1.00f);   // Prefer using Alpha=1.0 here
    colors[ImGuiCol_TableBorderLight]       = ImVec4(0.26f, 0.26f, 0.28f, 1.00f);   // Prefer using Alpha=1.0 here
    colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.07f);
    colors[ImGuiCol_TextLink]               = colors[ImGuiCol_HeaderActive];
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavCursor]              = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
}

// Those light colors are better suited with a thicker font than the default one + FrameBorder
void ImGui::StyleColorsLight(ImGuiStyle* dst)
{
    ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Text]                   = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
    colors[ImGuiCol_Border]                 = ImVec4(0.00f, 0.00f, 0.00f, 0.30f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.69f, 0.69f, 0.69f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.46f, 0.54f, 0.80f, 0.60f);
    colors[ImGuiCol_Button]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator]              = ImVec4(0.39f, 0.39f, 0.39f, 0.62f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.14f, 0.44f, 0.80f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.14f, 0.44f, 0.80f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.35f, 0.35f, 0.35f, 0.17f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_TabHovered]             = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_Tab]                    = ImLerp(colors[ImGuiCol_Header],       colors[ImGuiCol_TitleBgActive], 0.90f);
    colors[ImGuiCol_TabSelected]            = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
    colors[ImGuiCol_TabSelectedOverline]    = colors[ImGuiCol_HeaderActive];
    colors[ImGuiCol_TabDimmed]              = ImLerp(colors[ImGuiCol_Tab],          colors[ImGuiCol_TitleBg], 0.80f);
    colors[ImGuiCol_TabDimmedSelected]      = ImLerp(colors[ImGuiCol_TabSelected],  colors[ImGuiCol_TitleBg], 0.40f);
    colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.26f, 0.59f, 1.00f, 0.00f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.45f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.78f, 0.87f, 0.98f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.57f, 0.57f, 0.64f, 1.00f);   // Prefer using Alpha=1.0 here
    colors[ImGuiCol_TableBorderLight]       = ImVec4(0.68f, 0.68f, 0.74f, 1.00f);   // Prefer using Alpha=1.0 here
    colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]          = ImVec4(0.30f, 0.30f, 0.30f, 0.09f);
    colors[ImGuiCol_TextLink]               = colors[ImGuiCol_HeaderActive];
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_NavCursor]              = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.20f, 0.20f, 0.20f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
}

//-----------------------------------------------------------------------------
// [SECTION] ImDrawList
//-----------------------------------------------------------------------------

ImDrawListSharedData::ImDrawListSharedData()
{
    memset(this, 0, sizeof(*this));
    for (int i = 0; i < IM_ARRAYSIZE(ArcFastVtx); i++)
    {
        const float a = ((float)i * 2 * IM_PI) / (float)IM_ARRAYSIZE(ArcFastVtx);
        ArcFastVtx[i] = ImVec2(ImCos(a), ImSin(a));
    }
    ArcFastRadiusCutoff = IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC_R(IM_DRAWLIST_ARCFAST_SAMPLE_MAX, CircleSegmentMaxError);
}

void ImDrawListSharedData::SetCircleTessellationMaxError(float max_error)
{
    if (CircleSegmentMaxError == max_error)
        return;

    IM_ASSERT(max_error > 0.0f);
    CircleSegmentMaxError = max_error;
    for (int i = 0; i < IM_ARRAYSIZE(CircleSegmentCounts); i++)
    {
        const float radius = (float)i;
        CircleSegmentCounts[i] = (ImU8)((i > 0) ? IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC(radius, CircleSegmentMaxError) : IM_DRAWLIST_ARCFAST_SAMPLE_MAX);
    }
    ArcFastRadiusCutoff = IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC_R(IM_DRAWLIST_ARCFAST_SAMPLE_MAX, CircleSegmentMaxError);
}

ImDrawList::ImDrawList(ImDrawListSharedData* shared_data)
{
    memset(this, 0, sizeof(*this)); 
    _Data = shared_data;
}

ImDrawList::~ImDrawList()
{
    _ClearFreeMemory();
}

// Initialize before use in a new frame. We always have a command ready in the buffer.
// In the majority of cases, you would want to call PushClipRect() and PushTextureID() after this.
void ImDrawList::_ResetForNewFrame()
{
    // Verify that the ImDrawCmd fields we want to memcmp() are contiguous in memory.
    IM_STATIC_ASSERT(offsetof(ImDrawCmd, ClipRect) == 0);
    IM_STATIC_ASSERT(offsetof(ImDrawCmd, TextureId) == sizeof(ImVec4));
    IM_STATIC_ASSERT(offsetof(ImDrawCmd, VtxOffset) == sizeof(ImVec4) + sizeof(ImTextureID));
    if (_Splitter._Count > 1)
        _Splitter.Merge(this);

    CmdBuffer.resize(0);
    IdxBuffer.resize(0);
    VtxBuffer.resize(0);
    Flags = _Data->InitialFlags;
    memset(&_CmdHeader, 0, sizeof(_CmdHeader));
    _VtxCurrentIdx = 0;
    _VtxWritePtr = NULL;
    _IdxWritePtr = NULL;
    _ClipRectStack.resize(0);
    _TextureIdStack.resize(0);
    _CallbacksDataBuf.resize(0);
    _Path.resize(0);
    _Splitter.Clear();
    CmdBuffer.push_back(ImDrawCmd());
    _FringeScale = 1.0f;
}

void ImDrawList::_ClearFreeMemory()
{
    CmdBuffer.clear();
    IdxBuffer.clear();
    VtxBuffer.clear();
    Flags = ImDrawListFlags_None;
    _VtxCurrentIdx = 0;
    _VtxWritePtr = NULL;
    _IdxWritePtr = NULL;
    _ClipRectStack.clear();
    _TextureIdStack.clear();
    _CallbacksDataBuf.clear();
    _Path.clear();
    _Splitter.ClearFreeMemory();
}

ImDrawList* ImDrawList::CloneOutput() const
{
    ImDrawList* dst = IM_NEW(ImDrawList(_Data));
    dst->CmdBuffer = CmdBuffer;
    dst->IdxBuffer = IdxBuffer;
    dst->VtxBuffer = VtxBuffer;
    dst->Flags = Flags;
    return dst;
}

void ImDrawList::AddDrawCmd()
{
    ImDrawCmd draw_cmd;
    draw_cmd.ClipRect = _CmdHeader.ClipRect;    // Same as calling ImDrawCmd_HeaderCopy()
    draw_cmd.TextureId = _CmdHeader.TextureId;
    draw_cmd.VtxOffset = _CmdHeader.VtxOffset;
    draw_cmd.IdxOffset = IdxBuffer.Size;

    IM_ASSERT(draw_cmd.ClipRect.x <= draw_cmd.ClipRect.z && draw_cmd.ClipRect.y <= draw_cmd.ClipRect.w);
    CmdBuffer.push_back(draw_cmd);
}

// Pop trailing draw command (used before merging or presenting to user)
// Note that this leaves the ImDrawList in a state unfit for further commands, as most code assume that CmdBuffer.Size > 0 && CmdBuffer.back().UserCallback == NULL
void ImDrawList::_PopUnusedDrawCmd()
{
    while (CmdBuffer.Size > 0)
    {
        ImDrawCmd* curr_cmd = &CmdBuffer.Data[CmdBuffer.Size - 1];
        if (curr_cmd->ElemCount != 0 || curr_cmd->UserCallback != NULL)
            return;// break;
        CmdBuffer.pop_back();
    }
}

void ImDrawList::AddCallback(ImDrawCallback callback, void* userdata, size_t userdata_size)
{
    IM_ASSERT_PARANOID(CmdBuffer.Size > 0);
    ImDrawCmd* curr_cmd = &CmdBuffer.Data[CmdBuffer.Size - 1];
    IM_ASSERT(curr_cmd->UserCallback == NULL);
    if (curr_cmd->ElemCount != 0)
    {
        AddDrawCmd();
        curr_cmd = &CmdBuffer.Data[CmdBuffer.Size - 1];
    }

    curr_cmd->UserCallback = callback;
    if (userdata_size == 0)
    {
        // Store user data directly in command (no indirection)
        curr_cmd->UserCallbackData = userdata;
        curr_cmd->UserCallbackDataSize = 0;
        curr_cmd->UserCallbackDataOffset = -1;
    }
    else
    {
        // Copy and store user data in a buffer
        IM_ASSERT(userdata != NULL);
        IM_ASSERT(userdata_size < (1u << 31));
        curr_cmd->UserCallbackData = NULL; // Will be resolved during Render()
        curr_cmd->UserCallbackDataSize = (int)userdata_size;
        curr_cmd->UserCallbackDataOffset = _CallbacksDataBuf.Size;
        _CallbacksDataBuf.resize(_CallbacksDataBuf.Size + (int)userdata_size);
        memcpy(_CallbacksDataBuf.Data + (size_t)curr_cmd->UserCallbackDataOffset, userdata, userdata_size);
    }

    AddDrawCmd(); // Force a new command after us (see comment below)
}

// Compare ClipRect, TextureId and VtxOffset with a single memcmp()
#define ImDrawCmd_HeaderSize                            (offsetof(ImDrawCmd, VtxOffset) + sizeof(unsigned int))
#define ImDrawCmd_HeaderCompare(CMD_LHS, CMD_RHS)       (memcmp(CMD_LHS, CMD_RHS, ImDrawCmd_HeaderSize))    // Compare ClipRect, TextureId, VtxOffset
#define ImDrawCmd_HeaderCopy(CMD_DST, CMD_SRC)          (memcpy(CMD_DST, CMD_SRC, ImDrawCmd_HeaderSize))    // Copy ClipRect, TextureId, VtxOffset
#define ImDrawCmd_AreSequentialIdxOffset(CMD_0, CMD_1)  (CMD_0->IdxOffset + CMD_0->ElemCount == CMD_1->IdxOffset)

// Try to merge two last draw commands
void ImDrawList::_TryMergeDrawCmds()
{
    IM_ASSERT_PARANOID(CmdBuffer.Size > 0);
    ImDrawCmd* curr_cmd = &CmdBuffer.Data[CmdBuffer.Size - 1];
    ImDrawCmd* prev_cmd = curr_cmd - 1;
    if (ImDrawCmd_HeaderCompare(curr_cmd, prev_cmd) == 0 && ImDrawCmd_AreSequentialIdxOffset(prev_cmd, curr_cmd) && curr_cmd->UserCallback == NULL && prev_cmd->UserCallback == NULL)
    {
        prev_cmd->ElemCount += curr_cmd->ElemCount;
        CmdBuffer.pop_back();
    }
}

// Our scheme may appears a bit unusual, basically we want the most-common calls AddLine AddRect etc. to not have to perform any check so we always have a command ready in the stack.
// The cost of figuring out if a new command has to be added or if we can merge is paid in those Update** functions only.
void ImDrawList::_OnChangedClipRect()
{
    // If current command is used with different settings we need to add a new command
    IM_ASSERT_PARANOID(CmdBuffer.Size > 0);
    ImDrawCmd* curr_cmd = &CmdBuffer.Data[CmdBuffer.Size - 1];
    if (curr_cmd->ElemCount != 0 && memcmp(&curr_cmd->ClipRect, &_CmdHeader.ClipRect, sizeof(ImVec4)) != 0)
    {
        AddDrawCmd();
        return;
    }
    IM_ASSERT(curr_cmd->UserCallback == NULL);

    // Try to merge with previous command if it matches, else use current command
    ImDrawCmd* prev_cmd = curr_cmd - 1;
    if (curr_cmd->ElemCount == 0 && CmdBuffer.Size > 1 && ImDrawCmd_HeaderCompare(&_CmdHeader, prev_cmd) == 0 && ImDrawCmd_AreSequentialIdxOffset(prev_cmd, curr_cmd) && prev_cmd->UserCallback == NULL)
    {
        CmdBuffer.pop_back();
        return;
    }
    curr_cmd->ClipRect = _CmdHeader.ClipRect;
}

void ImDrawList::_OnChangedTextureID()
{
    // If current command is used with different settings we need to add a new command
    IM_ASSERT_PARANOID(CmdBuffer.Size > 0);
    ImDrawCmd* curr_cmd = &CmdBuffer.Data[CmdBuffer.Size - 1];
    if (curr_cmd->ElemCount != 0 && curr_cmd->TextureId != _CmdHeader.TextureId)
    {
        AddDrawCmd();
        return;
    }
    IM_ASSERT(curr_cmd->UserCallback == NULL);

    // Try to merge with previous command if it matches, else use current command
    ImDrawCmd* prev_cmd = curr_cmd - 1;
    if (curr_cmd->ElemCount == 0 && CmdBuffer.Size > 1 && ImDrawCmd_HeaderCompare(&_CmdHeader, prev_cmd) == 0 && ImDrawCmd_AreSequentialIdxOffset(prev_cmd, curr_cmd) && prev_cmd->UserCallback == NULL)
    {
        CmdBuffer.pop_back();
        return;
    }
    curr_cmd->TextureId = _CmdHeader.TextureId;
}

void ImDrawList::_OnChangedVtxOffset()
{
    // We don't need to compare curr_cmd->VtxOffset != _CmdHeader.VtxOffset because we know it'll be different at the time we call this.
    _VtxCurrentIdx = 0;
    IM_ASSERT_PARANOID(CmdBuffer.Size > 0);
    ImDrawCmd* curr_cmd = &CmdBuffer.Data[CmdBuffer.Size - 1];
    //IM_ASSERT(curr_cmd->VtxOffset != _CmdHeader.VtxOffset); // See #3349
    if (curr_cmd->ElemCount != 0)
    {
        AddDrawCmd();
        return;
    }
    IM_ASSERT(curr_cmd->UserCallback == NULL);
    curr_cmd->VtxOffset = _CmdHeader.VtxOffset;
}

int ImDrawList::_CalcCircleAutoSegmentCount(float radius) const
{
    // Automatic segment count
    const int radius_idx = (int)(radius + 0.999999f); // ceil to never reduce accuracy
    if (radius_idx >= 0 && radius_idx < IM_ARRAYSIZE(_Data->CircleSegmentCounts))
        return _Data->CircleSegmentCounts[radius_idx]; // Use cached value
    else
        return IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC(radius, _Data->CircleSegmentMaxError);
}

// Render-level scissoring. This is passed down to your render function but not used for CPU-side coarse clipping. Prefer using higher-level ImGui::PushClipRect() to affect logic (hit-testing and widget culling)
void ImDrawList::PushClipRect(const ImVec2& cr_min, const ImVec2& cr_max, bool intersect_with_current_clip_rect)
{
    ImVec4 cr(cr_min.x, cr_min.y, cr_max.x, cr_max.y);
    if (intersect_with_current_clip_rect)
    {
        ImVec4 current = _CmdHeader.ClipRect;
        if (cr.x < current.x) cr.x = current.x;
        if (cr.y < current.y) cr.y = current.y;
        if (cr.z > current.z) cr.z = current.z;
        if (cr.w > current.w) cr.w = current.w;
    }
    cr.z = ImMax(cr.x, cr.z);
    cr.w = ImMax(cr.y, cr.w);

    _ClipRectStack.push_back(cr);
    _CmdHeader.ClipRect = cr;
    _OnChangedClipRect();
}

void ImDrawList::PushClipRectFullScreen()
{
    PushClipRect(ImVec2(_Data->ClipRectFullscreen.x, _Data->ClipRectFullscreen.y), ImVec2(_Data->ClipRectFullscreen.z, _Data->ClipRectFullscreen.w));
}

void ImDrawList::PopClipRect()
{
    _ClipRectStack.pop_back();
    _CmdHeader.ClipRect = (_ClipRectStack.Size == 0) ? _Data->ClipRectFullscreen : _ClipRectStack.Data[_ClipRectStack.Size - 1];
    _OnChangedClipRect();
}

void ImDrawList::PushTextureID(ImTextureID texture_id)
{
    _TextureIdStack.push_back(texture_id);
    _CmdHeader.TextureId = texture_id;
    _OnChangedTextureID();
}

void ImDrawList::PopTextureID()
{
    _TextureIdStack.pop_back();
    _CmdHeader.TextureId = (_TextureIdStack.Size == 0) ? (ImTextureID)NULL : _TextureIdStack.Data[_TextureIdStack.Size - 1];
    _OnChangedTextureID();
}

// This is used by ImGui::PushFont()/PopFont(). It works because we never use _TextureIdStack[] elsewhere than in PushTextureID()/PopTextureID().
void ImDrawList::_SetTextureID(ImTextureID texture_id)
{
    if (_CmdHeader.TextureId == texture_id)
        return;
    _CmdHeader.TextureId = texture_id;
    _OnChangedTextureID();
}

// Reserve space for a number of vertices and indices.
// You must finish filling your reserved data before calling PrimReserve() again, as it may reallocate or
// submit the intermediate results. PrimUnreserve() can be used to release unused allocations.
void ImDrawList::PrimReserve(int idx_count, int vtx_count)
{
    // Large mesh support (when enabled)
    IM_ASSERT_PARANOID(idx_count >= 0 && vtx_count >= 0);
    if (sizeof(ImDrawIdx) == 2 && (_VtxCurrentIdx + vtx_count >= (1 << 16)) && (Flags & ImDrawListFlags_AllowVtxOffset))
    {
        // FIXME: In theory we should be testing that vtx_count <64k here.
        // In practice, RenderText() relies on reserving ahead for a worst case scenario so it is currently useful for us
        // to not make that check until we rework the text functions to handle clipping and large horizontal lines better.
        _CmdHeader.VtxOffset = VtxBuffer.Size;
        _OnChangedVtxOffset();
    }

    ImDrawCmd* draw_cmd = &CmdBuffer.Data[CmdBuffer.Size - 1];
    draw_cmd->ElemCount += idx_count;

    int vtx_buffer_old_size = VtxBuffer.Size;
    VtxBuffer.resize(vtx_buffer_old_size + vtx_count);
    _VtxWritePtr = VtxBuffer.Data + vtx_buffer_old_size;

    int idx_buffer_old_size = IdxBuffer.Size;
    IdxBuffer.resize(idx_buffer_old_size + idx_count);
    _IdxWritePtr = IdxBuffer.Data + idx_buffer_old_size;
}

// Release the number of reserved vertices/indices from the end of the last reservation made with PrimReserve().
void ImDrawList::PrimUnreserve(int idx_count, int vtx_count)
{
    IM_ASSERT_PARANOID(idx_count >= 0 && vtx_count >= 0);

    ImDrawCmd* draw_cmd = &CmdBuffer.Data[CmdBuffer.Size - 1];
    draw_cmd->ElemCount -= idx_count;
    VtxBuffer.shrink(VtxBuffer.Size - vtx_count);
    IdxBuffer.shrink(IdxBuffer.Size - idx_count);
}

// Fully unrolled with inline call to keep our debug builds decently fast.
void ImDrawList::PrimRect(const ImVec2& a, const ImVec2& c, ImU32 col)
{
    ImVec2 b(c.x, a.y), d(a.x, c.y), uv(_Data->TexUvWhitePixel);
    ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
    _IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx+1); _IdxWritePtr[2] = (ImDrawIdx)(idx+2);
    _IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx+2); _IdxWritePtr[5] = (ImDrawIdx)(idx+3);
    _VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
    _VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col;
    _VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col;
    _VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv; _VtxWritePtr[3].col = col;
    _VtxWritePtr += 4;
    _VtxCurrentIdx += 4;
    _IdxWritePtr += 6;
}

void ImDrawList::PrimRectUV(const ImVec2& a, const ImVec2& c, const ImVec2& uv_a, const ImVec2& uv_c, ImU32 col)
{
    ImVec2 b(c.x, a.y), d(a.x, c.y), uv_b(uv_c.x, uv_a.y), uv_d(uv_a.x, uv_c.y);
    ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
    _IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx+1); _IdxWritePtr[2] = (ImDrawIdx)(idx+2);
    _IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx+2); _IdxWritePtr[5] = (ImDrawIdx)(idx+3);
    _VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv_a; _VtxWritePtr[0].col = col;
    _VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv_b; _VtxWritePtr[1].col = col;
    _VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv_c; _VtxWritePtr[2].col = col;
    _VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv_d; _VtxWritePtr[3].col = col;
    _VtxWritePtr += 4;
    _VtxCurrentIdx += 4;
    _IdxWritePtr += 6;
}

void ImDrawList::PrimQuadUV(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, const ImVec2& uv_a, const ImVec2& uv_b, const ImVec2& uv_c, const ImVec2& uv_d, ImU32 col)
{
    ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
    _IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx+1); _IdxWritePtr[2] = (ImDrawIdx)(idx+2);
    _IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx+2); _IdxWritePtr[5] = (ImDrawIdx)(idx+3);
    _VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv_a; _VtxWritePtr[0].col = col;
    _VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv_b; _VtxWritePtr[1].col = col;
    _VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv_c; _VtxWritePtr[2].col = col;
    _VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv_d; _VtxWritePtr[3].col = col;
    _VtxWritePtr += 4;
    _VtxCurrentIdx += 4;
    _IdxWritePtr += 6;
}

// On AddPolyline() and AddConvexPolyFilled() we intentionally avoid using ImVec2 and superfluous function calls to optimize debug/non-inlined builds.
// - Those macros expects l-values and need to be used as their own statement.
// - Those macros are intentionally not surrounded by the 'do {} while (0)' idiom because even that translates to runtime with debug compilers.
#define IM_NORMALIZE2F_OVER_ZERO(VX,VY)     { float d2 = VX*VX + VY*VY; if (d2 > 0.0f) { float inv_len = ImRsqrt(d2); VX *= inv_len; VY *= inv_len; } } (void)0
#define IM_FIXNORMAL2F_MAX_INVLEN2          100.0f // 500.0f (see #4053, #3366)
#define IM_FIXNORMAL2F(VX,VY)               { float d2 = VX*VX + VY*VY; if (d2 > 0.000001f) { float inv_len2 = 1.0f / d2; if (inv_len2 > IM_FIXNORMAL2F_MAX_INVLEN2) inv_len2 = IM_FIXNORMAL2F_MAX_INVLEN2; VX *= inv_len2; VY *= inv_len2; } } (void)0

// TODO: Thickness anti-aliased lines cap are missing their AA fringe.
// We avoid using the ImVec2 math operators here to reduce cost to a minimum for debug/non-inlined builds.
void ImDrawList::AddPolyline(const ImVec2* points, const int points_count, ImU32 col, ImDrawFlags flags, float thickness)
{
    if (points_count < 2 || (col & IM_COL32_A_MASK) == 0)
        return;

    const bool closed = (flags & ImDrawFlags_Closed) != 0;
    const ImVec2 opaque_uv = _Data->TexUvWhitePixel;
    const int count = closed ? points_count : points_count - 1; // The number of line segments we need to draw
    const bool thick_line = (thickness > _FringeScale);

    if (Flags & ImDrawListFlags_AntiAliasedLines)
    {
        // Anti-aliased stroke
        const float AA_SIZE = _FringeScale;
        const ImU32 col_trans = col & ~IM_COL32_A_MASK;

        // Thicknesses <1.0 should behave like thickness 1.0
        thickness = ImMax(thickness, 1.0f);
        const int integer_thickness = (int)thickness;
        const float fractional_thickness = thickness - integer_thickness;

        // Do we want to draw this line using a texture?
        // - For now, only draw integer-width lines using textures to avoid issues with the way scaling occurs, could be improved.
        // - If AA_SIZE is not 1.0f we cannot use the texture path.
        const bool use_texture = (Flags & ImDrawListFlags_AntiAliasedLinesUseTex) && (integer_thickness < IM_DRAWLIST_TEX_LINES_WIDTH_MAX) && (fractional_thickness <= 0.00001f) && (AA_SIZE == 1.0f);

        // We should never hit this, because NewFrame() doesn't set ImDrawListFlags_AntiAliasedLinesUseTex unless ImFontAtlasFlags_NoBakedLines is off
        IM_ASSERT_PARANOID(!use_texture || !(_Data->Font->ContainerAtlas->Flags & ImFontAtlasFlags_NoBakedLines));

        const int idx_count = use_texture ? (count * 6) : (thick_line ? count * 18 : count * 12);
        const int vtx_count = use_texture ? (points_count * 2) : (thick_line ? points_count * 4 : points_count * 3);
        PrimReserve(idx_count, vtx_count);

        // Temporary buffer
        // The first <points_count> items are normals at each line point, then after that there are either 2 or 4 temp points for each line point
        _Data->TempBuffer.reserve_discard(points_count * ((use_texture || !thick_line) ? 3 : 5));
        ImVec2* temp_normals = _Data->TempBuffer.Data;
        ImVec2* temp_points = temp_normals + points_count;

        // Calculate normals (tangents) for each line segment
        for (int i1 = 0; i1 < count; i1++)
        {
            const int i2 = (i1 + 1) == points_count ? 0 : i1 + 1;
            float dx = points[i2].x - points[i1].x;
            float dy = points[i2].y - points[i1].y;
            IM_NORMALIZE2F_OVER_ZERO(dx, dy);
            temp_normals[i1].x = dy;
            temp_normals[i1].y = -dx;
        }
        if (!closed)
            temp_normals[points_count - 1] = temp_normals[points_count - 2];

        // If we are drawing a one-pixel-wide line without a texture, or a textured line of any width, we only need 2 or 3 vertices per point
        if (use_texture || !thick_line)
        {
            // [PATH 1] Texture-based lines (thick or non-thick)
            // [PATH 2] Non texture-based lines (non-thick)

            // The width of the geometry we need to draw - this is essentially <thickness> pixels for the line itself, plus "one pixel" for AA.
            // - In the texture-based path, we don't use AA_SIZE here because the +1 is tied to the generated texture
            //   (see ImFontAtlasBuildRenderLinesTexData() function), and so alternate values won't work without changes to that code.
            // - In the non texture-based paths, we would allow AA_SIZE to potentially be != 1.0f with a patch (e.g. fringe_scale patch to
            //   allow scaling geometry while preserving one-screen-pixel AA fringe).
            const float half_draw_size = use_texture ? ((thickness * 0.5f) + 1) : AA_SIZE;

            // If line is not closed, the first and last points need to be generated differently as there are no normals to blend
            if (!closed)
            {
                temp_points[0] = points[0] + temp_normals[0] * half_draw_size;
                temp_points[1] = points[0] - temp_normals[0] * half_draw_size;
                temp_points[(points_count-1)*2+0] = points[points_count-1] + temp_normals[points_count-1] * half_draw_size;
                temp_points[(points_count-1)*2+1] = points[points_count-1] - temp_normals[points_count-1] * half_draw_size;
            }

            // Generate the indices to form a number of triangles for each line segment, and the vertices for the line edges
            // This takes points n and n+1 and writes into n+1, with the first point in a closed line being generated from the final one (as n+1 wraps)
            // FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
            unsigned int idx1 = _VtxCurrentIdx; // Vertex index for start of line segment
            for (int i1 = 0; i1 < count; i1++) // i1 is the first point of the line segment
            {
                const int i2 = (i1 + 1) == points_count ? 0 : i1 + 1; // i2 is the second point of the line segment
                const unsigned int idx2 = ((i1 + 1) == points_count) ? _VtxCurrentIdx : (idx1 + (use_texture ? 2 : 3)); // Vertex index for end of segment

                // Average normals
                float dm_x = (temp_normals[i1].x + temp_normals[i2].x) * 0.5f;
                float dm_y = (temp_normals[i1].y + temp_normals[i2].y) * 0.5f;
                IM_FIXNORMAL2F(dm_x, dm_y);
                dm_x *= half_draw_size; // dm_x, dm_y are offset to the outer edge of the AA area
                dm_y *= half_draw_size;

                // Add temporary vertexes for the outer edges
                ImVec2* out_vtx = &temp_points[i2 * 2];
                out_vtx[0].x = points[i2].x + dm_x;
                out_vtx[0].y = points[i2].y + dm_y;
                out_vtx[1].x = points[i2].x - dm_x;
                out_vtx[1].y = points[i2].y - dm_y;

                if (use_texture)
                {
                    // Add indices for two triangles
                    _IdxWritePtr[0] = (ImDrawIdx)(idx2 + 0); _IdxWritePtr[1] = (ImDrawIdx)(idx1 + 0); _IdxWritePtr[2] = (ImDrawIdx)(idx1 + 1); // Right tri
                    _IdxWritePtr[3] = (ImDrawIdx)(idx2 + 1); _IdxWritePtr[4] = (ImDrawIdx)(idx1 + 1); _IdxWritePtr[5] = (ImDrawIdx)(idx2 + 0); // Left tri
                    _IdxWritePtr += 6;
                }
                else
                {
                    // Add indexes for four triangles
                    _IdxWritePtr[0] = (ImDrawIdx)(idx2 + 0); _IdxWritePtr[1] = (ImDrawIdx)(idx1 + 0); _IdxWritePtr[2] = (ImDrawIdx)(idx1 + 2); // Right tri 1
                    _IdxWritePtr[3] = (ImDrawIdx)(idx1 + 2); _IdxWritePtr[4] = (ImDrawIdx)(idx2 + 2); _IdxWritePtr[5] = (ImDrawIdx)(idx2 + 0); // Right tri 2
                    _IdxWritePtr[6] = (ImDrawIdx)(idx2 + 1); _IdxWritePtr[7] = (ImDrawIdx)(idx1 + 1); _IdxWritePtr[8] = (ImDrawIdx)(idx1 + 0); // Left tri 1
                    _IdxWritePtr[9] = (ImDrawIdx)(idx1 + 0); _IdxWritePtr[10] = (ImDrawIdx)(idx2 + 0); _IdxWritePtr[11] = (ImDrawIdx)(idx2 + 1); // Left tri 2
                    _IdxWritePtr += 12;
                }

                idx1 = idx2;
            }

            // Add vertexes for each point on the line
            if (use_texture)
            {
                // If we're using textures we only need to emit the left/right edge vertices
                ImVec4 tex_uvs = _Data->TexUvLines[integer_thickness];
                /*if (fractional_thickness != 0.0f) // Currently always zero when use_texture==false!
                {
                    const ImVec4 tex_uvs_1 = _Data->TexUvLines[integer_thickness + 1];
                    tex_uvs.x = tex_uvs.x + (tex_uvs_1.x - tex_uvs.x) * fractional_thickness; // inlined ImLerp()
                    tex_uvs.y = tex_uvs.y + (tex_uvs_1.y - tex_uvs.y) * fractional_thickness;
                    tex_uvs.z = tex_uvs.z + (tex_uvs_1.z - tex_uvs.z) * fractional_thickness;
                    tex_uvs.w = tex_uvs.w + (tex_uvs_1.w - tex_uvs.w) * fractional_thickness;
                }*/
                ImVec2 tex_uv0(tex_uvs.x, tex_uvs.y);
                ImVec2 tex_uv1(tex_uvs.z, tex_uvs.w);
                for (int i = 0; i < points_count; i++)
                {
                    _VtxWritePtr[0].pos = temp_points[i * 2 + 0]; _VtxWritePtr[0].uv = tex_uv0; _VtxWritePtr[0].col = col; // Left-side outer edge
                    _VtxWritePtr[1].pos = temp_points[i * 2 + 1]; _VtxWritePtr[1].uv = tex_uv1; _VtxWritePtr[1].col = col; // Right-side outer edge
                    _VtxWritePtr += 2;
                }
            }
            else
            {
                // If we're not using a texture, we need the center vertex as well
                for (int i = 0; i < points_count; i++)
                {
                    _VtxWritePtr[0].pos = points[i];              _VtxWritePtr[0].uv = opaque_uv; _VtxWritePtr[0].col = col;       // Center of line
                    _VtxWritePtr[1].pos = temp_points[i * 2 + 0]; _VtxWritePtr[1].uv = opaque_uv; _VtxWritePtr[1].col = col_trans; // Left-side outer edge
                    _VtxWritePtr[2].pos = temp_points[i * 2 + 1]; _VtxWritePtr[2].uv = opaque_uv; _VtxWritePtr[2].col = col_trans; // Right-side outer edge
                    _VtxWritePtr += 3;
                }
            }
        }
        else
        {
            // [PATH 2] Non texture-based lines (thick): we need to draw the solid line core and thus require four vertices per point
            const float half_inner_thickness = (thickness - AA_SIZE) * 0.5f;

            // If line is not closed, the first and last points need to be generated differently as there are no normals to blend
            if (!closed)
            {
                const int points_last = points_count - 1;
                temp_points[0] = points[0] + temp_normals[0] * (half_inner_thickness + AA_SIZE);
                temp_points[1] = points[0] + temp_normals[0] * (half_inner_thickness);
                temp_points[2] = points[0] - temp_normals[0] * (half_inner_thickness);
                temp_points[3] = points[0] - temp_normals[0] * (half_inner_thickness + AA_SIZE);
                temp_points[points_last * 4 + 0] = points[points_last] + temp_normals[points_last] * (half_inner_thickness + AA_SIZE);
                temp_points[points_last * 4 + 1] = points[points_last] + temp_normals[points_last] * (half_inner_thickness);
                temp_points[points_last * 4 + 2] = points[points_last] - temp_normals[points_last] * (half_inner_thickness);
                temp_points[points_last * 4 + 3] = points[points_last] - temp_normals[points_last] * (half_inner_thickness + AA_SIZE);
            }

            // Generate the indices to form a number of triangles for each line segment, and the vertices for the line edges
            // This takes points n and n+1 and writes into n+1, with the first point in a closed line being generated from the final one (as n+1 wraps)
            // FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
            unsigned int idx1 = _VtxCurrentIdx; // Vertex index for start of line segment
            for (int i1 = 0; i1 < count; i1++) // i1 is the first point of the line segment
            {
                const int i2 = (i1 + 1) == points_count ? 0 : (i1 + 1); // i2 is the second point of the line segment
                const unsigned int idx2 = (i1 + 1) == points_count ? _VtxCurrentIdx : (idx1 + 4); // Vertex index for end of segment

                // Average normals
                float dm_x = (temp_normals[i1].x + temp_normals[i2].x) * 0.5f;
                float dm_y = (temp_normals[i1].y + temp_normals[i2].y) * 0.5f;
                IM_FIXNORMAL2F(dm_x, dm_y);
                float dm_out_x = dm_x * (half_inner_thickness + AA_SIZE);
                float dm_out_y = dm_y * (half_inner_thickness + AA_SIZE);
                float dm_in_x = dm_x * half_inner_thickness;
                float dm_in_y = dm_y * half_inner_thickness;

                // Add temporary vertices
                ImVec2* out_vtx = &temp_points[i2 * 4];
                out_vtx[0].x = points[i2].x + dm_out_x;
                out_vtx[0].y = points[i2].y + dm_out_y;
                out_vtx[1].x = points[i2].x + dm_in_x;
                out_vtx[1].y = points[i2].y + dm_in_y;
                out_vtx[2].x = points[i2].x - dm_in_x;
                out_vtx[2].y = points[i2].y - dm_in_y;
                out_vtx[3].x = points[i2].x - dm_out_x;
                out_vtx[3].y = points[i2].y - dm_out_y;

                // Add indexes
                _IdxWritePtr[0]  = (ImDrawIdx)(idx2 + 1); _IdxWritePtr[1]  = (ImDrawIdx)(idx1 + 1); _IdxWritePtr[2]  = (ImDrawIdx)(idx1 + 2);
                _IdxWritePtr[3]  = (ImDrawIdx)(idx1 + 2); _IdxWritePtr[4]  = (ImDrawIdx)(idx2 + 2); _IdxWritePtr[5]  = (ImDrawIdx)(idx2 + 1);
                _IdxWritePtr[6]  = (ImDrawIdx)(idx2 + 1); _IdxWritePtr[7]  = (ImDrawIdx)(idx1 + 1); _IdxWritePtr[8]  = (ImDrawIdx)(idx1 + 0);
                _IdxWritePtr[9]  = (ImDrawIdx)(idx1 + 0); _IdxWritePtr[10] = (ImDrawIdx)(idx2 + 0); _IdxWritePtr[11] = (ImDrawIdx)(idx2 + 1);
                _IdxWritePtr[12] = (ImDrawIdx)(idx2 + 2); _IdxWritePtr[13] = (ImDrawIdx)(idx1 + 2); _IdxWritePtr[14] = (ImDrawIdx)(idx1 + 3);
                _IdxWritePtr[15] = (ImDrawIdx)(idx1 + 3); _IdxWritePtr[16] = (ImDrawIdx)(idx2 + 3); _IdxWritePtr[17] = (ImDrawIdx)(idx2 + 2);
                _IdxWritePtr += 18;

                idx1 = idx2;
            }

            // Add vertices
            for (int i = 0; i < points_count; i++)
            {
                _VtxWritePtr[0].pos = temp_points[i * 4 + 0]; _VtxWritePtr[0].uv = opaque_uv; _VtxWritePtr[0].col = col_trans;
                _VtxWritePtr[1].pos = temp_points[i * 4 + 1]; _VtxWritePtr[1].uv = opaque_uv; _VtxWritePtr[1].col = col;
                _VtxWritePtr[2].pos = temp_points[i * 4 + 2]; _VtxWritePtr[2].uv = opaque_uv; _VtxWritePtr[2].col = col;
                _VtxWritePtr[3].pos = temp_points[i * 4 + 3]; _VtxWritePtr[3].uv = opaque_uv; _VtxWritePtr[3].col = col_trans;
                _VtxWritePtr += 4;
            }
        }
        _VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
    else
    {
        // [PATH 4] Non texture-based, Non anti-aliased lines
        const int idx_count = count * 6;
        const int vtx_count = count * 4;    // FIXME-OPT: Not sharing edges
        PrimReserve(idx_count, vtx_count);

        for (int i1 = 0; i1 < count; i1++)
        {
            const int i2 = (i1 + 1) == points_count ? 0 : i1 + 1;
            const ImVec2& p1 = points[i1];
            const ImVec2& p2 = points[i2];

            float dx = p2.x - p1.x;
            float dy = p2.y - p1.y;
            IM_NORMALIZE2F_OVER_ZERO(dx, dy);
            dx *= (thickness * 0.5f);
            dy *= (thickness * 0.5f);

            _VtxWritePtr[0].pos.x = p1.x + dy; _VtxWritePtr[0].pos.y = p1.y - dx; _VtxWritePtr[0].uv = opaque_uv; _VtxWritePtr[0].col = col;
            _VtxWritePtr[1].pos.x = p2.x + dy; _VtxWritePtr[1].pos.y = p2.y - dx; _VtxWritePtr[1].uv = opaque_uv; _VtxWritePtr[1].col = col;
            _VtxWritePtr[2].pos.x = p2.x - dy; _VtxWritePtr[2].pos.y = p2.y + dx; _VtxWritePtr[2].uv = opaque_uv; _VtxWritePtr[2].col = col;
            _VtxWritePtr[3].pos.x = p1.x - dy; _VtxWritePtr[3].pos.y = p1.y + dx; _VtxWritePtr[3].uv = opaque_uv; _VtxWritePtr[3].col = col;
            _VtxWritePtr += 4;

            _IdxWritePtr[0] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[1] = (ImDrawIdx)(_VtxCurrentIdx + 1); _IdxWritePtr[2] = (ImDrawIdx)(_VtxCurrentIdx + 2);
            _IdxWritePtr[3] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[4] = (ImDrawIdx)(_VtxCurrentIdx + 2); _IdxWritePtr[5] = (ImDrawIdx)(_VtxCurrentIdx + 3);
            _IdxWritePtr += 6;
            _VtxCurrentIdx += 4;
        }
    }
}

// - We intentionally avoid using ImVec2 and its math operators here to reduce cost to a minimum for debug/non-inlined builds.
// - Filled shapes must always use clockwise winding order. The anti-aliasing fringe depends on it. Counter-clockwise shapes will have "inward" anti-aliasing.
void ImDrawList::AddConvexPolyFilled(const ImVec2* points, const int points_count, ImU32 col)
{
    if (points_count < 3 || (col & IM_COL32_A_MASK) == 0)
        return;

    const ImVec2 uv = _Data->TexUvWhitePixel;

    if (Flags & ImDrawListFlags_AntiAliasedFill)
    {
        // Anti-aliased Fill
        const float AA_SIZE = _FringeScale;
        const ImU32 col_trans = col & ~IM_COL32_A_MASK;
        const int idx_count = (points_count - 2)*3 + points_count * 6;
        const int vtx_count = (points_count * 2);
        PrimReserve(idx_count, vtx_count);

        // Add indexes for fill
        unsigned int vtx_inner_idx = _VtxCurrentIdx;
        unsigned int vtx_outer_idx = _VtxCurrentIdx + 1;
        for (int i = 2; i < points_count; i++)
        {
            _IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx); _IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx + ((i - 1) << 1)); _IdxWritePtr[2] = (ImDrawIdx)(vtx_inner_idx + (i << 1));
            _IdxWritePtr += 3;
        }

        // Compute normals
        _Data->TempBuffer.reserve_discard(points_count);
        ImVec2* temp_normals = _Data->TempBuffer.Data;
        for (int i0 = points_count - 1, i1 = 0; i1 < points_count; i0 = i1++)
        {
            const ImVec2& p0 = points[i0];
            const ImVec2& p1 = points[i1];
            float dx = p1.x - p0.x;
            float dy = p1.y - p0.y;
            IM_NORMALIZE2F_OVER_ZERO(dx, dy);
            temp_normals[i0].x = dy;
            temp_normals[i0].y = -dx;
        }

        for (int i0 = points_count - 1, i1 = 0; i1 < points_count; i0 = i1++)
        {
            // Average normals
            const ImVec2& n0 = temp_normals[i0];
            const ImVec2& n1 = temp_normals[i1];
            float dm_x = (n0.x + n1.x) * 0.5f;
            float dm_y = (n0.y + n1.y) * 0.5f;
            IM_FIXNORMAL2F(dm_x, dm_y);
            dm_x *= AA_SIZE * 0.5f;
            dm_y *= AA_SIZE * 0.5f;

            // Add vertices
            _VtxWritePtr[0].pos.x = (points[i1].x - dm_x); _VtxWritePtr[0].pos.y = (points[i1].y - dm_y); _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;        // Inner
            _VtxWritePtr[1].pos.x = (points[i1].x + dm_x); _VtxWritePtr[1].pos.y = (points[i1].y + dm_y); _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col_trans;  // Outer
            _VtxWritePtr += 2;

            // Add indexes for fringes
            _IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx + (i1 << 1)); _IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx + (i0 << 1)); _IdxWritePtr[2] = (ImDrawIdx)(vtx_outer_idx + (i0 << 1));
            _IdxWritePtr[3] = (ImDrawIdx)(vtx_outer_idx + (i0 << 1)); _IdxWritePtr[4] = (ImDrawIdx)(vtx_outer_idx + (i1 << 1)); _IdxWritePtr[5] = (ImDrawIdx)(vtx_inner_idx + (i1 << 1));
            _IdxWritePtr += 6;
        }
        _VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
    else
    {
        // Non Anti-aliased Fill
        const int idx_count = (points_count - 2)*3;
        const int vtx_count = points_count;
        PrimReserve(idx_count, vtx_count);
        for (int i = 0; i < vtx_count; i++)
        {
            _VtxWritePtr[0].pos = points[i]; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
            _VtxWritePtr++;
        }
        for (int i = 2; i < points_count; i++)
        {
            _IdxWritePtr[0] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[1] = (ImDrawIdx)(_VtxCurrentIdx + i - 1); _IdxWritePtr[2] = (ImDrawIdx)(_VtxCurrentIdx + i);
            _IdxWritePtr += 3;
        }
        _VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
}

void ImDrawList::_PathArcToFastEx(const ImVec2& center, float radius, int a_min_sample, int a_max_sample, int a_step)
{
    if (radius < 0.5f)
    {
        _Path.push_back(center);
        return;
    }

    // Calculate arc auto segment step size
    if (a_step <= 0)
        a_step = IM_DRAWLIST_ARCFAST_SAMPLE_MAX / _CalcCircleAutoSegmentCount(radius);

    // Make sure we never do steps larger than one quarter of the circle
    a_step = ImClamp(a_step, 1, IM_DRAWLIST_ARCFAST_TABLE_SIZE / 4);

    const int sample_range = ImAbs(a_max_sample - a_min_sample);
    const int a_next_step = a_step;

    int samples = sample_range + 1;
    bool extra_max_sample = false;
    if (a_step > 1)
    {
        samples            = sample_range / a_step + 1;
        const int overstep = sample_range % a_step;

        if (overstep > 0)
        {
            extra_max_sample = true;
            samples++;

            // When we have overstep to avoid awkwardly looking one long line and one tiny one at the end,
            // distribute first step range evenly between them by reducing first step size.
            if (sample_range > 0)
                a_step -= (a_step - overstep) / 2;
        }
    }

    _Path.resize(_Path.Size + samples);
    ImVec2* out_ptr = _Path.Data + (_Path.Size - samples);

    int sample_index = a_min_sample;
    if (sample_index < 0 || sample_index >= IM_DRAWLIST_ARCFAST_SAMPLE_MAX)
    {
        sample_index = sample_index % IM_DRAWLIST_ARCFAST_SAMPLE_MAX;
        if (sample_index < 0)
            sample_index += IM_DRAWLIST_ARCFAST_SAMPLE_MAX;
    }

    if (a_max_sample >= a_min_sample)
    {
        for (int a = a_min_sample; a <= a_max_sample; a += a_step, sample_index += a_step, a_step = a_next_step)
        {
            // a_step is clamped to IM_DRAWLIST_ARCFAST_SAMPLE_MAX, so we have guaranteed that it will not wrap over range twice or more
            if (sample_index >= IM_DRAWLIST_ARCFAST_SAMPLE_MAX)
                sample_index -= IM_DRAWLIST_ARCFAST_SAMPLE_MAX;

            const ImVec2 s = _Data->ArcFastVtx[sample_index];
            out_ptr->x = center.x + s.x * radius;
            out_ptr->y = center.y + s.y * radius;
            out_ptr++;
        }
    }
    else
    {
        for (int a = a_min_sample; a >= a_max_sample; a -= a_step, sample_index -= a_step, a_step = a_next_step)
        {
            // a_step is clamped to IM_DRAWLIST_ARCFAST_SAMPLE_MAX, so we have guaranteed that it will not wrap over range twice or more
            if (sample_index < 0)
                sample_index += IM_DRAWLIST_ARCFAST_SAMPLE_MAX;

            const ImVec2 s = _Data->ArcFastVtx[sample_index];
            out_ptr->x = center.x + s.x * radius;
            out_ptr->y = center.y + s.y * radius;
            out_ptr++;
        }
    }

    if (extra_max_sample)
    {
        int normalized_max_sample = a_max_sample % IM_DRAWLIST_ARCFAST_SAMPLE_MAX;
        if (normalized_max_sample < 0)
            normalized_max_sample += IM_DRAWLIST_ARCFAST_SAMPLE_MAX;

        const ImVec2 s = _Data->ArcFastVtx[normalized_max_sample];
        out_ptr->x = center.x + s.x * radius;
        out_ptr->y = center.y + s.y * radius;
        out_ptr++;
    }

    IM_ASSERT_PARANOID(_Path.Data + _Path.Size == out_ptr);
}

void ImDrawList::_PathArcToN(const ImVec2& center, float radius, float a_min, float a_max, int num_segments)
{
    if (radius < 0.5f)
    {
        _Path.push_back(center);
        return;
    }

    // Note that we are adding a point at both a_min and a_max.
    // If you are trying to draw a full closed circle you don't want the overlapping points!
    _Path.reserve(_Path.Size + (num_segments + 1));
    for (int i = 0; i <= num_segments; i++)
    {
        const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
        _Path.push_back(ImVec2(center.x + ImCos(a) * radius, center.y + ImSin(a) * radius));
    }
}

// 0: East, 3: South, 6: West, 9: North, 12: East
void ImDrawList::PathArcToFast(const ImVec2& center, float radius, int a_min_of_12, int a_max_of_12)
{
    if (radius < 0.5f)
    {
        _Path.push_back(center);
        return;
    }
    _PathArcToFastEx(center, radius, a_min_of_12 * IM_DRAWLIST_ARCFAST_SAMPLE_MAX / 12, a_max_of_12 * IM_DRAWLIST_ARCFAST_SAMPLE_MAX / 12, 0);
}

void ImDrawList::PathArcTo(const ImVec2& center, float radius, float a_min, float a_max, int num_segments)
{
    if (radius < 0.5f)
    {
        _Path.push_back(center);
        return;
    }

    if (num_segments > 0)
    {
        _PathArcToN(center, radius, a_min, a_max, num_segments);
        return;
    }

    // Automatic segment count
    if (radius <= _Data->ArcFastRadiusCutoff)
    {
        const bool a_is_reverse = a_max < a_min;

        // We are going to use precomputed values for mid samples.
        // Determine first and last sample in lookup table that belong to the arc.
        const float a_min_sample_f = IM_DRAWLIST_ARCFAST_SAMPLE_MAX * a_min / (IM_PI * 2.0f);
        const float a_max_sample_f = IM_DRAWLIST_ARCFAST_SAMPLE_MAX * a_max / (IM_PI * 2.0f);

        const int a_min_sample = a_is_reverse ? (int)ImFloor(a_min_sample_f) : (int)ImCeil(a_min_sample_f);
        const int a_max_sample = a_is_reverse ? (int)ImCeil(a_max_sample_f) : (int)ImFloor(a_max_sample_f);
        const int a_mid_samples = a_is_reverse ? ImMax(a_min_sample - a_max_sample, 0) : ImMax(a_max_sample - a_min_sample, 0);

        const float a_min_segment_angle = a_min_sample * IM_PI * 2.0f / IM_DRAWLIST_ARCFAST_SAMPLE_MAX;
        const float a_max_segment_angle = a_max_sample * IM_PI * 2.0f / IM_DRAWLIST_ARCFAST_SAMPLE_MAX;
        const bool a_emit_start = ImAbs(a_min_segment_angle - a_min) >= 1e-5f;
        const bool a_emit_end = ImAbs(a_max - a_max_segment_angle) >= 1e-5f;

        _Path.reserve(_Path.Size + (a_mid_samples + 1 + (a_emit_start ? 1 : 0) + (a_emit_end ? 1 : 0)));
        if (a_emit_start)
            _Path.push_back(ImVec2(center.x + ImCos(a_min) * radius, center.y + ImSin(a_min) * radius));
        if (a_mid_samples > 0)
            _PathArcToFastEx(center, radius, a_min_sample, a_max_sample, 0);
        if (a_emit_end)
            _Path.push_back(ImVec2(center.x + ImCos(a_max) * radius, center.y + ImSin(a_max) * radius));
    }
    else
    {
        const float arc_length = ImAbs(a_max - a_min);
        const int circle_segment_count = _CalcCircleAutoSegmentCount(radius);
        const int arc_segment_count = ImMax((int)ImCeil(circle_segment_count * arc_length / (IM_PI * 2.0f)), (int)(2.0f * IM_PI / arc_length));
        _PathArcToN(center, radius, a_min, a_max, arc_segment_count);
    }
}

void ImDrawList::PathEllipticalArcTo(const ImVec2& center, const ImVec2& radius, float rot, float a_min, float a_max, int num_segments)
{
    if (num_segments <= 0)
        num_segments = _CalcCircleAutoSegmentCount(ImMax(radius.x, radius.y)); // A bit pessimistic, maybe there's a better computation to do here.

    _Path.reserve(_Path.Size + (num_segments + 1));

    const float cos_rot = ImCos(rot);
    const float sin_rot = ImSin(rot);
    for (int i = 0; i <= num_segments; i++)
    {
        const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
        ImVec2 point(ImCos(a) * radius.x, ImSin(a) * radius.y);
        const ImVec2 rel((point.x * cos_rot) - (point.y * sin_rot), (point.x * sin_rot) + (point.y * cos_rot));
        point.x = rel.x + center.x;
        point.y = rel.y + center.y;
        _Path.push_back(point);
    }
}

ImVec2 ImBezierCubicCalc(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, float t)
{
    float u = 1.0f - t;
    float w1 = u * u * u;
    float w2 = 3 * u * u * t;
    float w3 = 3 * u * t * t;
    float w4 = t * t * t;
    return ImVec2(w1 * p1.x + w2 * p2.x + w3 * p3.x + w4 * p4.x, w1 * p1.y + w2 * p2.y + w3 * p3.y + w4 * p4.y);
}

ImVec2 ImBezierQuadraticCalc(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, float t)
{
    float u = 1.0f - t;
    float w1 = u * u;
    float w2 = 2 * u * t;
    float w3 = t * t;
    return ImVec2(w1 * p1.x + w2 * p2.x + w3 * p3.x, w1 * p1.y + w2 * p2.y + w3 * p3.y);
}

// Closely mimics ImBezierCubicClosestPointCasteljau() in imgui.cpp
static void PathBezierCubicCurveToCasteljau(ImVector<ImVec2>* path, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float tess_tol, int level)
{
    float dx = x4 - x1;
    float dy = y4 - y1;
    float d2 = (x2 - x4) * dy - (y2 - y4) * dx;
    float d3 = (x3 - x4) * dy - (y3 - y4) * dx;
    d2 = (d2 >= 0) ? d2 : -d2;
    d3 = (d3 >= 0) ? d3 : -d3;
    if ((d2 + d3) * (d2 + d3) < tess_tol * (dx * dx + dy * dy))
    {
        path->push_back(ImVec2(x4, y4));
    }
    else if (level < 10)
    {
        float x12 = (x1 + x2) * 0.5f, y12 = (y1 + y2) * 0.5f;
        float x23 = (x2 + x3) * 0.5f, y23 = (y2 + y3) * 0.5f;
        float x34 = (x3 + x4) * 0.5f, y34 = (y3 + y4) * 0.5f;
        float x123 = (x12 + x23) * 0.5f, y123 = (y12 + y23) * 0.5f;
        float x234 = (x23 + x34) * 0.5f, y234 = (y23 + y34) * 0.5f;
        float x1234 = (x123 + x234) * 0.5f, y1234 = (y123 + y234) * 0.5f;
        PathBezierCubicCurveToCasteljau(path, x1, y1, x12, y12, x123, y123, x1234, y1234, tess_tol, level + 1);
        PathBezierCubicCurveToCasteljau(path, x1234, y1234, x234, y234, x34, y34, x4, y4, tess_tol, level + 1);
    }
}

static void PathBezierQuadraticCurveToCasteljau(ImVector<ImVec2>* path, float x1, float y1, float x2, float y2, float x3, float y3, float tess_tol, int level)
{
    float dx = x3 - x1, dy = y3 - y1;
    float det = (x2 - x3) * dy - (y2 - y3) * dx;
    if (det * det * 4.0f < tess_tol * (dx * dx + dy * dy))
    {
        path->push_back(ImVec2(x3, y3));
    }
    else if (level < 10)
    {
        float x12 = (x1 + x2) * 0.5f, y12 = (y1 + y2) * 0.5f;
        float x23 = (x2 + x3) * 0.5f, y23 = (y2 + y3) * 0.5f;
        float x123 = (x12 + x23) * 0.5f, y123 = (y12 + y23) * 0.5f;
        PathBezierQuadraticCurveToCasteljau(path, x1, y1, x12, y12, x123, y123, tess_tol, level + 1);
        PathBezierQuadraticCurveToCasteljau(path, x123, y123, x23, y23, x3, y3, tess_tol, level + 1);
    }
}

void ImDrawList::PathBezierCubicCurveTo(const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, int num_segments)
{
    ImVec2 p1 = _Path.back();
    if (num_segments == 0)
    {
        IM_ASSERT(_Data->CurveTessellationTol > 0.0f);
        PathBezierCubicCurveToCasteljau(&_Path, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y, _Data->CurveTessellationTol, 0); // Auto-tessellated
    }
    else
    {
        float t_step = 1.0f / (float)num_segments;
        for (int i_step = 1; i_step <= num_segments; i_step++)
            _Path.push_back(ImBezierCubicCalc(p1, p2, p3, p4, t_step * i_step));
    }
}

void ImDrawList::PathBezierQuadraticCurveTo(const ImVec2& p2, const ImVec2& p3, int num_segments)
{
    ImVec2 p1 = _Path.back();
    if (num_segments == 0)
    {
        IM_ASSERT(_Data->CurveTessellationTol > 0.0f);
        PathBezierQuadraticCurveToCasteljau(&_Path, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, _Data->CurveTessellationTol, 0);// Auto-tessellated
    }
    else
    {
        float t_step = 1.0f / (float)num_segments;
        for (int i_step = 1; i_step <= num_segments; i_step++)
            _Path.push_back(ImBezierQuadraticCalc(p1, p2, p3, t_step * i_step));
    }
}

static inline ImDrawFlags FixRectCornerFlags(ImDrawFlags flags)
{
    /*
    IM_STATIC_ASSERT(ImDrawFlags_RoundCornersTopLeft == (1 << 4));
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    // Obsoleted in 1.82 (from February 2021). This code was stripped/simplified and mostly commented in 1.90 (from September 2023)
    // - Legacy Support for hard coded ~0 (used to be a suggested equivalent to ImDrawCornerFlags_All)
    if (flags == ~0)                    { return ImDrawFlags_RoundCornersAll; }
    // - Legacy Support for hard coded 0x01 to 0x0F (matching 15 out of 16 old flags combinations). Read details in older version of this code.
    if (flags >= 0x01 && flags <= 0x0F) { return (flags << 4); }
    // We cannot support hard coded 0x00 with 'float rounding > 0.0f' --> replace with ImDrawFlags_RoundCornersNone or use 'float rounding = 0.0f'
#endif
    */
    // If this assert triggers, please update your code replacing hardcoded values with new ImDrawFlags_RoundCorners* values.
    // Note that ImDrawFlags_Closed (== 0x01) is an invalid flag for AddRect(), AddRectFilled(), PathRect() etc. anyway.
    // See details in 1.82 Changelog as well as 2021/03/12 and 2023/09/08 entries in "API BREAKING CHANGES" section.
    IM_ASSERT((flags & 0x0F) == 0 && "Misuse of legacy hardcoded ImDrawCornerFlags values!");

    if ((flags & ImDrawFlags_RoundCornersMask_) == 0)
        flags |= ImDrawFlags_RoundCornersAll;

    return flags;
}

void ImDrawList::PathRect(const ImVec2& a, const ImVec2& b, float rounding, ImDrawFlags flags)
{
    if (rounding >= 0.5f)
    {
        flags = FixRectCornerFlags(flags);
        rounding = ImMin(rounding, ImFabs(b.x - a.x) * (((flags & ImDrawFlags_RoundCornersTop) == ImDrawFlags_RoundCornersTop) || ((flags & ImDrawFlags_RoundCornersBottom) == ImDrawFlags_RoundCornersBottom) ? 0.5f : 1.0f) - 1.0f);
        rounding = ImMin(rounding, ImFabs(b.y - a.y) * (((flags & ImDrawFlags_RoundCornersLeft) == ImDrawFlags_RoundCornersLeft) || ((flags & ImDrawFlags_RoundCornersRight) == ImDrawFlags_RoundCornersRight) ? 0.5f : 1.0f) - 1.0f);
    }
    if (rounding < 0.5f || (flags & ImDrawFlags_RoundCornersMask_) == ImDrawFlags_RoundCornersNone)
    {
        PathLineTo(a);
        PathLineTo(ImVec2(b.x, a.y));
        PathLineTo(b);
        PathLineTo(ImVec2(a.x, b.y));
    }
    else
    {
        const float rounding_tl = (flags & ImDrawFlags_RoundCornersTopLeft)     ? rounding : 0.0f;
        const float rounding_tr = (flags & ImDrawFlags_RoundCornersTopRight)    ? rounding : 0.0f;
        const float rounding_br = (flags & ImDrawFlags_RoundCornersBottomRight) ? rounding : 0.0f;
        const float rounding_bl = (flags & ImDrawFlags_RoundCornersBottomLeft)  ? rounding : 0.0f;
        PathArcToFast(ImVec2(a.x + rounding_tl, a.y + rounding_tl), rounding_tl, 6, 9);
        PathArcToFast(ImVec2(b.x - rounding_tr, a.y + rounding_tr), rounding_tr, 9, 12);
        PathArcToFast(ImVec2(b.x - rounding_br, b.y - rounding_br), rounding_br, 0, 3);
        PathArcToFast(ImVec2(a.x + rounding_bl, b.y - rounding_bl), rounding_bl, 3, 6);
    }
}

void ImDrawList::AddLine(const ImVec2& p1, const ImVec2& p2, ImU32 col, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;
    PathLineTo(p1 + ImVec2(0.5f, 0.5f));
    PathLineTo(p2 + ImVec2(0.5f, 0.5f));
    PathStroke(col, 0, thickness);
}

// p_min = upper-left, p_max = lower-right
// Note we don't render 1 pixels sized rectangles properly.
void ImDrawList::AddRect(const ImVec2& p_min, const ImVec2& p_max, ImU32 col, float rounding, ImDrawFlags flags, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;
    if (Flags & ImDrawListFlags_AntiAliasedLines)
        PathRect(p_min + ImVec2(0.50f, 0.50f), p_max - ImVec2(0.50f, 0.50f), rounding, flags);
    else
        PathRect(p_min + ImVec2(0.50f, 0.50f), p_max - ImVec2(0.49f, 0.49f), rounding, flags); // Better looking lower-right corner and rounded non-AA shapes.
    PathStroke(col, ImDrawFlags_Closed, thickness);
}

void ImDrawList::AddRectFilled(const ImVec2& p_min, const ImVec2& p_max, ImU32 col, float rounding, ImDrawFlags flags)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;
    if (rounding < 0.5f || (flags & ImDrawFlags_RoundCornersMask_) == ImDrawFlags_RoundCornersNone)
    {
        PrimReserve(6, 4);
        PrimRect(p_min, p_max, col);
    }
    else
    {
        PathRect(p_min, p_max, rounding, flags);
        PathFillConvex(col);
    }
}

// p_min = upper-left, p_max = lower-right
void ImDrawList::AddRectFilledMultiColor(const ImVec2& p_min, const ImVec2& p_max, ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left)
{
    if (((col_upr_left | col_upr_right | col_bot_right | col_bot_left) & IM_COL32_A_MASK) == 0)
        return;

    const ImVec2 uv = _Data->TexUvWhitePixel;
    PrimReserve(6, 4);
    PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx + 1)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx + 2));
    PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx + 2)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx + 3));
    PrimWriteVtx(p_min, uv, col_upr_left);
    PrimWriteVtx(ImVec2(p_max.x, p_min.y), uv, col_upr_right);
    PrimWriteVtx(p_max, uv, col_bot_right);
    PrimWriteVtx(ImVec2(p_min.x, p_max.y), uv, col_bot_left);
}

void ImDrawList::AddQuad(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, ImU32 col, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(p1);
    PathLineTo(p2);
    PathLineTo(p3);
    PathLineTo(p4);
    PathStroke(col, ImDrawFlags_Closed, thickness);
}

void ImDrawList::AddQuadFilled(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(p1);
    PathLineTo(p2);
    PathLineTo(p3);
    PathLineTo(p4);
    PathFillConvex(col);
}

void ImDrawList::AddTriangle(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, ImU32 col, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(p1);
    PathLineTo(p2);
    PathLineTo(p3);
    PathStroke(col, ImDrawFlags_Closed, thickness);
}

void ImDrawList::AddTriangleFilled(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(p1);
    PathLineTo(p2);
    PathLineTo(p3);
    PathFillConvex(col);
}

void ImDrawList::AddCircle(const ImVec2& center, float radius, ImU32 col, int num_segments, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0 || radius < 0.5f)
        return;

    if (num_segments <= 0)
    {
        // Use arc with automatic segment count
        _PathArcToFastEx(center, radius - 0.5f, 0, IM_DRAWLIST_ARCFAST_SAMPLE_MAX, 0);
        _Path.Size--;
    }
    else
    {
        // Explicit segment count (still clamp to avoid drawing insanely tessellated shapes)
        num_segments = ImClamp(num_segments, 3, IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX);

        // Because we are filling a closed shape we remove 1 from the count of segments/points
        const float a_max = (IM_PI * 2.0f) * ((float)num_segments - 1.0f) / (float)num_segments;
        PathArcTo(center, radius - 0.5f, 0.0f, a_max, num_segments - 1);
    }

    PathStroke(col, ImDrawFlags_Closed, thickness);
}

void ImDrawList::AddCircleFilled(const ImVec2& center, float radius, ImU32 col, int num_segments)
{
    if ((col & IM_COL32_A_MASK) == 0 || radius < 0.5f)
        return;

    if (num_segments <= 0)
    {
        // Use arc with automatic segment count
        _PathArcToFastEx(center, radius, 0, IM_DRAWLIST_ARCFAST_SAMPLE_MAX, 0);
        _Path.Size--;
    }
    else
    {
        // Explicit segment count (still clamp to avoid drawing insanely tessellated shapes)
        num_segments = ImClamp(num_segments, 3, IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX);

        // Because we are filling a closed shape we remove 1 from the count of segments/points
        const float a_max = (IM_PI * 2.0f) * ((float)num_segments - 1.0f) / (float)num_segments;
        PathArcTo(center, radius, 0.0f, a_max, num_segments - 1);
    }

    PathFillConvex(col);
}

// Guaranteed to honor 'num_segments'
void ImDrawList::AddNgon(const ImVec2& center, float radius, ImU32 col, int num_segments, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0 || num_segments <= 2)
        return;

    // Because we are filling a closed shape we remove 1 from the count of segments/points
    const float a_max = (IM_PI * 2.0f) * ((float)num_segments - 1.0f) / (float)num_segments;
    PathArcTo(center, radius - 0.5f, 0.0f, a_max, num_segments - 1);
    PathStroke(col, ImDrawFlags_Closed, thickness);
}

// Guaranteed to honor 'num_segments'
void ImDrawList::AddNgonFilled(const ImVec2& center, float radius, ImU32 col, int num_segments)
{
    if ((col & IM_COL32_A_MASK) == 0 || num_segments <= 2)
        return;

    // Because we are filling a closed shape we remove 1 from the count of segments/points
    const float a_max = (IM_PI * 2.0f) * ((float)num_segments - 1.0f) / (float)num_segments;
    PathArcTo(center, radius, 0.0f, a_max, num_segments - 1);
    PathFillConvex(col);
}

// Ellipse
void ImDrawList::AddEllipse(const ImVec2& center, const ImVec2& radius, ImU32 col, float rot, int num_segments, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    if (num_segments <= 0)
        num_segments = _CalcCircleAutoSegmentCount(ImMax(radius.x, radius.y)); // A bit pessimistic, maybe there's a better computation to do here.

    // Because we are filling a closed shape we remove 1 from the count of segments/points
    const float a_max = IM_PI * 2.0f * ((float)num_segments - 1.0f) / (float)num_segments;
    PathEllipticalArcTo(center, radius, rot, 0.0f, a_max, num_segments - 1);
    PathStroke(col, true, thickness);
}

void ImDrawList::AddEllipseFilled(const ImVec2& center, const ImVec2& radius, ImU32 col, float rot, int num_segments)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    if (num_segments <= 0)
        num_segments = _CalcCircleAutoSegmentCount(ImMax(radius.x, radius.y)); // A bit pessimistic, maybe there's a better computation to do here.

    // Because we are filling a closed shape we remove 1 from the count of segments/points
    const float a_max = IM_PI * 2.0f * ((float)num_segments - 1.0f) / (float)num_segments;
    PathEllipticalArcTo(center, radius, rot, 0.0f, a_max, num_segments - 1);
    PathFillConvex(col);
}

// Cubic Bezier takes 4 controls points
void ImDrawList::AddBezierCubic(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, ImU32 col, float thickness, int num_segments)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(p1);
    PathBezierCubicCurveTo(p2, p3, p4, num_segments);
    PathStroke(col, 0, thickness);
}

// Quadratic Bezier takes 3 controls points
void ImDrawList::AddBezierQuadratic(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, ImU32 col, float thickness, int num_segments)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(p1);
    PathBezierQuadraticCurveTo(p2, p3, num_segments);
    PathStroke(col, 0, thickness);
}

void ImDrawList::AddText(ImFont* font, float font_size, const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end, float wrap_width, const ImVec4* cpu_fine_clip_rect)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    // Accept null ranges
    if (text_begin == text_end || text_begin[0] == 0)
        return;
    if (text_end == NULL)
        text_end = text_begin + strlen(text_begin);

    // Pull default font/size from the shared ImDrawListSharedData instance
    if (font == NULL)
        font = _Data->Font;
    if (font_size == 0.0f)
        font_size = _Data->FontSize;

    IM_ASSERT(font->ContainerAtlas->TexID == _CmdHeader.TextureId);  // Use high-level ImGui::PushFont() or low-level ImDrawList::PushTextureId() to change font.

    ImVec4 clip_rect = _CmdHeader.ClipRect;
    if (cpu_fine_clip_rect)
    {
        clip_rect.x = ImMax(clip_rect.x, cpu_fine_clip_rect->x);
        clip_rect.y = ImMax(clip_rect.y, cpu_fine_clip_rect->y);
        clip_rect.z = ImMin(clip_rect.z, cpu_fine_clip_rect->z);
        clip_rect.w = ImMin(clip_rect.w, cpu_fine_clip_rect->w);
    }
    font->RenderText(this, font_size, pos, col, clip_rect, text_begin, text_end, wrap_width, cpu_fine_clip_rect != NULL);
}

void ImDrawList::AddText(const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end)
{
    AddText(NULL, 0.0f, pos, col, text_begin, text_end);
}

void ImDrawList::AddImage(ImTextureID user_texture_id, const ImVec2& p_min, const ImVec2& p_max, const ImVec2& uv_min, const ImVec2& uv_max, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    const bool push_texture_id = user_texture_id != _CmdHeader.TextureId;
    if (push_texture_id)
        PushTextureID(user_texture_id);

    PrimReserve(6, 4);
    PrimRectUV(p_min, p_max, uv_min, uv_max, col);

    if (push_texture_id)
        PopTextureID();
}

void ImDrawList::AddImageQuad(ImTextureID user_texture_id, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, const ImVec2& uv1, const ImVec2& uv2, const ImVec2& uv3, const ImVec2& uv4, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    const bool push_texture_id = user_texture_id != _CmdHeader.TextureId;
    if (push_texture_id)
        PushTextureID(user_texture_id);

    PrimReserve(6, 4);
    PrimQuadUV(p1, p2, p3, p4, uv1, uv2, uv3, uv4, col);

    if (push_texture_id)
        PopTextureID();
}

void ImDrawList::AddImageRounded(ImTextureID user_texture_id, const ImVec2& p_min, const ImVec2& p_max, const ImVec2& uv_min, const ImVec2& uv_max, ImU32 col, float rounding, ImDrawFlags flags)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    flags = FixRectCornerFlags(flags);
    if (rounding < 0.5f || (flags & ImDrawFlags_RoundCornersMask_) == ImDrawFlags_RoundCornersNone)
    {
        AddImage(user_texture_id, p_min, p_max, uv_min, uv_max, col);
        return;
    }

    const bool push_texture_id = user_texture_id != _CmdHeader.TextureId;
    if (push_texture_id)
        PushTextureID(user_texture_id);

    int vert_start_idx = VtxBuffer.Size;
    PathRect(p_min, p_max, rounding, flags);
    PathFillConvex(col);
    int vert_end_idx = VtxBuffer.Size;
    ImGui::ShadeVertsLinearUV(this, vert_start_idx, vert_end_idx, p_min, p_max, uv_min, uv_max, true);

    if (push_texture_id)
        PopTextureID();
}

//-----------------------------------------------------------------------------
// [SECTION] ImTriangulator, ImDrawList concave polygon fill
//-----------------------------------------------------------------------------
// Triangulate concave polygons. Based on "Triangulation by Ear Clipping" paper, O(N^2) complexity.
// Reference: https://www.geometrictools.com/Documentation/TriangulationByEarClipping.pdf
// Provided as a convenience for user but not used by main library.
//-----------------------------------------------------------------------------
// - ImTriangulator [Internal]
// - AddConcavePolyFilled()
//-----------------------------------------------------------------------------

enum ImTriangulatorNodeType
{
    ImTriangulatorNodeType_Convex,
    ImTriangulatorNodeType_Ear,
    ImTriangulatorNodeType_Reflex
};

struct ImTriangulatorNode
{
    ImTriangulatorNodeType  Type;
    int                     Index;
    ImVec2                  Pos;
    ImTriangulatorNode*     Next;
    ImTriangulatorNode*     Prev;

    void    Unlink()        { Next->Prev = Prev; Prev->Next = Next; }
};

struct ImTriangulatorNodeSpan
{
    ImTriangulatorNode**    Data = NULL;
    int                     Size = 0;

    void    push_back(ImTriangulatorNode* node) { Data[Size++] = node; }
    void    find_erase_unsorted(int idx)        { for (int i = Size - 1; i >= 0; i--) if (Data[i]->Index == idx) { Data[i] = Data[Size - 1]; Size--; return; } }
};

struct ImTriangulator
{
    static int EstimateTriangleCount(int points_count)      { return (points_count < 3) ? 0 : points_count - 2; }
    static int EstimateScratchBufferSize(int points_count)  { return sizeof(ImTriangulatorNode) * points_count + sizeof(ImTriangulatorNode*) * points_count * 2; }

    void    Init(const ImVec2* points, int points_count, void* scratch_buffer);
    void    GetNextTriangle(unsigned int out_triangle[3]);     // Return relative indexes for next triangle

    // Internal functions
    void    BuildNodes(const ImVec2* points, int points_count);
    void    BuildReflexes();
    void    BuildEars();
    void    FlipNodeList();
    bool    IsEar(int i0, int i1, int i2, const ImVec2& v0, const ImVec2& v1, const ImVec2& v2) const;
    void    ReclassifyNode(ImTriangulatorNode* node);

    // Internal members
    int                     _TrianglesLeft = 0;
    ImTriangulatorNode*     _Nodes = NULL;
    ImTriangulatorNodeSpan  _Ears;
    ImTriangulatorNodeSpan  _Reflexes;
};

// Distribute storage for nodes, ears and reflexes.
// FIXME-OPT: if everything is convex, we could report it to caller and let it switch to an convex renderer
// (this would require first building reflexes to bail to convex if empty, without even building nodes)
void ImTriangulator::Init(const ImVec2* points, int points_count, void* scratch_buffer)
{
    IM_ASSERT(scratch_buffer != NULL && points_count >= 3);
    _TrianglesLeft = EstimateTriangleCount(points_count);
    _Nodes         = (ImTriangulatorNode*)scratch_buffer;                          // points_count x Node
    _Ears.Data     = (ImTriangulatorNode**)(_Nodes + points_count);                // points_count x Node*
    _Reflexes.Data = (ImTriangulatorNode**)(_Nodes + points_count) + points_count; // points_count x Node*
    BuildNodes(points, points_count);
    BuildReflexes();
    BuildEars();
}

void ImTriangulator::BuildNodes(const ImVec2* points, int points_count)
{
    for (int i = 0; i < points_count; i++)
    {
        _Nodes[i].Type = ImTriangulatorNodeType_Convex;
        _Nodes[i].Index = i;
        _Nodes[i].Pos = points[i];
        _Nodes[i].Next = _Nodes + i + 1;
        _Nodes[i].Prev = _Nodes + i - 1;
    }
    _Nodes[0].Prev = _Nodes + points_count - 1;
    _Nodes[points_count - 1].Next = _Nodes;
}

void ImTriangulator::BuildReflexes()
{
    ImTriangulatorNode* n1 = _Nodes;
    for (int i = _TrianglesLeft; i >= 0; i--, n1 = n1->Next)
    {
        if (ImTriangleIsClockwise(n1->Prev->Pos, n1->Pos, n1->Next->Pos))
            continue;
        n1->Type = ImTriangulatorNodeType_Reflex;
        _Reflexes.push_back(n1);
    }
}

void ImTriangulator::BuildEars()
{
    ImTriangulatorNode* n1 = _Nodes;
    for (int i = _TrianglesLeft; i >= 0; i--, n1 = n1->Next)
    {
        if (n1->Type != ImTriangulatorNodeType_Convex)
            continue;
        if (!IsEar(n1->Prev->Index, n1->Index, n1->Next->Index, n1->Prev->Pos, n1->Pos, n1->Next->Pos))
            continue;
        n1->Type = ImTriangulatorNodeType_Ear;
        _Ears.push_back(n1);
    }
}

void ImTriangulator::GetNextTriangle(unsigned int out_triangle[3])
{
    if (_Ears.Size == 0)
    {
        FlipNodeList();

        ImTriangulatorNode* node = _Nodes;
        for (int i = _TrianglesLeft; i >= 0; i--, node = node->Next)
            node->Type = ImTriangulatorNodeType_Convex;
        _Reflexes.Size = 0;
        BuildReflexes();
        BuildEars();

        // If we still don't have ears, it means geometry is degenerated.
        if (_Ears.Size == 0)
        {
            // Return first triangle available, mimicking the behavior of convex fill.
            IM_ASSERT(_TrianglesLeft > 0); // Geometry is degenerated
            _Ears.Data[0] = _Nodes;
            _Ears.Size    = 1;
        }
    }

    ImTriangulatorNode* ear = _Ears.Data[--_Ears.Size];
    out_triangle[0] = ear->Prev->Index;
    out_triangle[1] = ear->Index;
    out_triangle[2] = ear->Next->Index;

    ear->Unlink();
    if (ear == _Nodes)
        _Nodes = ear->Next;

    ReclassifyNode(ear->Prev);
    ReclassifyNode(ear->Next);
    _TrianglesLeft--;
}

void ImTriangulator::FlipNodeList()
{
    ImTriangulatorNode* prev = _Nodes;
    ImTriangulatorNode* temp = _Nodes;
    ImTriangulatorNode* current = _Nodes->Next;
    prev->Next = prev;
    prev->Prev = prev;
    while (current != _Nodes)
    {
        temp = current->Next;

        current->Next = prev;
        prev->Prev = current;
        _Nodes->Next = current;
        current->Prev = _Nodes;

        prev = current;
        current = temp;
    }
    _Nodes = prev;
}

// A triangle is an ear is no other vertex is inside it. We can test reflexes vertices only (see reference algorithm)
bool ImTriangulator::IsEar(int i0, int i1, int i2, const ImVec2& v0, const ImVec2& v1, const ImVec2& v2) const
{
    ImTriangulatorNode** p_end = _Reflexes.Data + _Reflexes.Size;
    for (ImTriangulatorNode** p = _Reflexes.Data; p < p_end; p++)
    {
        ImTriangulatorNode* reflex = *p;
        if (reflex->Index != i0 && reflex->Index != i1 && reflex->Index != i2)
            if (ImTriangleContainsPoint(v0, v1, v2, reflex->Pos))
                return false;
    }
    return true;
}

void ImTriangulator::ReclassifyNode(ImTriangulatorNode* n1)
{
    // Classify node
    ImTriangulatorNodeType type;
    const ImTriangulatorNode* n0 = n1->Prev;
    const ImTriangulatorNode* n2 = n1->Next;
    if (!ImTriangleIsClockwise(n0->Pos, n1->Pos, n2->Pos))
        type = ImTriangulatorNodeType_Reflex;
    else if (IsEar(n0->Index, n1->Index, n2->Index, n0->Pos, n1->Pos, n2->Pos))
        type = ImTriangulatorNodeType_Ear;
    else
        type = ImTriangulatorNodeType_Convex;

    // Update lists when a type changes
    if (type == n1->Type)
        return;
    if (n1->Type == ImTriangulatorNodeType_Reflex)
        _Reflexes.find_erase_unsorted(n1->Index);
    else if (n1->Type == ImTriangulatorNodeType_Ear)
        _Ears.find_erase_unsorted(n1->Index);
    if (type == ImTriangulatorNodeType_Reflex)
        _Reflexes.push_back(n1);
    else if (type == ImTriangulatorNodeType_Ear)
        _Ears.push_back(n1);
    n1->Type = type;
}

// Use ear-clipping algorithm to triangulate a simple polygon (no self-interaction, no holes).
// (Reminder: we don't perform any coarse clipping/culling in ImDrawList layer!
// It is up to caller to ensure not making costly calls that will be outside of visible area.
// As concave fill is noticeably more expensive than other primitives, be mindful of this...
// Caller can build AABB of points, and avoid filling if 'draw_list->_CmdHeader.ClipRect.Overlays(points_bb) == false')
void ImDrawList::AddConcavePolyFilled(const ImVec2* points, const int points_count, ImU32 col)
{
    if (points_count < 3 || (col & IM_COL32_A_MASK) == 0)
        return;

    const ImVec2 uv = _Data->TexUvWhitePixel;
    ImTriangulator triangulator;
    unsigned int triangle[3];
    if (Flags & ImDrawListFlags_AntiAliasedFill)
    {
        // Anti-aliased Fill
        const float AA_SIZE = _FringeScale;
        const ImU32 col_trans = col & ~IM_COL32_A_MASK;
        const int idx_count = (points_count - 2) * 3 + points_count * 6;
        const int vtx_count = (points_count * 2);
        PrimReserve(idx_count, vtx_count);

        // Add indexes for fill
        unsigned int vtx_inner_idx = _VtxCurrentIdx;
        unsigned int vtx_outer_idx = _VtxCurrentIdx + 1;

        _Data->TempBuffer.reserve_discard((ImTriangulator::EstimateScratchBufferSize(points_count) + sizeof(ImVec2)) / sizeof(ImVec2));
        triangulator.Init(points, points_count, _Data->TempBuffer.Data);
        while (triangulator._TrianglesLeft > 0)
        {
            triangulator.GetNextTriangle(triangle);
            _IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx + (triangle[0] << 1)); _IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx + (triangle[1] << 1)); _IdxWritePtr[2] = (ImDrawIdx)(vtx_inner_idx + (triangle[2] << 1));
            _IdxWritePtr += 3;
        }

        // Compute normals
        _Data->TempBuffer.reserve_discard(points_count);
        ImVec2* temp_normals = _Data->TempBuffer.Data;
        for (int i0 = points_count - 1, i1 = 0; i1 < points_count; i0 = i1++)
        {
            const ImVec2& p0 = points[i0];
            const ImVec2& p1 = points[i1];
            float dx = p1.x - p0.x;
            float dy = p1.y - p0.y;
            IM_NORMALIZE2F_OVER_ZERO(dx, dy);
            temp_normals[i0].x = dy;
            temp_normals[i0].y = -dx;
        }

        for (int i0 = points_count - 1, i1 = 0; i1 < points_count; i0 = i1++)
        {
            // Average normals
            const ImVec2& n0 = temp_normals[i0];
            const ImVec2& n1 = temp_normals[i1];
            float dm_x = (n0.x + n1.x) * 0.5f;
            float dm_y = (n0.y + n1.y) * 0.5f;
            IM_FIXNORMAL2F(dm_x, dm_y);
            dm_x *= AA_SIZE * 0.5f;
            dm_y *= AA_SIZE * 0.5f;

            // Add vertices
            _VtxWritePtr[0].pos.x = (points[i1].x - dm_x); _VtxWritePtr[0].pos.y = (points[i1].y - dm_y); _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;        // Inner
            _VtxWritePtr[1].pos.x = (points[i1].x + dm_x); _VtxWritePtr[1].pos.y = (points[i1].y + dm_y); _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col_trans;  // Outer
            _VtxWritePtr += 2;

            // Add indexes for fringes
            _IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx + (i1 << 1)); _IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx + (i0 << 1)); _IdxWritePtr[2] = (ImDrawIdx)(vtx_outer_idx + (i0 << 1));
            _IdxWritePtr[3] = (ImDrawIdx)(vtx_outer_idx + (i0 << 1)); _IdxWritePtr[4] = (ImDrawIdx)(vtx_outer_idx + (i1 << 1)); _IdxWritePtr[5] = (ImDrawIdx)(vtx_inner_idx + (i1 << 1));
            _IdxWritePtr += 6;
        }
        _VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
    else
    {
        // Non Anti-aliased Fill
        const int idx_count = (points_count - 2) * 3;
        const int vtx_count = points_count;
        PrimReserve(idx_count, vtx_count);
        for (int i = 0; i < vtx_count; i++)
        {
            _VtxWritePtr[0].pos = points[i]; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
            _VtxWritePtr++;
        }
        _Data->TempBuffer.reserve_discard((ImTriangulator::EstimateScratchBufferSize(points_count) + sizeof(ImVec2)) / sizeof(ImVec2));
        triangulator.Init(points, points_count, _Data->TempBuffer.Data);
        while (triangulator._TrianglesLeft > 0)
        {
            triangulator.GetNextTriangle(triangle);
            _IdxWritePtr[0] = (ImDrawIdx)(_VtxCurrentIdx + triangle[0]); _IdxWritePtr[1] = (ImDrawIdx)(_VtxCurrentIdx + triangle[1]); _IdxWritePtr[2] = (ImDrawIdx)(_VtxCurrentIdx + triangle[2]);
            _IdxWritePtr += 3;
        }
        _VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
}

//-----------------------------------------------------------------------------
// [SECTION] ImDrawListSplitter
//-----------------------------------------------------------------------------
// FIXME: This may be a little confusing, trying to be a little too low-level/optimal instead of just doing vector swap..
//-----------------------------------------------------------------------------

void ImDrawListSplitter::ClearFreeMemory()
{
    for (int i = 0; i < _Channels.Size; i++)
    {
        if (i == _Current)
            memset(&_Channels[i], 0, sizeof(_Channels[i]));  // Current channel is a copy of CmdBuffer/IdxBuffer, don't destruct again
        _Channels[i]._CmdBuffer.clear();
        _Channels[i]._IdxBuffer.clear();
    }
    _Current = 0;
    _Count = 1;
    _Channels.clear();
}

void ImDrawListSplitter::Split(ImDrawList* draw_list, int channels_count)
{
    IM_UNUSED(draw_list);
    IM_ASSERT(_Current == 0 && _Count <= 1 && "Nested channel splitting is not supported. Please use separate instances of ImDrawListSplitter.");
    int old_channels_count = _Channels.Size;
    if (old_channels_count < channels_count)
    {
        _Channels.reserve(channels_count); // Avoid over reserving since this is likely to stay stable
        _Channels.resize(channels_count);
    }
    _Count = channels_count;

    // Channels[] (24/32 bytes each) hold storage that we'll swap with draw_list->_CmdBuffer/_IdxBuffer
    // The content of Channels[0] at this point doesn't matter. We clear it to make state tidy in a debugger but we don't strictly need to.
    // When we switch to the next channel, we'll copy draw_list->_CmdBuffer/_IdxBuffer into Channels[0] and then Channels[1] into draw_list->CmdBuffer/_IdxBuffer
    memset(&_Channels[0], 0, sizeof(ImDrawChannel));
    for (int i = 1; i < channels_count; i++)
    {
        if (i >= old_channels_count)
        {
            IM_PLACEMENT_NEW(&_Channels[i]) ImDrawChannel();
        }
        else
        {
            _Channels[i]._CmdBuffer.resize(0);
            _Channels[i]._IdxBuffer.resize(0);
        }
    }
}

void ImDrawListSplitter::Merge(ImDrawList* draw_list)
{
    // Note that we never use or rely on _Channels.Size because it is merely a buffer that we never shrink back to 0 to keep all sub-buffers ready for use.
    if (_Count <= 1)
        return;

    SetCurrentChannel(draw_list, 0);
    draw_list->_PopUnusedDrawCmd();

    // Calculate our final buffer sizes. Also fix the incorrect IdxOffset values in each command.
    int new_cmd_buffer_count = 0;
    int new_idx_buffer_count = 0;
    ImDrawCmd* last_cmd = (_Count > 0 && draw_list->CmdBuffer.Size > 0) ? &draw_list->CmdBuffer.back() : NULL;
    int idx_offset = last_cmd ? last_cmd->IdxOffset + last_cmd->ElemCount : 0;
    for (int i = 1; i < _Count; i++)
    {
        ImDrawChannel& ch = _Channels[i];
        if (ch._CmdBuffer.Size > 0 && ch._CmdBuffer.back().ElemCount == 0 && ch._CmdBuffer.back().UserCallback == NULL) // Equivalent of PopUnusedDrawCmd()
            ch._CmdBuffer.pop_back();

        if (ch._CmdBuffer.Size > 0 && last_cmd != NULL)
        {
            // Do not include ImDrawCmd_AreSequentialIdxOffset() in the compare as we rebuild IdxOffset values ourselves.
            // Manipulating IdxOffset (e.g. by reordering draw commands like done by RenderDimmedBackgroundBehindWindow()) is not supported within a splitter.
            ImDrawCmd* next_cmd = &ch._CmdBuffer[0];
            if (ImDrawCmd_HeaderCompare(last_cmd, next_cmd) == 0 && last_cmd->UserCallback == NULL && next_cmd->UserCallback == NULL)
            {
                // Merge previous channel last draw command with current channel first draw command if matching.
                last_cmd->ElemCount += next_cmd->ElemCount;
                idx_offset += next_cmd->ElemCount;
                ch._CmdBuffer.erase(ch._CmdBuffer.Data); // FIXME-OPT: Improve for multiple merges.
            }
        }
        if (ch._CmdBuffer.Size > 0)
            last_cmd = &ch._CmdBuffer.back();
        new_cmd_buffer_count += ch._CmdBuffer.Size;
        new_idx_buffer_count += ch._IdxBuffer.Size;
        for (int cmd_n = 0; cmd_n < ch._CmdBuffer.Size; cmd_n++)
        {
            ch._CmdBuffer.Data[cmd_n].IdxOffset = idx_offset;
            idx_offset += ch._CmdBuffer.Data[cmd_n].ElemCount;
        }
    }
    draw_list->CmdBuffer.resize(draw_list->CmdBuffer.Size + new_cmd_buffer_count);
    draw_list->IdxBuffer.resize(draw_list->IdxBuffer.Size + new_idx_buffer_count);

    // Write commands and indices in order (they are fairly small structures, we don't copy vertices only indices)
    ImDrawCmd* cmd_write = draw_list->CmdBuffer.Data + draw_list->CmdBuffer.Size - new_cmd_buffer_count;
    ImDrawIdx* idx_write = draw_list->IdxBuffer.Data + draw_list->IdxBuffer.Size - new_idx_buffer_count;
    for (int i = 1; i < _Count; i++)
    {
        ImDrawChannel& ch = _Channels[i];
        if (int sz = ch._CmdBuffer.Size) { memcpy(cmd_write, ch._CmdBuffer.Data, sz * sizeof(ImDrawCmd)); cmd_write += sz; }
        if (int sz = ch._IdxBuffer.Size) { memcpy(idx_write, ch._IdxBuffer.Data, sz * sizeof(ImDrawIdx)); idx_write += sz; }
    }
    draw_list->_IdxWritePtr = idx_write;

    // Ensure there's always a non-callback draw command trailing the command-buffer
    if (draw_list->CmdBuffer.Size == 0 || draw_list->CmdBuffer.back().UserCallback != NULL)
        draw_list->AddDrawCmd();

    // If current command is used with different settings we need to add a new command
    ImDrawCmd* curr_cmd = &draw_list->CmdBuffer.Data[draw_list->CmdBuffer.Size - 1];
    if (curr_cmd->ElemCount == 0)
        ImDrawCmd_HeaderCopy(curr_cmd, &draw_list->_CmdHeader); // Copy ClipRect, TextureId, VtxOffset
    else if (ImDrawCmd_HeaderCompare(curr_cmd, &draw_list->_CmdHeader) != 0)
        draw_list->AddDrawCmd();

    _Count = 1;
}

void ImDrawListSplitter::SetCurrentChannel(ImDrawList* draw_list, int idx)
{
    IM_ASSERT(idx >= 0 && idx < _Count);
    if (_Current == idx)
        return;

    // Overwrite ImVector (12/16 bytes), four times. This is merely a silly optimization instead of doing .swap()
    memcpy(&_Channels.Data[_Current]._CmdBuffer, &draw_list->CmdBuffer, sizeof(draw_list->CmdBuffer));
    memcpy(&_Channels.Data[_Current]._IdxBuffer, &draw_list->IdxBuffer, sizeof(draw_list->IdxBuffer));
    _Current = idx;
    memcpy(&draw_list->CmdBuffer, &_Channels.Data[idx]._CmdBuffer, sizeof(draw_list->CmdBuffer));
    memcpy(&draw_list->IdxBuffer, &_Channels.Data[idx]._IdxBuffer, sizeof(draw_list->IdxBuffer));
    draw_list->_IdxWritePtr = draw_list->IdxBuffer.Data + draw_list->IdxBuffer.Size;

    // If current command is used with different settings we need to add a new command
    ImDrawCmd* curr_cmd = (draw_list->CmdBuffer.Size == 0) ? NULL : &draw_list->CmdBuffer.Data[draw_list->CmdBuffer.Size - 1];
    if (curr_cmd == NULL)
        draw_list->AddDrawCmd();
    else if (curr_cmd->ElemCount == 0)
        ImDrawCmd_HeaderCopy(curr_cmd, &draw_list->_CmdHeader); // Copy ClipRect, TextureId, VtxOffset
    else if (ImDrawCmd_HeaderCompare(curr_cmd, &draw_list->_CmdHeader) != 0)
        draw_list->AddDrawCmd();
}

//-----------------------------------------------------------------------------
// [SECTION] ImDrawData
//-----------------------------------------------------------------------------

void ImDrawData::Clear()
{
    Valid = false;
    CmdListsCount = TotalIdxCount = TotalVtxCount = 0;
    CmdLists.resize(0); // The ImDrawList are NOT owned by ImDrawData but e.g. by ImGuiContext, so we don't clear them.
    DisplayPos = DisplaySize = FramebufferScale = ImVec2(0.0f, 0.0f);
    OwnerViewport = NULL;
}

// Important: 'out_list' is generally going to be draw_data->CmdLists, but may be another temporary list
// as long at it is expected that the result will be later merged into draw_data->CmdLists[].
void ImGui::AddDrawListToDrawDataEx(ImDrawData* draw_data, ImVector<ImDrawList*>* out_list, ImDrawList* draw_list)
{
    if (draw_list->CmdBuffer.Size == 0)
        return;
    if (draw_list->CmdBuffer.Size == 1 && draw_list->CmdBuffer[0].ElemCount == 0 && draw_list->CmdBuffer[0].UserCallback == NULL)
        return;

    // Draw list sanity check. Detect mismatch between PrimReserve() calls and incrementing _VtxCurrentIdx, _VtxWritePtr etc.
    // May trigger for you if you are using PrimXXX functions incorrectly.
    IM_ASSERT(draw_list->VtxBuffer.Size == 0 || draw_list->_VtxWritePtr == draw_list->VtxBuffer.Data + draw_list->VtxBuffer.Size);
    IM_ASSERT(draw_list->IdxBuffer.Size == 0 || draw_list->_IdxWritePtr == draw_list->IdxBuffer.Data + draw_list->IdxBuffer.Size);
    if (!(draw_list->Flags & ImDrawListFlags_AllowVtxOffset))
        IM_ASSERT((int)draw_list->_VtxCurrentIdx == draw_list->VtxBuffer.Size);

    // Check that draw_list doesn't use more vertices than indexable (default ImDrawIdx = unsigned short = 2 bytes = 64K vertices per ImDrawList = per window)
    // If this assert triggers because you are drawing lots of stuff manually:
    // - First, make sure you are coarse clipping yourself and not trying to draw many things outside visible bounds.
    //   Be mindful that the lower-level ImDrawList API doesn't filter vertices. Use the Metrics/Debugger window to inspect draw list contents.
    // - If you want large meshes with more than 64K vertices, you can either:
    //   (A) Handle the ImDrawCmd::VtxOffset value in your renderer backend, and set 'io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset'.
    //       Most example backends already support this from 1.71. Pre-1.71 backends won't.
    //       Some graphics API such as GL ES 1/2 don't have a way to offset the starting vertex so it is not supported for them.
    //   (B) Or handle 32-bit indices in your renderer backend, and uncomment '#define ImDrawIdx unsigned int' line in imconfig.h.
    //       Most example backends already support this. For example, the OpenGL example code detect index size at compile-time:
    //         glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
    //       Your own engine or render API may use different parameters or function calls to specify index sizes.
    //       2 and 4 bytes indices are generally supported by most graphics API.
    // - If for some reason neither of those solutions works for you, a workaround is to call BeginChild()/EndChild() before reaching
    //   the 64K limit to split your draw commands in multiple draw lists.
    if (sizeof(ImDrawIdx) == 2)
        IM_ASSERT(draw_list->_VtxCurrentIdx < (1 << 16) && "Too many vertices in ImDrawList using 16-bit indices. Read comment above");

    // Resolve callback data pointers
    if (draw_list->_CallbacksDataBuf.Size > 0)
        for (ImDrawCmd& cmd : draw_list->CmdBuffer)
            if (cmd.UserCallback != NULL && cmd.UserCallbackDataOffset != -1 && cmd.UserCallbackDataSize > 0)
                cmd.UserCallbackData = draw_list->_CallbacksDataBuf.Data + cmd.UserCallbackDataOffset;

    // Add to output list + records state in ImDrawData
    out_list->push_back(draw_list);
    draw_data->CmdListsCount++;
    draw_data->TotalVtxCount += draw_list->VtxBuffer.Size;
    draw_data->TotalIdxCount += draw_list->IdxBuffer.Size;
}

void ImDrawData::AddDrawList(ImDrawList* draw_list)
{
    IM_ASSERT(CmdLists.Size == CmdListsCount);
    draw_list->_PopUnusedDrawCmd();
    ImGui::AddDrawListToDrawDataEx(this, &CmdLists, draw_list);
}

// For backward compatibility: convert all buffers from indexed to de-indexed, in case you cannot render indexed. Note: this is slow and most likely a waste of resources. Always prefer indexed rendering!
void ImDrawData::DeIndexAllBuffers()
{
    ImVector<ImDrawVert> new_vtx_buffer;
    TotalVtxCount = TotalIdxCount = 0;
    for (int i = 0; i < CmdListsCount; i++)
    {
        ImDrawList* cmd_list = CmdLists[i];
        if (cmd_list->IdxBuffer.empty())
            continue;
        new_vtx_buffer.resize(cmd_list->IdxBuffer.Size);
        for (int j = 0; j < cmd_list->IdxBuffer.Size; j++)
            new_vtx_buffer[j] = cmd_list->VtxBuffer[cmd_list->IdxBuffer[j]];
        cmd_list->VtxBuffer.swap(new_vtx_buffer);
        cmd_list->IdxBuffer.resize(0);
        TotalVtxCount += cmd_list->VtxBuffer.Size;
    }
}

// Helper to scale the ClipRect field of each ImDrawCmd.
// Use if your final output buffer is at a different scale than draw_data->DisplaySize,
// or if there is a difference between your window resolution and framebuffer resolution.
void ImDrawData::ScaleClipRects(const ImVec2& fb_scale)
{
    for (ImDrawList* draw_list : CmdLists)
        for (ImDrawCmd& cmd : draw_list->CmdBuffer)
            cmd.ClipRect = ImVec4(cmd.ClipRect.x * fb_scale.x, cmd.ClipRect.y * fb_scale.y, cmd.ClipRect.z * fb_scale.x, cmd.ClipRect.w * fb_scale.y);
}

//-----------------------------------------------------------------------------
// [SECTION] Helpers ShadeVertsXXX functions
//-----------------------------------------------------------------------------

// Generic linear color gradient, write to RGB fields, leave A untouched.
void ImGui::ShadeVertsLinearColorGradientKeepAlpha(ImDrawList* draw_list, int vert_start_idx, int vert_end_idx, ImVec2 gradient_p0, ImVec2 gradient_p1, ImU32 col0, ImU32 col1)
{
    ImVec2 gradient_extent = gradient_p1 - gradient_p0;
    float gradient_inv_length2 = 1.0f / ImLengthSqr(gradient_extent);
    ImDrawVert* vert_start = draw_list->VtxBuffer.Data + vert_start_idx;
    ImDrawVert* vert_end = draw_list->VtxBuffer.Data + vert_end_idx;
    const int col0_r = (int)(col0 >> IM_COL32_R_SHIFT) & 0xFF;
    const int col0_g = (int)(col0 >> IM_COL32_G_SHIFT) & 0xFF;
    const int col0_b = (int)(col0 >> IM_COL32_B_SHIFT) & 0xFF;
    const int col_delta_r = ((int)(col1 >> IM_COL32_R_SHIFT) & 0xFF) - col0_r;
    const int col_delta_g = ((int)(col1 >> IM_COL32_G_SHIFT) & 0xFF) - col0_g;
    const int col_delta_b = ((int)(col1 >> IM_COL32_B_SHIFT) & 0xFF) - col0_b;
    for (ImDrawVert* vert = vert_start; vert < vert_end; vert++)
    {
        float d = ImDot(vert->pos - gradient_p0, gradient_extent);
        float t = ImClamp(d * gradient_inv_length2, 0.0f, 1.0f);
        int r = (int)(col0_r + col_delta_r * t);
        int g = (int)(col0_g + col_delta_g * t);
        int b = (int)(col0_b + col_delta_b * t);
        vert->col = (r << IM_COL32_R_SHIFT) | (g << IM_COL32_G_SHIFT) | (b << IM_COL32_B_SHIFT) | (vert->col & IM_COL32_A_MASK);
    }
}

// Distribute UV over (a, b) rectangle
void ImGui::ShadeVertsLinearUV(ImDrawList* draw_list, int vert_start_idx, int vert_end_idx, const ImVec2& a, const ImVec2& b, const ImVec2& uv_a, const ImVec2& uv_b, bool clamp)
{
    const ImVec2 size = b - a;
    const ImVec2 uv_size = uv_b - uv_a;
    const ImVec2 scale = ImVec2(
        size.x != 0.0f ? (uv_size.x / size.x) : 0.0f,
        size.y != 0.0f ? (uv_size.y / size.y) : 0.0f);

    ImDrawVert* vert_start = draw_list->VtxBuffer.Data + vert_start_idx;
    ImDrawVert* vert_end = draw_list->VtxBuffer.Data + vert_end_idx;
    if (clamp)
    {
        const ImVec2 min = ImMin(uv_a, uv_b);
        const ImVec2 max = ImMax(uv_a, uv_b);
        for (ImDrawVert* vertex = vert_start; vertex < vert_end; ++vertex)
            vertex->uv = ImClamp(uv_a + ImMul(ImVec2(vertex->pos.x, vertex->pos.y) - a, scale), min, max);
    }
    else
    {
        for (ImDrawVert* vertex = vert_start; vertex < vert_end; ++vertex)
            vertex->uv = uv_a + ImMul(ImVec2(vertex->pos.x, vertex->pos.y) - a, scale);
    }
}

void ImGui::ShadeVertsTransformPos(ImDrawList* draw_list, int vert_start_idx, int vert_end_idx, const ImVec2& pivot_in, float cos_a, float sin_a, const ImVec2& pivot_out)
{
    ImDrawVert* vert_start = draw_list->VtxBuffer.Data + vert_start_idx;
    ImDrawVert* vert_end = draw_list->VtxBuffer.Data + vert_end_idx;
    for (ImDrawVert* vertex = vert_start; vertex < vert_end; ++vertex)
        vertex->pos = ImRotate(vertex->pos- pivot_in, cos_a, sin_a) + pivot_out;
}

//-----------------------------------------------------------------------------
// [SECTION] ImFontConfig
//-----------------------------------------------------------------------------

ImFontConfig::ImFontConfig()
{
    memset(this, 0, sizeof(*this));
    FontDataOwnedByAtlas = true;
    OversampleH = 2;
    OversampleV = 1;
    GlyphMaxAdvanceX = FLT_MAX;
    RasterizerMultiply = 1.0f;
    RasterizerDensity = 1.0f;
    EllipsisChar = (ImWchar)-1;
}

//-----------------------------------------------------------------------------
// [SECTION] ImFontAtlas
//-----------------------------------------------------------------------------

// A work of art lies ahead! (. = white layer, X = black layer, others are blank)
// The 2x2 white texels on the top left are the ones we'll use everywhere in Dear ImGui to render filled shapes.
// (This is used when io.MouseDrawCursor = true)
const int FONT_ATLAS_DEFAULT_TEX_DATA_W = 122; // Actual texture will be 2 times that + 1 spacing.
const int FONT_ATLAS_DEFAULT_TEX_DATA_H = 27;
static const char FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS[FONT_ATLAS_DEFAULT_TEX_DATA_W * FONT_ATLAS_DEFAULT_TEX_DATA_H + 1] =
{
    "..-         -XXXXXXX-    X    -           X           -XXXXXXX          -          XXXXXXX-     XX          - XX       XX "
    "..-         -X.....X-   X.X   -          X.X          -X.....X          -          X.....X-    X..X         -X..X     X..X"
    "---         -XXX.XXX-  X...X  -         X...X         -X....X           -           X....X-    X..X         -X...X   X...X"
    "X           -  X.X  - X.....X -        X.....X        -X...X            -            X...X-    X..X         - X...X X...X "
    "XX          -  X.X  -X.......X-       X.......X       -X..X.X           -           X.X..X-    X..X         -  X...X...X  "
    "X.X         -  X.X  -XXXX.XXXX-       XXXX.XXXX       -X.X X.X          -          X.X X.X-    X..XXX       -   X.....X   "
    "X..X        -  X.X  -   X.X   -          X.X          -XX   X.X         -         X.X   XX-    X..X..XXX    -    X...X    "
    "X...X       -  X.X  -   X.X   -    XX    X.X    XX    -      X.X        -        X.X      -    X..X..X..XX  -     X.X     "
    "X....X      -  X.X  -   X.X   -   X.X    X.X    X.X   -       X.X       -       X.X       -    X..X..X..X.X -    X...X    "
    "X.....X     -  X.X  -   X.X   -  X..X    X.X    X..X  -        X.X      -      X.X        -XXX X..X..X..X..X-   X.....X   "
    "X......X    -  X.X  -   X.X   - X...XXXXXX.XXXXXX...X -         X.X   XX-XX   X.X         -X..XX........X..X-  X...X...X  "
    "X.......X   -  X.X  -   X.X   -X.....................X-          X.X X.X-X.X X.X          -X...X...........X- X...X X...X "
    "X........X  -  X.X  -   X.X   - X...XXXXXX.XXXXXX...X -           X.X..X-X..X.X           - X..............X-X...X   X...X"
    "X.........X -XXX.XXX-   X.X   -  X..X    X.X    X..X  -            X...X-X...X            -  X.............X-X..X     X..X"
    "X..........X-X.....X-   X.X   -   X.X    X.X    X.X   -           X....X-X....X           -  X.............X- XX       XX "
    "X......XXXXX-XXXXXXX-   X.X   -    XX    X.X    XX    -          X.....X-X.....X          -   X............X--------------"
    "X...X..X    ---------   X.X   -          X.X          -          XXXXXXX-XXXXXXX          -   X...........X -             "
    "X..X X..X   -       -XXXX.XXXX-       XXXX.XXXX       -------------------------------------    X..........X -             "
    "X.X  X..X   -       -X.......X-       X.......X       -    XX           XX    -           -    X..........X -             "
    "XX    X..X  -       - X.....X -        X.....X        -   X.X           X.X   -           -     X........X  -             "
    "      X..X  -       -  X...X  -         X...X         -  X..X           X..X  -           -     X........X  -             "
    "       XX   -       -   X.X   -          X.X          - X...XXXXXXXXXXXXX...X -           -     XXXXXXXXXX  -             "
    "-------------       -    X    -           X           -X.....................X-           -------------------             "
    "                    ----------------------------------- X...XXXXXXXXXXXXX...X -                                           "
    "                                                      -  X..X           X..X  -                                           "
    "                                                      -   X.X           X.X   -                                           "
    "                                                      -    XX           XX    -                                           "
};

static const ImVec2 FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[ImGuiMouseCursor_COUNT][3] =
{
    // Pos ........ Size ......... Offset ......
    { ImVec2( 0,3), ImVec2(12,19), ImVec2( 0, 0) }, // ImGuiMouseCursor_Arrow
    { ImVec2(13,0), ImVec2( 7,16), ImVec2( 1, 8) }, // ImGuiMouseCursor_TextInput
    { ImVec2(31,0), ImVec2(23,23), ImVec2(11,11) }, // ImGuiMouseCursor_ResizeAll
    { ImVec2(21,0), ImVec2( 9,23), ImVec2( 4,11) }, // ImGuiMouseCursor_ResizeNS
    { ImVec2(55,18),ImVec2(23, 9), ImVec2(11, 4) }, // ImGuiMouseCursor_ResizeEW
    { ImVec2(73,0), ImVec2(17,17), ImVec2( 8, 8) }, // ImGuiMouseCursor_ResizeNESW
    { ImVec2(55,0), ImVec2(17,17), ImVec2( 8, 8) }, // ImGuiMouseCursor_ResizeNWSE
    { ImVec2(91,0), ImVec2(17,22), ImVec2( 5, 0) }, // ImGuiMouseCursor_Hand
    { ImVec2(109,0),ImVec2(13,15), ImVec2( 6, 7) }, // ImGuiMouseCursor_NotAllowed
};

ImFontAtlas::ImFontAtlas()
{
    memset(this, 0, sizeof(*this));
    TexGlyphPadding = 1;
    PackIdMouseCursors = PackIdLines = -1;
}

ImFontAtlas::~ImFontAtlas()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    Clear();
}

void    ImFontAtlas::ClearInputData()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    for (ImFontConfig& font_cfg : ConfigData)
        if (font_cfg.FontData && font_cfg.FontDataOwnedByAtlas)
        {
            IM_FREE(font_cfg.FontData);
            font_cfg.FontData = NULL;
        }

    // When clearing this we lose access to the font name and other information used to build the font.
    for (ImFont* font : Fonts)
        if (font->ConfigData >= ConfigData.Data && font->ConfigData < ConfigData.Data + ConfigData.Size)
        {
            font->ConfigData = NULL;
            font->ConfigDataCount = 0;
        }
    ConfigData.clear();
    CustomRects.clear();
    PackIdMouseCursors = PackIdLines = -1;
    // Important: we leave TexReady untouched
}

void    ImFontAtlas::ClearTexData()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    if (TexPixelsAlpha8)
        IM_FREE(TexPixelsAlpha8);
    if (TexPixelsRGBA32)
        IM_FREE(TexPixelsRGBA32);
    TexPixelsAlpha8 = NULL;
    TexPixelsRGBA32 = NULL;
    TexPixelsUseColors = false;
    // Important: we leave TexReady untouched
}

void    ImFontAtlas::ClearFonts()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    Fonts.clear_delete();
    TexReady = false;
}

void    ImFontAtlas::Clear()
{
    ClearInputData();
    ClearTexData();
    ClearFonts();
}

void    ImFontAtlas::GetTexDataAsAlpha8(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel)
{
    // Build atlas on demand
    if (TexPixelsAlpha8 == NULL)
        Build();

    *out_pixels = TexPixelsAlpha8;
    if (out_width) *out_width = TexWidth;
    if (out_height) *out_height = TexHeight;
    if (out_bytes_per_pixel) *out_bytes_per_pixel = 1;
}

void    ImFontAtlas::GetTexDataAsRGBA32(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel)
{
    // Convert to RGBA32 format on demand
    // Although it is likely to be the most commonly used format, our font rendering is 1 channel / 8 bpp
    if (!TexPixelsRGBA32)
    {
        unsigned char* pixels = NULL;
        GetTexDataAsAlpha8(&pixels, NULL, NULL);
        if (pixels)
        {
            TexPixelsRGBA32 = (unsigned int*)IM_ALLOC((size_t)TexWidth * (size_t)TexHeight * 4);
            const unsigned char* src = pixels;
            unsigned int* dst = TexPixelsRGBA32;
            for (int n = TexWidth * TexHeight; n > 0; n--)
                *dst++ = IM_COL32(255, 255, 255, (unsigned int)(*src++));
        }
    }

    *out_pixels = (unsigned char*)TexPixelsRGBA32;
    if (out_width) *out_width = TexWidth;
    if (out_height) *out_height = TexHeight;
    if (out_bytes_per_pixel) *out_bytes_per_pixel = 4;
}

ImFont* ImFontAtlas::AddFont(const ImFontConfig* font_cfg)
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    IM_ASSERT(font_cfg->FontData != NULL && font_cfg->FontDataSize > 0);
    IM_ASSERT(font_cfg->SizePixels > 0.0f && "Is ImFontConfig struct correctly initialized?");
    IM_ASSERT(font_cfg->OversampleH > 0 && font_cfg->OversampleV > 0 && "Is ImFontConfig struct correctly initialized?");
    IM_ASSERT(font_cfg->RasterizerDensity > 0.0f);

    // Create new font
    if (!font_cfg->MergeMode)
        Fonts.push_back(IM_NEW(ImFont));
    else
        IM_ASSERT(Fonts.Size > 0 && "Cannot use MergeMode for the first font"); // When using MergeMode make sure that a font has already been added before. You can use ImGui::GetIO().Fonts->AddFontDefault() to add the default imgui font.

    ConfigData.push_back(*font_cfg);
    ImFontConfig& new_font_cfg = ConfigData.back();
    if (new_font_cfg.DstFont == NULL)
        new_font_cfg.DstFont = Fonts.back();
    if (!new_font_cfg.FontDataOwnedByAtlas)
    {
        new_font_cfg.FontData = IM_ALLOC(new_font_cfg.FontDataSize);
        new_font_cfg.FontDataOwnedByAtlas = true;
        memcpy(new_font_cfg.FontData, font_cfg->FontData, (size_t)new_font_cfg.FontDataSize);
    }

    // Round font size
    // - We started rounding in 1.90 WIP (18991) as our layout system currently doesn't support non-rounded font size well yet.
    // - Note that using io.FontGlobalScale or SetWindowFontScale(), with are legacy-ish, partially supported features, can still lead to unrounded sizes.
    // - We may support it better later and remove this rounding.
    new_font_cfg.SizePixels = ImTrunc(new_font_cfg.SizePixels);

    if (new_font_cfg.DstFont->EllipsisChar == (ImWchar)-1)
        new_font_cfg.DstFont->EllipsisChar = font_cfg->EllipsisChar;

    // Pointers to ConfigData and BuilderData are otherwise dangling
    ImFontAtlasUpdateConfigDataPointers(this);

    // Invalidate texture
    TexReady = false;
    ClearTexData();
    return new_font_cfg.DstFont;
}

// Default font TTF is compressed with stb_compress then base85 encoded (see misc/fonts/binary_to_compressed_c.cpp for encoder)
static unsigned int stb_decompress_length(const unsigned char* input);
static unsigned int stb_decompress(unsigned char* output, const unsigned char* input, unsigned int length);
static unsigned int Decode85Byte(char c)                                    { return c >= '\\' ? c-36 : c-35; }
static void         Decode85(const unsigned char* src, unsigned char* dst)
{
    while (*src)
    {
        unsigned int tmp = Decode85Byte(src[0]) + 85 * (Decode85Byte(src[1]) + 85 * (Decode85Byte(src[2]) + 85 * (Decode85Byte(src[3]) + 85 * Decode85Byte(src[4]))));
        dst[0] = ((tmp >> 0) & 0xFF); dst[1] = ((tmp >> 8) & 0xFF); dst[2] = ((tmp >> 16) & 0xFF); dst[3] = ((tmp >> 24) & 0xFF);   // We can't assume little-endianness.
        src += 5;
        dst += 4;
    }
}
#ifndef IMGUI_DISABLE_DEFAULT_FONT
static const char* GetDefaultCompressedFontDataTTF(int* out_size);
#endif

// Load embedded ProggyClean.ttf at size 13, disable oversampling
ImFont* ImFontAtlas::AddFontDefault(const ImFontConfig* font_cfg_template)
{
#ifndef IMGUI_DISABLE_DEFAULT_FONT
    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    if (!font_cfg_template)
    {
        font_cfg.OversampleH = font_cfg.OversampleV = 1;
        font_cfg.PixelSnapH = true;
    }
    if (font_cfg.SizePixels <= 0.0f)
        font_cfg.SizePixels = 13.0f * 1.0f;
    if (font_cfg.Name[0] == '\0')
        ImFormatString(font_cfg.Name, IM_ARRAYSIZE(font_cfg.Name), "ProggyClean.ttf, %dpx", (int)font_cfg.SizePixels);
    font_cfg.EllipsisChar = (ImWchar)0x0085;
    font_cfg.GlyphOffset.y = 1.0f * IM_TRUNC(font_cfg.SizePixels / 13.0f);  // Add +1 offset per 13 units

    int ttf_compressed_size = 0;
    const char* ttf_compressed = GetDefaultCompressedFontDataTTF(&ttf_compressed_size);
    const ImWchar* glyph_ranges = font_cfg.GlyphRanges != NULL ? font_cfg.GlyphRanges : GetGlyphRangesDefault();
    ImFont* font = AddFontFromMemoryCompressedTTF(ttf_compressed, ttf_compressed_size, font_cfg.SizePixels, &font_cfg, glyph_ranges);
    return font;
#else
    IM_ASSERT(0 && "AddFontDefault() disabled in this build.");
    IM_UNUSED(font_cfg_template);
    return NULL;
#endif // #ifndef IMGUI_DISABLE_DEFAULT_FONT
}

ImFont* ImFontAtlas::AddFontFromFileTTF(const char* filename, float size_pixels, const ImFontConfig* font_cfg_template, const ImWchar* glyph_ranges)
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    size_t data_size = 0;
    void* data = ImFileLoadToMemory(filename, "rb", &data_size, 0);
    if (!data)
    {
        IM_ASSERT_USER_ERROR(0, "Could not load font file!");
        return NULL;
    }
    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    if (font_cfg.Name[0] == '\0')
    {
        // Store a short copy of filename into into the font name for convenience
        const char* p;
        for (p = filename + strlen(filename); p > filename && p[-1] != '/' && p[-1] != '\\'; p--) {}
        ImFormatString(font_cfg.Name, IM_ARRAYSIZE(font_cfg.Name), "%s, %.0fpx", p, size_pixels);
    }
    return AddFontFromMemoryTTF(data, (int)data_size, size_pixels, &font_cfg, glyph_ranges);
}

// NB: Transfer ownership of 'ttf_data' to ImFontAtlas, unless font_cfg_template->FontDataOwnedByAtlas == false. Owned TTF buffer will be deleted after Build().
ImFont* ImFontAtlas::AddFontFromMemoryTTF(void* font_data, int font_data_size, float size_pixels, const ImFontConfig* font_cfg_template, const ImWchar* glyph_ranges)
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    IM_ASSERT(font_cfg.FontData == NULL);
    IM_ASSERT(font_data_size > 100 && "Incorrect value for font_data_size!"); // Heuristic to prevent accidentally passing a wrong value to font_data_size.
    font_cfg.FontData = font_data;
    font_cfg.FontDataSize = font_data_size;
    font_cfg.SizePixels = size_pixels > 0.0f ? size_pixels : font_cfg.SizePixels;
    if (glyph_ranges)
        font_cfg.GlyphRanges = glyph_ranges;
    return AddFont(&font_cfg);
}

ImFont* ImFontAtlas::AddFontFromMemoryCompressedTTF(const void* compressed_ttf_data, int compressed_ttf_size, float size_pixels, const ImFontConfig* font_cfg_template, const ImWchar* glyph_ranges)
{
    const unsigned int buf_decompressed_size = stb_decompress_length((const unsigned char*)compressed_ttf_data);
    unsigned char* buf_decompressed_data = (unsigned char*)IM_ALLOC(buf_decompressed_size);
    stb_decompress(buf_decompressed_data, (const unsigned char*)compressed_ttf_data, (unsigned int)compressed_ttf_size);

    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    IM_ASSERT(font_cfg.FontData == NULL);
    font_cfg.FontDataOwnedByAtlas = true;
    return AddFontFromMemoryTTF(buf_decompressed_data, (int)buf_decompressed_size, size_pixels, &font_cfg, glyph_ranges);
}

ImFont* ImFontAtlas::AddFontFromMemoryCompressedBase85TTF(const char* compressed_ttf_data_base85, float size_pixels, const ImFontConfig* font_cfg, const ImWchar* glyph_ranges)
{
    int compressed_ttf_size = (((int)strlen(compressed_ttf_data_base85) + 4) / 5) * 4;
    void* compressed_ttf = IM_ALLOC((size_t)compressed_ttf_size);
    Decode85((const unsigned char*)compressed_ttf_data_base85, (unsigned char*)compressed_ttf);
    ImFont* font = AddFontFromMemoryCompressedTTF(compressed_ttf, compressed_ttf_size, size_pixels, font_cfg, glyph_ranges);
    IM_FREE(compressed_ttf);
    return font;
}

int ImFontAtlas::AddCustomRectRegular(int width, int height)
{
    IM_ASSERT(width > 0 && width <= 0xFFFF);
    IM_ASSERT(height > 0 && height <= 0xFFFF);
    ImFontAtlasCustomRect r;
    r.Width = (unsigned short)width;
    r.Height = (unsigned short)height;
    CustomRects.push_back(r);
    return CustomRects.Size - 1; // Return index
}

int ImFontAtlas::AddCustomRectFontGlyph(ImFont* font, ImWchar id, int width, int height, float advance_x, const ImVec2& offset)
{
#ifdef IMGUI_USE_WCHAR32
    IM_ASSERT(id <= IM_UNICODE_CODEPOINT_MAX);
#endif
    IM_ASSERT(font != NULL);
    IM_ASSERT(width > 0 && width <= 0xFFFF);
    IM_ASSERT(height > 0 && height <= 0xFFFF);
    ImFontAtlasCustomRect r;
    r.Width = (unsigned short)width;
    r.Height = (unsigned short)height;
    r.GlyphID = id;
    r.GlyphColored = 0; // Set to 1 manually to mark glyph as colored // FIXME: No official API for that (#8133)
    r.GlyphAdvanceX = advance_x;
    r.GlyphOffset = offset;
    r.Font = font;
    CustomRects.push_back(r);
    return CustomRects.Size - 1; // Return index
}

void ImFontAtlas::CalcCustomRectUV(const ImFontAtlasCustomRect* rect, ImVec2* out_uv_min, ImVec2* out_uv_max) const
{
    IM_ASSERT(TexWidth > 0 && TexHeight > 0);   // Font atlas needs to be built before we can calculate UV coordinates
    IM_ASSERT(rect->IsPacked());                // Make sure the rectangle has been packed
    *out_uv_min = ImVec2((float)rect->X * TexUvScale.x, (float)rect->Y * TexUvScale.y);
    *out_uv_max = ImVec2((float)(rect->X + rect->Width) * TexUvScale.x, (float)(rect->Y + rect->Height) * TexUvScale.y);
}

bool ImFontAtlas::GetMouseCursorTexData(ImGuiMouseCursor cursor_type, ImVec2* out_offset, ImVec2* out_size, ImVec2 out_uv_border[2], ImVec2 out_uv_fill[2])
{
    if (cursor_type <= ImGuiMouseCursor_None || cursor_type >= ImGuiMouseCursor_COUNT)
        return false;
    if (Flags & ImFontAtlasFlags_NoMouseCursors)
        return false;

    IM_ASSERT(PackIdMouseCursors != -1);
    ImFontAtlasCustomRect* r = GetCustomRectByIndex(PackIdMouseCursors);
    ImVec2 pos = FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[cursor_type][0] + ImVec2((float)r->X, (float)r->Y);
    ImVec2 size = FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[cursor_type][1];
    *out_size = size;
    *out_offset = FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[cursor_type][2];
    out_uv_border[0] = (pos) * TexUvScale;
    out_uv_border[1] = (pos + size) * TexUvScale;
    pos.x += FONT_ATLAS_DEFAULT_TEX_DATA_W + 1;
    out_uv_fill[0] = (pos) * TexUvScale;
    out_uv_fill[1] = (pos + size) * TexUvScale;
    return true;
}

bool    ImFontAtlas::Build()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");

    // Default font is none are specified
    if (ConfigData.Size == 0)
        AddFontDefault();

    // Select builder
    // - Note that we do not reassign to atlas->FontBuilderIO, since it is likely to point to static data which
    //   may mess with some hot-reloading schemes. If you need to assign to this (for dynamic selection) AND are
    //   using a hot-reloading scheme that messes up static data, store your own instance of ImFontBuilderIO somewhere
    //   and point to it instead of pointing directly to return value of the GetBuilderXXX functions.
    const ImFontBuilderIO* builder_io = FontBuilderIO;
    if (builder_io == NULL)
    {
#ifdef IMGUI_ENABLE_FREETYPE
        builder_io = ImGuiFreeType::GetBuilderForFreeType();
#elif defined(IMGUI_ENABLE_STB_TRUETYPE)
        builder_io = ImFontAtlasGetBuilderForStbTruetype();
#else
        IM_ASSERT(0); // Invalid Build function
#endif
    }

    // Build
    return builder_io->FontBuilder_Build(this);
}

void    ImFontAtlasBuildMultiplyCalcLookupTable(unsigned char out_table[256], float in_brighten_factor)
{
    for (unsigned int i = 0; i < 256; i++)
    {
        unsigned int value = (unsigned int)(i * in_brighten_factor);
        out_table[i] = value > 255 ? 255 : (value & 0xFF);
    }
}

void    ImFontAtlasBuildMultiplyRectAlpha8(const unsigned char table[256], unsigned char* pixels, int x, int y, int w, int h, int stride)
{
    IM_ASSERT_PARANOID(w <= stride);
    unsigned char* data = pixels + x + y * stride;
    for (int j = h; j > 0; j--, data += stride - w)
        for (int i = w; i > 0; i--, data++)
            *data = table[*data];
}

#ifdef IMGUI_ENABLE_STB_TRUETYPE
// Temporary data for one source font (multiple source fonts can be merged into one destination ImFont)
// (C++03 doesn't allow instancing ImVector<> with function-local types so we declare the type here.)
struct ImFontBuildSrcData
{
    stbtt_fontinfo      FontInfo;
    stbtt_pack_range    PackRange;          // Hold the list of codepoints to pack (essentially points to Codepoints.Data)
    stbrp_rect*         Rects;              // Rectangle to pack. We first fill in their size and the packer will give us their position.
    stbtt_packedchar*   PackedChars;        // Output glyphs
    const ImWchar*      SrcRanges;          // Ranges as requested by user (user is allowed to request too much, e.g. 0x0020..0xFFFF)
    int                 DstIndex;           // Index into atlas->Fonts[] and dst_tmp_array[]
    int                 GlyphsHighest;      // Highest requested codepoint
    int                 GlyphsCount;        // Glyph count (excluding missing glyphs and glyphs already set by an earlier source font)
    ImBitVector         GlyphsSet;          // Glyph bit map (random access, 1-bit per codepoint. This will be a maximum of 8KB)
    ImVector<int>       GlyphsList;         // Glyph codepoints list (flattened version of GlyphsSet)
};

// Temporary data for one destination ImFont* (multiple source fonts can be merged into one destination ImFont)
struct ImFontBuildDstData
{
    int                 SrcCount;           // Number of source fonts targeting this destination font.
    int                 GlyphsHighest;
    int                 GlyphsCount;
    ImBitVector         GlyphsSet;          // This is used to resolve collision when multiple sources are merged into a same destination font.
};

static void UnpackBitVectorToFlatIndexList(const ImBitVector* in, ImVector<int>* out)
{
    IM_ASSERT(sizeof(in->Storage.Data[0]) == sizeof(int));
    const ImU32* it_begin = in->Storage.begin();
    const ImU32* it_end = in->Storage.end();
    for (const ImU32* it = it_begin; it < it_end; it++)
        if (ImU32 entries_32 = *it)
            for (ImU32 bit_n = 0; bit_n < 32; bit_n++)
                if (entries_32 & ((ImU32)1 << bit_n))
                    out->push_back((int)(((it - it_begin) << 5) + bit_n));
}

static bool ImFontAtlasBuildWithStbTruetype(ImFontAtlas* atlas)
{
    IM_ASSERT(atlas->ConfigData.Size > 0);

    ImFontAtlasBuildInit(atlas);

    // Clear atlas
    atlas->TexID = (ImTextureID)NULL;
    atlas->TexWidth = atlas->TexHeight = 0;
    atlas->TexUvScale = ImVec2(0.0f, 0.0f);
    atlas->TexUvWhitePixel = ImVec2(0.0f, 0.0f);
    atlas->ClearTexData();

    // Temporary storage for building
    ImVector<ImFontBuildSrcData> src_tmp_array;
    ImVector<ImFontBuildDstData> dst_tmp_array;
    src_tmp_array.resize(atlas->ConfigData.Size);
    dst_tmp_array.resize(atlas->Fonts.Size);
    memset(src_tmp_array.Data, 0, (size_t)src_tmp_array.size_in_bytes());
    memset(dst_tmp_array.Data, 0, (size_t)dst_tmp_array.size_in_bytes());

    // 1. Initialize font loading structure, check font data validity
    for (int src_i = 0; src_i < atlas->ConfigData.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        ImFontConfig& cfg = atlas->ConfigData[src_i];
        IM_ASSERT(cfg.DstFont && (!cfg.DstFont->IsLoaded() || cfg.DstFont->ContainerAtlas == atlas));

        // Find index from cfg.DstFont (we allow the user to set cfg.DstFont. Also it makes casual debugging nicer than when storing indices)
        src_tmp.DstIndex = -1;
        for (int output_i = 0; output_i < atlas->Fonts.Size && src_tmp.DstIndex == -1; output_i++)
            if (cfg.DstFont == atlas->Fonts[output_i])
                src_tmp.DstIndex = output_i;
        if (src_tmp.DstIndex == -1)
        {
            IM_ASSERT(src_tmp.DstIndex != -1); // cfg.DstFont not pointing within atlas->Fonts[] array?
            return false;
        }
        // Initialize helper structure for font loading and verify that the TTF/OTF data is correct
        const int font_offset = stbtt_GetFontOffsetForIndex((unsigned char*)cfg.FontData, cfg.FontNo);
        IM_ASSERT(font_offset >= 0 && "FontData is incorrect, or FontNo cannot be found.");
        if (!stbtt_InitFont(&src_tmp.FontInfo, (unsigned char*)cfg.FontData, font_offset))
        {
            IM_ASSERT(0 && "stbtt_InitFont(): failed to parse FontData. It is correct and complete? Check FontDataSize.");
            return false;
        }

        // Measure highest codepoints
        ImFontBuildDstData& dst_tmp = dst_tmp_array[src_tmp.DstIndex];
        src_tmp.SrcRanges = cfg.GlyphRanges ? cfg.GlyphRanges : atlas->GetGlyphRangesDefault();
        for (const ImWchar* src_range = src_tmp.SrcRanges; src_range[0] && src_range[1]; src_range += 2)
        {
            // Check for valid range. This may also help detect *some* dangling pointers, because a common
            // user error is to setup ImFontConfig::GlyphRanges with a pointer to data that isn't persistent,
            // or to forget to zero-terminate the glyph range array.
            IM_ASSERT(src_range[0] <= src_range[1] && "Invalid range: is your glyph range array persistent? it is zero-terminated?");
            src_tmp.GlyphsHighest = ImMax(src_tmp.GlyphsHighest, (int)src_range[1]);
        }
        dst_tmp.SrcCount++;
        dst_tmp.GlyphsHighest = ImMax(dst_tmp.GlyphsHighest, src_tmp.GlyphsHighest);
    }

    // 2. For every requested codepoint, check for their presence in the font data, and handle redundancy or overlaps between source fonts to avoid unused glyphs.
    int total_glyphs_count = 0;
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        ImFontBuildDstData& dst_tmp = dst_tmp_array[src_tmp.DstIndex];
        src_tmp.GlyphsSet.Create(src_tmp.GlyphsHighest + 1);
        if (dst_tmp.GlyphsSet.Storage.empty())
            dst_tmp.GlyphsSet.Create(dst_tmp.GlyphsHighest + 1);

        for (const ImWchar* src_range = src_tmp.SrcRanges; src_range[0] && src_range[1]; src_range += 2)
            for (unsigned int codepoint = src_range[0]; codepoint <= src_range[1]; codepoint++)
            {
                if (dst_tmp.GlyphsSet.TestBit(codepoint))    // Don't overwrite existing glyphs. We could make this an option for MergeMode (e.g. MergeOverwrite==true)
                    continue;
                if (!stbtt_FindGlyphIndex(&src_tmp.FontInfo, codepoint))    // It is actually in the font?
                    continue;

                // Add to avail set/counters
                src_tmp.GlyphsCount++;
                dst_tmp.GlyphsCount++;
                src_tmp.GlyphsSet.SetBit(codepoint);
                dst_tmp.GlyphsSet.SetBit(codepoint);
                total_glyphs_count++;
            }
    }

    // 3. Unpack our bit map into a flat list (we now have all the Unicode points that we know are requested _and_ available _and_ not overlapping another)
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        src_tmp.GlyphsList.reserve(src_tmp.GlyphsCount);
        UnpackBitVectorToFlatIndexList(&src_tmp.GlyphsSet, &src_tmp.GlyphsList);
        src_tmp.GlyphsSet.Clear();
        IM_ASSERT(src_tmp.GlyphsList.Size == src_tmp.GlyphsCount);
    }
    for (int dst_i = 0; dst_i < dst_tmp_array.Size; dst_i++)
        dst_tmp_array[dst_i].GlyphsSet.Clear();
    dst_tmp_array.clear();

    // Allocate packing character data and flag packed characters buffer as non-packed (x0=y0=x1=y1=0)
    // (We technically don't need to zero-clear buf_rects, but let's do it for the sake of sanity)
    ImVector<stbrp_rect> buf_rects;
    ImVector<stbtt_packedchar> buf_packedchars;
    buf_rects.resize(total_glyphs_count);
    buf_packedchars.resize(total_glyphs_count);
    memset(buf_rects.Data, 0, (size_t)buf_rects.size_in_bytes());
    memset(buf_packedchars.Data, 0, (size_t)buf_packedchars.size_in_bytes());

    // 4. Gather glyphs sizes so we can pack them in our virtual canvas.
    int total_surface = 0;
    int buf_rects_out_n = 0;
    int buf_packedchars_out_n = 0;
    const int pack_padding = atlas->TexGlyphPadding;
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        if (src_tmp.GlyphsCount == 0)
            continue;

        src_tmp.Rects = &buf_rects[buf_rects_out_n];
        src_tmp.PackedChars = &buf_packedchars[buf_packedchars_out_n];
        buf_rects_out_n += src_tmp.GlyphsCount;
        buf_packedchars_out_n += src_tmp.GlyphsCount;

        // Convert our ranges in the format stb_truetype wants
        ImFontConfig& cfg = atlas->ConfigData[src_i];
        src_tmp.PackRange.font_size = cfg.SizePixels * cfg.RasterizerDensity;
        src_tmp.PackRange.first_unicode_codepoint_in_range = 0;
        src_tmp.PackRange.array_of_unicode_codepoints = src_tmp.GlyphsList.Data;
        src_tmp.PackRange.num_chars = src_tmp.GlyphsList.Size;
        src_tmp.PackRange.chardata_for_range = src_tmp.PackedChars;
        src_tmp.PackRange.h_oversample = (unsigned char)cfg.OversampleH;
        src_tmp.PackRange.v_oversample = (unsigned char)cfg.OversampleV;

        // Gather the sizes of all rectangles we will need to pack (this loop is based on stbtt_PackFontRangesGatherRects)
        const float scale = (cfg.SizePixels > 0.0f) ? stbtt_ScaleForPixelHeight(&src_tmp.FontInfo, cfg.SizePixels * cfg.RasterizerDensity) : stbtt_ScaleForMappingEmToPixels(&src_tmp.FontInfo, -cfg.SizePixels * cfg.RasterizerDensity);
        for (int glyph_i = 0; glyph_i < src_tmp.GlyphsList.Size; glyph_i++)
        {
            int x0, y0, x1, y1;
            const int glyph_index_in_font = stbtt_FindGlyphIndex(&src_tmp.FontInfo, src_tmp.GlyphsList[glyph_i]);
            IM_ASSERT(glyph_index_in_font != 0);
            stbtt_GetGlyphBitmapBoxSubpixel(&src_tmp.FontInfo, glyph_index_in_font, scale * cfg.OversampleH, scale * cfg.OversampleV, 0, 0, &x0, &y0, &x1, &y1);
            src_tmp.Rects[glyph_i].w = (stbrp_coord)(x1 - x0 + pack_padding + cfg.OversampleH - 1);
            src_tmp.Rects[glyph_i].h = (stbrp_coord)(y1 - y0 + pack_padding + cfg.OversampleV - 1);
            total_surface += src_tmp.Rects[glyph_i].w * src_tmp.Rects[glyph_i].h;
        }
    }
    for (int i = 0; i < atlas->CustomRects.Size; i++)
        total_surface += (atlas->CustomRects[i].Width + pack_padding) * (atlas->CustomRects[i].Height + pack_padding);

    // We need a width for the skyline algorithm, any width!
    // The exact width doesn't really matter much, but some API/GPU have texture size limitations and increasing width can decrease height.
    // User can override TexDesiredWidth and TexGlyphPadding if they wish, otherwise we use a simple heuristic to select the width based on expected surface.
    const int surface_sqrt = (int)ImSqrt((float)total_surface) + 1;
    atlas->TexHeight = 0;
    if (atlas->TexDesiredWidth > 0)
        atlas->TexWidth = atlas->TexDesiredWidth;
    else
        atlas->TexWidth = (surface_sqrt >= 4096 * 0.7f) ? 4096 : (surface_sqrt >= 2048 * 0.7f) ? 2048 : (surface_sqrt >= 1024 * 0.7f) ? 1024 : 512;

    // 5. Start packing
    // Pack our extra data rectangles first, so it will be on the upper-left corner of our texture (UV will have small values).
    const int TEX_HEIGHT_MAX = 1024 * 32;
    stbtt_pack_context spc = {};
    stbtt_PackBegin(&spc, NULL, atlas->TexWidth, TEX_HEIGHT_MAX, 0, 0, NULL);
    spc.padding = atlas->TexGlyphPadding; // Because we mixup stbtt_PackXXX and stbrp_PackXXX there's a bit of a hack here, not passing the value to stbtt_PackBegin() allows us to still pack a TexWidth-1 wide item. (#8107)
    ImFontAtlasBuildPackCustomRects(atlas, spc.pack_info);

    // 6. Pack each source font. No rendering yet, we are working with rectangles in an infinitely tall texture at this point.
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        if (src_tmp.GlyphsCount == 0)
            continue;

        stbrp_pack_rects((stbrp_context*)spc.pack_info, src_tmp.Rects, src_tmp.GlyphsCount);

        // Extend texture height and mark missing glyphs as non-packed so we won't render them.
        // FIXME: We are not handling packing failure here (would happen if we got off TEX_HEIGHT_MAX or if a single if larger than TexWidth?)
        for (int glyph_i = 0; glyph_i < src_tmp.GlyphsCount; glyph_i++)
            if (src_tmp.Rects[glyph_i].was_packed)
                atlas->TexHeight = ImMax(atlas->TexHeight, src_tmp.Rects[glyph_i].y + src_tmp.Rects[glyph_i].h);
    }

    // 7. Allocate texture
    atlas->TexHeight = (atlas->Flags & ImFontAtlasFlags_NoPowerOfTwoHeight) ? (atlas->TexHeight + 1) : ImUpperPowerOfTwo(atlas->TexHeight);
    atlas->TexUvScale = ImVec2(1.0f / atlas->TexWidth, 1.0f / atlas->TexHeight);
    atlas->TexPixelsAlpha8 = (unsigned char*)IM_ALLOC(atlas->TexWidth * atlas->TexHeight);
    memset(atlas->TexPixelsAlpha8, 0, atlas->TexWidth * atlas->TexHeight);
    spc.pixels = atlas->TexPixelsAlpha8;
    spc.height = atlas->TexHeight;

    // 8. Render/rasterize font characters into the texture
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontConfig& cfg = atlas->ConfigData[src_i];
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        if (src_tmp.GlyphsCount == 0)
            continue;

        stbtt_PackFontRangesRenderIntoRects(&spc, &src_tmp.FontInfo, &src_tmp.PackRange, 1, src_tmp.Rects);

        // Apply multiply operator
        if (cfg.RasterizerMultiply != 1.0f)
        {
            unsigned char multiply_table[256];
            ImFontAtlasBuildMultiplyCalcLookupTable(multiply_table, cfg.RasterizerMultiply);
            stbrp_rect* r = &src_tmp.Rects[0];
            for (int glyph_i = 0; glyph_i < src_tmp.GlyphsCount; glyph_i++, r++)
                if (r->was_packed)
                    ImFontAtlasBuildMultiplyRectAlpha8(multiply_table, atlas->TexPixelsAlpha8, r->x, r->y, r->w, r->h, atlas->TexWidth * 1);
        }
        src_tmp.Rects = NULL;
    }

    // End packing
    stbtt_PackEnd(&spc);
    buf_rects.clear();

    // 9. Setup ImFont and glyphs for runtime
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        // When merging fonts with MergeMode=true:
        // - We can have multiple input fonts writing into a same destination font.
        // - dst_font->ConfigData is != from cfg which is our source configuration.
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        ImFontConfig& cfg = atlas->ConfigData[src_i];
        ImFont* dst_font = cfg.DstFont;

        const float font_scale = stbtt_ScaleForPixelHeight(&src_tmp.FontInfo, cfg.SizePixels);
        int unscaled_ascent, unscaled_descent, unscaled_line_gap;
        stbtt_GetFontVMetrics(&src_tmp.FontInfo, &unscaled_ascent, &unscaled_descent, &unscaled_line_gap);

        const float ascent = ImCeil(unscaled_ascent * font_scale);
        const float descent = ImFloor(unscaled_descent * font_scale);
        ImFontAtlasBuildSetupFont(atlas, dst_font, &cfg, ascent, descent);
        const float font_off_x = cfg.GlyphOffset.x;
        const float font_off_y = cfg.GlyphOffset.y + IM_ROUND(dst_font->Ascent);

        const float inv_rasterization_scale = 1.0f / cfg.RasterizerDensity;

        for (int glyph_i = 0; glyph_i < src_tmp.GlyphsCount; glyph_i++)
        {
            // Register glyph
            const int codepoint = src_tmp.GlyphsList[glyph_i];
            const stbtt_packedchar& pc = src_tmp.PackedChars[glyph_i];
            stbtt_aligned_quad q;
            float unused_x = 0.0f, unused_y = 0.0f;
            stbtt_GetPackedQuad(src_tmp.PackedChars, atlas->TexWidth, atlas->TexHeight, glyph_i, &unused_x, &unused_y, &q, 0);
            float x0 = q.x0 * inv_rasterization_scale + font_off_x;
            float y0 = q.y0 * inv_rasterization_scale + font_off_y;
            float x1 = q.x1 * inv_rasterization_scale + font_off_x;
            float y1 = q.y1 * inv_rasterization_scale + font_off_y;
            dst_font->AddGlyph(&cfg, (ImWchar)codepoint, x0, y0, x1, y1, q.s0, q.t0, q.s1, q.t1, pc.xadvance * inv_rasterization_scale);
        }
    }

    // Cleanup
    src_tmp_array.clear_destruct();

    ImFontAtlasBuildFinish(atlas);
    return true;
}

const ImFontBuilderIO* ImFontAtlasGetBuilderForStbTruetype()
{
    static ImFontBuilderIO io;
    io.FontBuilder_Build = ImFontAtlasBuildWithStbTruetype;
    return &io;
}

#endif // IMGUI_ENABLE_STB_TRUETYPE

void ImFontAtlasUpdateConfigDataPointers(ImFontAtlas* atlas)
{
    for (ImFontConfig& font_cfg : atlas->ConfigData)
    {
        ImFont* font = font_cfg.DstFont;
        if (!font_cfg.MergeMode)
        {
            font->ConfigData = &font_cfg;
            font->ConfigDataCount = 0;
        }
        font->ConfigDataCount++;
    }
}

void ImFontAtlasBuildSetupFont(ImFontAtlas* atlas, ImFont* font, ImFontConfig* font_config, float ascent, float descent)
{
    if (!font_config->MergeMode)
    {
        font->ClearOutputData();
        font->FontSize = font_config->SizePixels;
        IM_ASSERT(font->ConfigData == font_config);
        font->ContainerAtlas = atlas;
        font->Ascent = ascent;
        font->Descent = descent;
    }
}

void ImFontAtlasBuildPackCustomRects(ImFontAtlas* atlas, void* stbrp_context_opaque)
{
    stbrp_context* pack_context = (stbrp_context*)stbrp_context_opaque;
    IM_ASSERT(pack_context != NULL);

    ImVector<ImFontAtlasCustomRect>& user_rects = atlas->CustomRects;
    IM_ASSERT(user_rects.Size >= 1); // We expect at least the default custom rects to be registered, else something went wrong.
#ifdef __GNUC__
    if (user_rects.Size < 1) { __builtin_unreachable(); } // Workaround for GCC bug if IM_ASSERT() is defined to conditionally throw (see #5343)
#endif

    const int pack_padding = atlas->TexGlyphPadding;
    ImVector<stbrp_rect> pack_rects;
    pack_rects.resize(user_rects.Size);
    memset(pack_rects.Data, 0, (size_t)pack_rects.size_in_bytes());
    for (int i = 0; i < user_rects.Size; i++)
    {
        pack_rects[i].w = user_rects[i].Width + pack_padding;
        pack_rects[i].h = user_rects[i].Height + pack_padding;
    }
    stbrp_pack_rects(pack_context, &pack_rects[0], pack_rects.Size);
    for (int i = 0; i < pack_rects.Size; i++)
        if (pack_rects[i].was_packed)
        {
            user_rects[i].X = (unsigned short)pack_rects[i].x;
            user_rects[i].Y = (unsigned short)pack_rects[i].y;
            IM_ASSERT(pack_rects[i].w == user_rects[i].Width + pack_padding && pack_rects[i].h == user_rects[i].Height + pack_padding);
            atlas->TexHeight = ImMax(atlas->TexHeight, pack_rects[i].y + pack_rects[i].h);
        }
}

void ImFontAtlasBuildRender8bppRectFromString(ImFontAtlas* atlas, int x, int y, int w, int h, const char* in_str, char in_marker_char, unsigned char in_marker_pixel_value)
{
    IM_ASSERT(x >= 0 && x + w <= atlas->TexWidth);
    IM_ASSERT(y >= 0 && y + h <= atlas->TexHeight);
    unsigned char* out_pixel = atlas->TexPixelsAlpha8 + x + (y * atlas->TexWidth);
    for (int off_y = 0; off_y < h; off_y++, out_pixel += atlas->TexWidth, in_str += w)
        for (int off_x = 0; off_x < w; off_x++)
            out_pixel[off_x] = (in_str[off_x] == in_marker_char) ? in_marker_pixel_value : 0x00;
}

void ImFontAtlasBuildRender32bppRectFromString(ImFontAtlas* atlas, int x, int y, int w, int h, const char* in_str, char in_marker_char, unsigned int in_marker_pixel_value)
{
    IM_ASSERT(x >= 0 && x + w <= atlas->TexWidth);
    IM_ASSERT(y >= 0 && y + h <= atlas->TexHeight);
    unsigned int* out_pixel = atlas->TexPixelsRGBA32 + x + (y * atlas->TexWidth);
    for (int off_y = 0; off_y < h; off_y++, out_pixel += atlas->TexWidth, in_str += w)
        for (int off_x = 0; off_x < w; off_x++)
            out_pixel[off_x] = (in_str[off_x] == in_marker_char) ? in_marker_pixel_value : IM_COL32_BLACK_TRANS;
}

static void ImFontAtlasBuildRenderDefaultTexData(ImFontAtlas* atlas)
{
    ImFontAtlasCustomRect* r = atlas->GetCustomRectByIndex(atlas->PackIdMouseCursors);
    IM_ASSERT(r->IsPacked());

    const int w = atlas->TexWidth;
    if (atlas->Flags & ImFontAtlasFlags_NoMouseCursors)
    {
        // White pixels only
        IM_ASSERT(r->Width == 2 && r->Height == 2);
        const int offset = (int)r->X + (int)r->Y * w;
        if (atlas->TexPixelsAlpha8 != NULL)
        {
            atlas->TexPixelsAlpha8[offset] = atlas->TexPixelsAlpha8[offset + 1] = atlas->TexPixelsAlpha8[offset + w] = atlas->TexPixelsAlpha8[offset + w + 1] = 0xFF;
        }
        else
        {
            atlas->TexPixelsRGBA32[offset] = atlas->TexPixelsRGBA32[offset + 1] = atlas->TexPixelsRGBA32[offset + w] = atlas->TexPixelsRGBA32[offset + w + 1] = IM_COL32_WHITE;
        }
    }
    else
    {
        // White pixels and mouse cursor
        IM_ASSERT(r->Width == FONT_ATLAS_DEFAULT_TEX_DATA_W * 2 + 1 && r->Height == FONT_ATLAS_DEFAULT_TEX_DATA_H);
        const int x_for_white = r->X;
        const int x_for_black = r->X + FONT_ATLAS_DEFAULT_TEX_DATA_W + 1;
        if (atlas->TexPixelsAlpha8 != NULL)
        {
            ImFontAtlasBuildRender8bppRectFromString(atlas, x_for_white, r->Y, FONT_ATLAS_DEFAULT_TEX_DATA_W, FONT_ATLAS_DEFAULT_TEX_DATA_H, FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS, '.', 0xFF);
            ImFontAtlasBuildRender8bppRectFromString(atlas, x_for_black, r->Y, FONT_ATLAS_DEFAULT_TEX_DATA_W, FONT_ATLAS_DEFAULT_TEX_DATA_H, FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS, 'X', 0xFF);
        }
        else
        {
            ImFontAtlasBuildRender32bppRectFromString(atlas, x_for_white, r->Y, FONT_ATLAS_DEFAULT_TEX_DATA_W, FONT_ATLAS_DEFAULT_TEX_DATA_H, FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS, '.', IM_COL32_WHITE);
            ImFontAtlasBuildRender32bppRectFromString(atlas, x_for_black, r->Y, FONT_ATLAS_DEFAULT_TEX_DATA_W, FONT_ATLAS_DEFAULT_TEX_DATA_H, FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS, 'X', IM_COL32_WHITE);
        }
    }
    atlas->TexUvWhitePixel = ImVec2((r->X + 0.5f) * atlas->TexUvScale.x, (r->Y + 0.5f) * atlas->TexUvScale.y);
}

static void ImFontAtlasBuildRenderLinesTexData(ImFontAtlas* atlas)
{
    if (atlas->Flags & ImFontAtlasFlags_NoBakedLines)
        return;

    // This generates a triangular shape in the texture, with the various line widths stacked on top of each other to allow interpolation between them
    ImFontAtlasCustomRect* r = atlas->GetCustomRectByIndex(atlas->PackIdLines);
    IM_ASSERT(r->IsPacked());
    for (int n = 0; n < IM_DRAWLIST_TEX_LINES_WIDTH_MAX + 1; n++) // +1 because of the zero-width row
    {
        // Each line consists of at least two empty pixels at the ends, with a line of solid pixels in the middle
        int y = n;
        int line_width = n;
        int pad_left = (r->Width - line_width) / 2;
        int pad_right = r->Width - (pad_left + line_width);

        // Write each slice
        IM_ASSERT(pad_left + line_width + pad_right == r->Width && y < r->Height); // Make sure we're inside the texture bounds before we start writing pixels
        if (atlas->TexPixelsAlpha8 != NULL)
        {
            unsigned char* write_ptr = &atlas->TexPixelsAlpha8[r->X + ((r->Y + y) * atlas->TexWidth)];
            for (int i = 0; i < pad_left; i++)
                *(write_ptr + i) = 0x00;

            for (int i = 0; i < line_width; i++)
                *(write_ptr + pad_left + i) = 0xFF;

            for (int i = 0; i < pad_right; i++)
                *(write_ptr + pad_left + line_width + i) = 0x00;
        }
        else
        {
            unsigned int* write_ptr = &atlas->TexPixelsRGBA32[r->X + ((r->Y + y) * atlas->TexWidth)];
            for (int i = 0; i < pad_left; i++)
                *(write_ptr + i) = IM_COL32(255, 255, 255, 0);

            for (int i = 0; i < line_width; i++)
                *(write_ptr + pad_left + i) = IM_COL32_WHITE;

            for (int i = 0; i < pad_right; i++)
                *(write_ptr + pad_left + line_width + i) = IM_COL32(255, 255, 255, 0);
        }

        // Calculate UVs for this line
        ImVec2 uv0 = ImVec2((float)(r->X + pad_left - 1), (float)(r->Y + y)) * atlas->TexUvScale;
        ImVec2 uv1 = ImVec2((float)(r->X + pad_left + line_width + 1), (float)(r->Y + y + 1)) * atlas->TexUvScale;
        float half_v = (uv0.y + uv1.y) * 0.5f; // Calculate a constant V in the middle of the row to avoid sampling artifacts
        atlas->TexUvLines[n] = ImVec4(uv0.x, half_v, uv1.x, half_v);
    }
}

// Note: this is called / shared by both the stb_truetype and the FreeType builder
void ImFontAtlasBuildInit(ImFontAtlas* atlas)
{
    // Register texture region for mouse cursors or standard white pixels
    if (atlas->PackIdMouseCursors < 0)
    {
        if (!(atlas->Flags & ImFontAtlasFlags_NoMouseCursors))
            atlas->PackIdMouseCursors = atlas->AddCustomRectRegular(FONT_ATLAS_DEFAULT_TEX_DATA_W * 2 + 1, FONT_ATLAS_DEFAULT_TEX_DATA_H);
        else
            atlas->PackIdMouseCursors = atlas->AddCustomRectRegular(2, 2);
    }

    // Register texture region for thick lines
    // The +2 here is to give space for the end caps, whilst height +1 is to accommodate the fact we have a zero-width row
    if (atlas->PackIdLines < 0)
    {
        if (!(atlas->Flags & ImFontAtlasFlags_NoBakedLines))
            atlas->PackIdLines = atlas->AddCustomRectRegular(IM_DRAWLIST_TEX_LINES_WIDTH_MAX + 2, IM_DRAWLIST_TEX_LINES_WIDTH_MAX + 1);
    }
}

// This is called/shared by both the stb_truetype and the FreeType builder.
void ImFontAtlasBuildFinish(ImFontAtlas* atlas)
{
    // Render into our custom data blocks
    IM_ASSERT(atlas->TexPixelsAlpha8 != NULL || atlas->TexPixelsRGBA32 != NULL);
    ImFontAtlasBuildRenderDefaultTexData(atlas);
    ImFontAtlasBuildRenderLinesTexData(atlas);

    // Register custom rectangle glyphs
    for (int i = 0; i < atlas->CustomRects.Size; i++)
    {
        const ImFontAtlasCustomRect* r = &atlas->CustomRects[i];
        if (r->Font == NULL || r->GlyphID == 0)
            continue;

        // Will ignore ImFontConfig settings: GlyphMinAdvanceX, GlyphMinAdvanceY, GlyphExtraSpacing, PixelSnapH
        IM_ASSERT(r->Font->ContainerAtlas == atlas);
        ImVec2 uv0, uv1;
        atlas->CalcCustomRectUV(r, &uv0, &uv1);
        r->Font->AddGlyph(NULL, (ImWchar)r->GlyphID, r->GlyphOffset.x, r->GlyphOffset.y, r->GlyphOffset.x + r->Width, r->GlyphOffset.y + r->Height, uv0.x, uv0.y, uv1.x, uv1.y, r->GlyphAdvanceX);
        if (r->GlyphColored)
            r->Font->Glyphs.back().Colored = 1;
    }

    // Build all fonts lookup tables
    for (ImFont* font : atlas->Fonts)
        if (font->DirtyLookupTables)
            font->BuildLookupTable();

    atlas->TexReady = true;
}

//-------------------------------------------------------------------------
// [SECTION] ImFontAtlas: glyph ranges helpers
//-------------------------------------------------------------------------

// Retrieve list of range (2 int per range, values are inclusive)
const ImWchar*   ImFontAtlas::GetGlyphRangesDefault()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0,
    };
    return &ranges[0];
}

const ImWchar*   ImFontAtlas::GetGlyphRangesGreek()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x0370, 0x03FF, // Greek and Coptic
        0,
    };
    return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesKorean()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x3131, 0x3163, // Korean alphabets
        0xAC00, 0xD7A3, // Korean characters
        0xFFFD, 0xFFFD, // Invalid
        0,
    };
    return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesChineseFull()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x2000, 0x206F, // General Punctuation
        0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
        0x31F0, 0x31FF, // Katakana Phonetic Extensions
        0xFF00, 0xFFEF, // Half-width characters
        0xFFFD, 0xFFFD, // Invalid
        0x4e00, 0x9FAF, // CJK Ideograms
        0,
    };
    return &ranges[0];
}

static void UnpackAccumulativeOffsetsIntoRanges(int base_codepoint, const short* accumulative_offsets, int accumulative_offsets_count, ImWchar* out_ranges)
{
    for (int n = 0; n < accumulative_offsets_count; n++, out_ranges += 2)
    {
        out_ranges[0] = out_ranges[1] = (ImWchar)(base_codepoint + accumulative_offsets[n]);
        base_codepoint += accumulative_offsets[n];
    }
    out_ranges[0] = 0;
}

const ImWchar*  ImFontAtlas::GetGlyphRangesChineseSimplifiedCommon()
{
    // Store 2500 regularly used characters for Simplified Chinese.
    // Sourced from https://zh.wiktionary.org/wiki/%E9%99%84%E5%BD%95:%E7%8E%B0%E4%BB%A3%E6%B1%89%E8%AF%AD%E5%B8%B8%E7%94%A8%E5%AD%97%E8%A1%A8
    // This table covers 97.97% of all characters used during the month in July, 1987.
    // You can use ImFontGlyphRangesBuilder to create your own ranges derived from this, by merging existing ranges or adding new characters.
    // (Stored as accumulative offsets from the initial unicode codepoint 0x4E00. This encoding is designed to helps us compact the source code size.)
    static const short accumulative_offsets_from_0x4E00[] =
    {
        0,1,2,4,1,1,1,1,2,1,3,2,1,2,2,1,1,1,1,1,5,2,1,2,3,3,3,2,2,4,1,1,1,2,1,5,2,3,1,2,1,2,1,1,2,1,1,2,2,1,4,1,1,1,1,5,10,1,2,19,2,1,2,1,2,1,2,1,2,
        1,5,1,6,3,2,1,2,2,1,1,1,4,8,5,1,1,4,1,1,3,1,2,1,5,1,2,1,1,1,10,1,1,5,2,4,6,1,4,2,2,2,12,2,1,1,6,1,1,1,4,1,1,4,6,5,1,4,2,2,4,10,7,1,1,4,2,4,
        2,1,4,3,6,10,12,5,7,2,14,2,9,1,1,6,7,10,4,7,13,1,5,4,8,4,1,1,2,28,5,6,1,1,5,2,5,20,2,2,9,8,11,2,9,17,1,8,6,8,27,4,6,9,20,11,27,6,68,2,2,1,1,
        1,2,1,2,2,7,6,11,3,3,1,1,3,1,2,1,1,1,1,1,3,1,1,8,3,4,1,5,7,2,1,4,4,8,4,2,1,2,1,1,4,5,6,3,6,2,12,3,1,3,9,2,4,3,4,1,5,3,3,1,3,7,1,5,1,1,1,1,2,
        3,4,5,2,3,2,6,1,1,2,1,7,1,7,3,4,5,15,2,2,1,5,3,22,19,2,1,1,1,1,2,5,1,1,1,6,1,1,12,8,2,9,18,22,4,1,1,5,1,16,1,2,7,10,15,1,1,6,2,4,1,2,4,1,6,
        1,1,3,2,4,1,6,4,5,1,2,1,1,2,1,10,3,1,3,2,1,9,3,2,5,7,2,19,4,3,6,1,1,1,1,1,4,3,2,1,1,1,2,5,3,1,1,1,2,2,1,1,2,1,1,2,1,3,1,1,1,3,7,1,4,1,1,2,1,
        1,2,1,2,4,4,3,8,1,1,1,2,1,3,5,1,3,1,3,4,6,2,2,14,4,6,6,11,9,1,15,3,1,28,5,2,5,5,3,1,3,4,5,4,6,14,3,2,3,5,21,2,7,20,10,1,2,19,2,4,28,28,2,3,
        2,1,14,4,1,26,28,42,12,40,3,52,79,5,14,17,3,2,2,11,3,4,6,3,1,8,2,23,4,5,8,10,4,2,7,3,5,1,1,6,3,1,2,2,2,5,28,1,1,7,7,20,5,3,29,3,17,26,1,8,4,
        27,3,6,11,23,5,3,4,6,13,24,16,6,5,10,25,35,7,3,2,3,3,14,3,6,2,6,1,4,2,3,8,2,1,1,3,3,3,4,1,1,13,2,2,4,5,2,1,14,14,1,2,2,1,4,5,2,3,1,14,3,12,
        3,17,2,16,5,1,2,1,8,9,3,19,4,2,2,4,17,25,21,20,28,75,1,10,29,103,4,1,2,1,1,4,2,4,1,2,3,24,2,2,2,1,1,2,1,3,8,1,1,1,2,1,1,3,1,1,1,6,1,5,3,1,1,
        1,3,4,1,1,5,2,1,5,6,13,9,16,1,1,1,1,3,2,3,2,4,5,2,5,2,2,3,7,13,7,2,2,1,1,1,1,2,3,3,2,1,6,4,9,2,1,14,2,14,2,1,18,3,4,14,4,11,41,15,23,15,23,
        176,1,3,4,1,1,1,1,5,3,1,2,3,7,3,1,1,2,1,2,4,4,6,2,4,1,9,7,1,10,5,8,16,29,1,1,2,2,3,1,3,5,2,4,5,4,1,1,2,2,3,3,7,1,6,10,1,17,1,44,4,6,2,1,1,6,
        5,4,2,10,1,6,9,2,8,1,24,1,2,13,7,8,8,2,1,4,1,3,1,3,3,5,2,5,10,9,4,9,12,2,1,6,1,10,1,1,7,7,4,10,8,3,1,13,4,3,1,6,1,3,5,2,1,2,17,16,5,2,16,6,
        1,4,2,1,3,3,6,8,5,11,11,1,3,3,2,4,6,10,9,5,7,4,7,4,7,1,1,4,2,1,3,6,8,7,1,6,11,5,5,3,24,9,4,2,7,13,5,1,8,82,16,61,1,1,1,4,2,2,16,10,3,8,1,1,
        6,4,2,1,3,1,1,1,4,3,8,4,2,2,1,1,1,1,1,6,3,5,1,1,4,6,9,2,1,1,1,2,1,7,2,1,6,1,5,4,4,3,1,8,1,3,3,1,3,2,2,2,2,3,1,6,1,2,1,2,1,3,7,1,8,2,1,2,1,5,
        2,5,3,5,10,1,2,1,1,3,2,5,11,3,9,3,5,1,1,5,9,1,2,1,5,7,9,9,8,1,3,3,3,6,8,2,3,2,1,1,32,6,1,2,15,9,3,7,13,1,3,10,13,2,14,1,13,10,2,1,3,10,4,15,
        2,15,15,10,1,3,9,6,9,32,25,26,47,7,3,2,3,1,6,3,4,3,2,8,5,4,1,9,4,2,2,19,10,6,2,3,8,1,2,2,4,2,1,9,4,4,4,6,4,8,9,2,3,1,1,1,1,3,5,5,1,3,8,4,6,
        2,1,4,12,1,5,3,7,13,2,5,8,1,6,1,2,5,14,6,1,5,2,4,8,15,5,1,23,6,62,2,10,1,1,8,1,2,2,10,4,2,2,9,2,1,1,3,2,3,1,5,3,3,2,1,3,8,1,1,1,11,3,1,1,4,
        3,7,1,14,1,2,3,12,5,2,5,1,6,7,5,7,14,11,1,3,1,8,9,12,2,1,11,8,4,4,2,6,10,9,13,1,1,3,1,5,1,3,2,4,4,1,18,2,3,14,11,4,29,4,2,7,1,3,13,9,2,2,5,
        3,5,20,7,16,8,5,72,34,6,4,22,12,12,28,45,36,9,7,39,9,191,1,1,1,4,11,8,4,9,2,3,22,1,1,1,1,4,17,1,7,7,1,11,31,10,2,4,8,2,3,2,1,4,2,16,4,32,2,
        3,19,13,4,9,1,5,2,14,8,1,1,3,6,19,6,5,1,16,6,2,10,8,5,1,2,3,1,5,5,1,11,6,6,1,3,3,2,6,3,8,1,1,4,10,7,5,7,7,5,8,9,2,1,3,4,1,1,3,1,3,3,2,6,16,
        1,4,6,3,1,10,6,1,3,15,2,9,2,10,25,13,9,16,6,2,2,10,11,4,3,9,1,2,6,6,5,4,30,40,1,10,7,12,14,33,6,3,6,7,3,1,3,1,11,14,4,9,5,12,11,49,18,51,31,
        140,31,2,2,1,5,1,8,1,10,1,4,4,3,24,1,10,1,3,6,6,16,3,4,5,2,1,4,2,57,10,6,22,2,22,3,7,22,6,10,11,36,18,16,33,36,2,5,5,1,1,1,4,10,1,4,13,2,7,
        5,2,9,3,4,1,7,43,3,7,3,9,14,7,9,1,11,1,1,3,7,4,18,13,1,14,1,3,6,10,73,2,2,30,6,1,11,18,19,13,22,3,46,42,37,89,7,3,16,34,2,2,3,9,1,7,1,1,1,2,
        2,4,10,7,3,10,3,9,5,28,9,2,6,13,7,3,1,3,10,2,7,2,11,3,6,21,54,85,2,1,4,2,2,1,39,3,21,2,2,5,1,1,1,4,1,1,3,4,15,1,3,2,4,4,2,3,8,2,20,1,8,7,13,
        4,1,26,6,2,9,34,4,21,52,10,4,4,1,5,12,2,11,1,7,2,30,12,44,2,30,1,1,3,6,16,9,17,39,82,2,2,24,7,1,7,3,16,9,14,44,2,1,2,1,2,3,5,2,4,1,6,7,5,3,
        2,6,1,11,5,11,2,1,18,19,8,1,3,24,29,2,1,3,5,2,2,1,13,6,5,1,46,11,3,5,1,1,5,8,2,10,6,12,6,3,7,11,2,4,16,13,2,5,1,1,2,2,5,2,28,5,2,23,10,8,4,
        4,22,39,95,38,8,14,9,5,1,13,5,4,3,13,12,11,1,9,1,27,37,2,5,4,4,63,211,95,2,2,2,1,3,5,2,1,1,2,2,1,1,1,3,2,4,1,2,1,1,5,2,2,1,1,2,3,1,3,1,1,1,
        3,1,4,2,1,3,6,1,1,3,7,15,5,3,2,5,3,9,11,4,2,22,1,6,3,8,7,1,4,28,4,16,3,3,25,4,4,27,27,1,4,1,2,2,7,1,3,5,2,28,8,2,14,1,8,6,16,25,3,3,3,14,3,
        3,1,1,2,1,4,6,3,8,4,1,1,1,2,3,6,10,6,2,3,18,3,2,5,5,4,3,1,5,2,5,4,23,7,6,12,6,4,17,11,9,5,1,1,10,5,12,1,1,11,26,33,7,3,6,1,17,7,1,5,12,1,11,
        2,4,1,8,14,17,23,1,2,1,7,8,16,11,9,6,5,2,6,4,16,2,8,14,1,11,8,9,1,1,1,9,25,4,11,19,7,2,15,2,12,8,52,7,5,19,2,16,4,36,8,1,16,8,24,26,4,6,2,9,
        5,4,36,3,28,12,25,15,37,27,17,12,59,38,5,32,127,1,2,9,17,14,4,1,2,1,1,8,11,50,4,14,2,19,16,4,17,5,4,5,26,12,45,2,23,45,104,30,12,8,3,10,2,2,
        3,3,1,4,20,7,2,9,6,15,2,20,1,3,16,4,11,15,6,134,2,5,59,1,2,2,2,1,9,17,3,26,137,10,211,59,1,2,4,1,4,1,1,1,2,6,2,3,1,1,2,3,2,3,1,3,4,4,2,3,3,
        1,4,3,1,7,2,2,3,1,2,1,3,3,3,2,2,3,2,1,3,14,6,1,3,2,9,6,15,27,9,34,145,1,1,2,1,1,1,1,2,1,1,1,1,2,2,2,3,1,2,1,1,1,2,3,5,8,3,5,2,4,1,3,2,2,2,12,
        4,1,1,1,10,4,5,1,20,4,16,1,15,9,5,12,2,9,2,5,4,2,26,19,7,1,26,4,30,12,15,42,1,6,8,172,1,1,4,2,1,1,11,2,2,4,2,1,2,1,10,8,1,2,1,4,5,1,2,5,1,8,
        4,1,3,4,2,1,6,2,1,3,4,1,2,1,1,1,1,12,5,7,2,4,3,1,1,1,3,3,6,1,2,2,3,3,3,2,1,2,12,14,11,6,6,4,12,2,8,1,7,10,1,35,7,4,13,15,4,3,23,21,28,52,5,
        26,5,6,1,7,10,2,7,53,3,2,1,1,1,2,163,532,1,10,11,1,3,3,4,8,2,8,6,2,2,23,22,4,2,2,4,2,1,3,1,3,3,5,9,8,2,1,2,8,1,10,2,12,21,20,15,105,2,3,1,1,
        3,2,3,1,1,2,5,1,4,15,11,19,1,1,1,1,5,4,5,1,1,2,5,3,5,12,1,2,5,1,11,1,1,15,9,1,4,5,3,26,8,2,1,3,1,1,15,19,2,12,1,2,5,2,7,2,19,2,20,6,26,7,5,
        2,2,7,34,21,13,70,2,128,1,1,2,1,1,2,1,1,3,2,2,2,15,1,4,1,3,4,42,10,6,1,49,85,8,1,2,1,1,4,4,2,3,6,1,5,7,4,3,211,4,1,2,1,2,5,1,2,4,2,2,6,5,6,
        10,3,4,48,100,6,2,16,296,5,27,387,2,2,3,7,16,8,5,38,15,39,21,9,10,3,7,59,13,27,21,47,5,21,6
    };
    static ImWchar base_ranges[] = // not zero-terminated
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x2000, 0x206F, // General Punctuation
        0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
        0x31F0, 0x31FF, // Katakana Phonetic Extensions
        0xFF00, 0xFFEF, // Half-width characters
        0xFFFD, 0xFFFD  // Invalid
    };
    static ImWchar full_ranges[IM_ARRAYSIZE(base_ranges) + IM_ARRAYSIZE(accumulative_offsets_from_0x4E00) * 2 + 1] = { 0 };
    if (!full_ranges[0])
    {
        memcpy(full_ranges, base_ranges, sizeof(base_ranges));
        UnpackAccumulativeOffsetsIntoRanges(0x4E00, accumulative_offsets_from_0x4E00, IM_ARRAYSIZE(accumulative_offsets_from_0x4E00), full_ranges + IM_ARRAYSIZE(base_ranges));
    }
    return &full_ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesJapanese()
{
    // 2999 ideograms code points for Japanese
    // - 2136 Joyo (meaning "for regular use" or "for common use") Kanji code points
    // - 863 Jinmeiyo (meaning "for personal name") Kanji code points
    // - Sourced from official information provided by the government agencies of Japan:
    //   - List of Joyo Kanji by the Agency for Cultural Affairs
    //     - https://www.bunka.go.jp/kokugo_nihongo/sisaku/joho/joho/kijun/naikaku/kanji/
    //   - List of Jinmeiyo Kanji by the Ministry of Justice
    //     - http://www.moj.go.jp/MINJI/minji86.html
    //   - Available under the terms of the Creative Commons Attribution 4.0 International (CC BY 4.0).
    //     - https://creativecommons.org/licenses/by/4.0/legalcode
    // - You can generate this code by the script at:
    //   - https://github.com/vaiorabbit/everyday_use_kanji
    // - References:
    //   - List of Joyo Kanji
    //     - (Wikipedia) https://en.wikipedia.org/wiki/List_of_j%C5%8Dy%C5%8D_kanji
    //   - List of Jinmeiyo Kanji
    //     - (Wikipedia) https://en.wikipedia.org/wiki/Jinmeiy%C5%8D_kanji
    // - Missing 1 Joyo Kanji: U+20B9F (Kun'yomi: Shikaru, On'yomi: Shitsu,shichi), see https://github.com/ocornut/imgui/pull/3627 for details.
    // You can use ImFontGlyphRangesBuilder to create your own ranges derived from this, by merging existing ranges or adding new characters.
    // (Stored as accumulative offsets from the initial unicode codepoint 0x4E00. This encoding is designed to helps us compact the source code size.)
    static const short accumulative_offsets_from_0x4E00[] =
    {
        0,1,2,4,1,1,1,1,2,1,3,3,2,2,1,5,3,5,7,5,6,1,2,1,7,2,6,3,1,8,1,1,4,1,1,18,2,11,2,6,2,1,2,1,5,1,2,1,3,1,2,1,2,3,3,1,1,2,3,1,1,1,12,7,9,1,4,5,1,
        1,2,1,10,1,1,9,2,2,4,5,6,9,3,1,1,1,1,9,3,18,5,2,2,2,2,1,6,3,7,1,1,1,1,2,2,4,2,1,23,2,10,4,3,5,2,4,10,2,4,13,1,6,1,9,3,1,1,6,6,7,6,3,1,2,11,3,
        2,2,3,2,15,2,2,5,4,3,6,4,1,2,5,2,12,16,6,13,9,13,2,1,1,7,16,4,7,1,19,1,5,1,2,2,7,7,8,2,6,5,4,9,18,7,4,5,9,13,11,8,15,2,1,1,1,2,1,2,2,1,2,2,8,
        2,9,3,3,1,1,4,4,1,1,1,4,9,1,4,3,5,5,2,7,5,3,4,8,2,1,13,2,3,3,1,14,1,1,4,5,1,3,6,1,5,2,1,1,3,3,3,3,1,1,2,7,6,6,7,1,4,7,6,1,1,1,1,1,12,3,3,9,5,
        2,6,1,5,6,1,2,3,18,2,4,14,4,1,3,6,1,1,6,3,5,5,3,2,2,2,2,12,3,1,4,2,3,2,3,11,1,7,4,1,2,1,3,17,1,9,1,24,1,1,4,2,2,4,1,2,7,1,1,1,3,1,2,2,4,15,1,
        1,2,1,1,2,1,5,2,5,20,2,5,9,1,10,8,7,6,1,1,1,1,1,1,6,2,1,2,8,1,1,1,1,5,1,1,3,1,1,1,1,3,1,1,12,4,1,3,1,1,1,1,1,10,3,1,7,5,13,1,2,3,4,6,1,1,30,
        2,9,9,1,15,38,11,3,1,8,24,7,1,9,8,10,2,1,9,31,2,13,6,2,9,4,49,5,2,15,2,1,10,2,1,1,1,2,2,6,15,30,35,3,14,18,8,1,16,10,28,12,19,45,38,1,3,2,3,
        13,2,1,7,3,6,5,3,4,3,1,5,7,8,1,5,3,18,5,3,6,1,21,4,24,9,24,40,3,14,3,21,3,2,1,2,4,2,3,1,15,15,6,5,1,1,3,1,5,6,1,9,7,3,3,2,1,4,3,8,21,5,16,4,
        5,2,10,11,11,3,6,3,2,9,3,6,13,1,2,1,1,1,1,11,12,6,6,1,4,2,6,5,2,1,1,3,3,6,13,3,1,1,5,1,2,3,3,14,2,1,2,2,2,5,1,9,5,1,1,6,12,3,12,3,4,13,2,14,
        2,8,1,17,5,1,16,4,2,2,21,8,9,6,23,20,12,25,19,9,38,8,3,21,40,25,33,13,4,3,1,4,1,2,4,1,2,5,26,2,1,1,2,1,3,6,2,1,1,1,1,1,1,2,3,1,1,1,9,2,3,1,1,
        1,3,6,3,2,1,1,6,6,1,8,2,2,2,1,4,1,2,3,2,7,3,2,4,1,2,1,2,2,1,1,1,1,1,3,1,2,5,4,10,9,4,9,1,1,1,1,1,1,5,3,2,1,6,4,9,6,1,10,2,31,17,8,3,7,5,40,1,
        7,7,1,6,5,2,10,7,8,4,15,39,25,6,28,47,18,10,7,1,3,1,1,2,1,1,1,3,3,3,1,1,1,3,4,2,1,4,1,3,6,10,7,8,6,2,2,1,3,3,2,5,8,7,9,12,2,15,1,1,4,1,2,1,1,
        1,3,2,1,3,3,5,6,2,3,2,10,1,4,2,8,1,1,1,11,6,1,21,4,16,3,1,3,1,4,2,3,6,5,1,3,1,1,3,3,4,6,1,1,10,4,2,7,10,4,7,4,2,9,4,3,1,1,1,4,1,8,3,4,1,3,1,
        6,1,4,2,1,4,7,2,1,8,1,4,5,1,1,2,2,4,6,2,7,1,10,1,1,3,4,11,10,8,21,4,6,1,3,5,2,1,2,28,5,5,2,3,13,1,2,3,1,4,2,1,5,20,3,8,11,1,3,3,3,1,8,10,9,2,
        10,9,2,3,1,1,2,4,1,8,3,6,1,7,8,6,11,1,4,29,8,4,3,1,2,7,13,1,4,1,6,2,6,12,12,2,20,3,2,3,6,4,8,9,2,7,34,5,1,18,6,1,1,4,4,5,7,9,1,2,2,4,3,4,1,7,
        2,2,2,6,2,3,25,5,3,6,1,4,6,7,4,2,1,4,2,13,6,4,4,3,1,5,3,4,4,3,2,1,1,4,1,2,1,1,3,1,11,1,6,3,1,7,3,6,2,8,8,6,9,3,4,11,3,2,10,12,2,5,11,1,6,4,5,
        3,1,8,5,4,6,6,3,5,1,1,3,2,1,2,2,6,17,12,1,10,1,6,12,1,6,6,19,9,6,16,1,13,4,4,15,7,17,6,11,9,15,12,6,7,2,1,2,2,15,9,3,21,4,6,49,18,7,3,2,3,1,
        6,8,2,2,6,2,9,1,3,6,4,4,1,2,16,2,5,2,1,6,2,3,5,3,1,2,5,1,2,1,9,3,1,8,6,4,8,11,3,1,1,1,1,3,1,13,8,4,1,3,2,2,1,4,1,11,1,5,2,1,5,2,5,8,6,1,1,7,
        4,3,8,3,2,7,2,1,5,1,5,2,4,7,6,2,8,5,1,11,4,5,3,6,18,1,2,13,3,3,1,21,1,1,4,1,4,1,1,1,8,1,2,2,7,1,2,4,2,2,9,2,1,1,1,4,3,6,3,12,5,1,1,1,5,6,3,2,
        4,8,2,2,4,2,7,1,8,9,5,2,3,2,1,3,2,13,7,14,6,5,1,1,2,1,4,2,23,2,1,1,6,3,1,4,1,15,3,1,7,3,9,14,1,3,1,4,1,1,5,8,1,3,8,3,8,15,11,4,14,4,4,2,5,5,
        1,7,1,6,14,7,7,8,5,15,4,8,6,5,6,2,1,13,1,20,15,11,9,2,5,6,2,11,2,6,2,5,1,5,8,4,13,19,25,4,1,1,11,1,34,2,5,9,14,6,2,2,6,1,1,14,1,3,14,13,1,6,
        12,21,14,14,6,32,17,8,32,9,28,1,2,4,11,8,3,1,14,2,5,15,1,1,1,1,3,6,4,1,3,4,11,3,1,1,11,30,1,5,1,4,1,5,8,1,1,3,2,4,3,17,35,2,6,12,17,3,1,6,2,
        1,1,12,2,7,3,3,2,1,16,2,8,3,6,5,4,7,3,3,8,1,9,8,5,1,2,1,3,2,8,1,2,9,12,1,1,2,3,8,3,24,12,4,3,7,5,8,3,3,3,3,3,3,1,23,10,3,1,2,2,6,3,1,16,1,16,
        22,3,10,4,11,6,9,7,7,3,6,2,2,2,4,10,2,1,1,2,8,7,1,6,4,1,3,3,3,5,10,12,12,2,3,12,8,15,1,1,16,6,6,1,5,9,11,4,11,4,2,6,12,1,17,5,13,1,4,9,5,1,11,
        2,1,8,1,5,7,28,8,3,5,10,2,17,3,38,22,1,2,18,12,10,4,38,18,1,4,44,19,4,1,8,4,1,12,1,4,31,12,1,14,7,75,7,5,10,6,6,13,3,2,11,11,3,2,5,28,15,6,18,
        18,5,6,4,3,16,1,7,18,7,36,3,5,3,1,7,1,9,1,10,7,2,4,2,6,2,9,7,4,3,32,12,3,7,10,2,23,16,3,1,12,3,31,4,11,1,3,8,9,5,1,30,15,6,12,3,2,2,11,19,9,
        14,2,6,2,3,19,13,17,5,3,3,25,3,14,1,1,1,36,1,3,2,19,3,13,36,9,13,31,6,4,16,34,2,5,4,2,3,3,5,1,1,1,4,3,1,17,3,2,3,5,3,1,3,2,3,5,6,3,12,11,1,3,
        1,2,26,7,12,7,2,14,3,3,7,7,11,25,25,28,16,4,36,1,2,1,6,2,1,9,3,27,17,4,3,4,13,4,1,3,2,2,1,10,4,2,4,6,3,8,2,1,18,1,1,24,2,2,4,33,2,3,63,7,1,6,
        40,7,3,4,4,2,4,15,18,1,16,1,1,11,2,41,14,1,3,18,13,3,2,4,16,2,17,7,15,24,7,18,13,44,2,2,3,6,1,1,7,5,1,7,1,4,3,3,5,10,8,2,3,1,8,1,1,27,4,2,1,
        12,1,2,1,10,6,1,6,7,5,2,3,7,11,5,11,3,6,6,2,3,15,4,9,1,1,2,1,2,11,2,8,12,8,5,4,2,3,1,5,2,2,1,14,1,12,11,4,1,11,17,17,4,3,2,5,5,7,3,1,5,9,9,8,
        2,5,6,6,13,13,2,1,2,6,1,2,2,49,4,9,1,2,10,16,7,8,4,3,2,23,4,58,3,29,1,14,19,19,11,11,2,7,5,1,3,4,6,2,18,5,12,12,17,17,3,3,2,4,1,6,2,3,4,3,1,
        1,1,1,5,1,1,9,1,3,1,3,6,1,8,1,1,2,6,4,14,3,1,4,11,4,1,3,32,1,2,4,13,4,1,2,4,2,1,3,1,11,1,4,2,1,4,4,6,3,5,1,6,5,7,6,3,23,3,5,3,5,3,3,13,3,9,10,
        1,12,10,2,3,18,13,7,160,52,4,2,2,3,2,14,5,4,12,4,6,4,1,20,4,11,6,2,12,27,1,4,1,2,2,7,4,5,2,28,3,7,25,8,3,19,3,6,10,2,2,1,10,2,5,4,1,3,4,1,5,
        3,2,6,9,3,6,2,16,3,3,16,4,5,5,3,2,1,2,16,15,8,2,6,21,2,4,1,22,5,8,1,1,21,11,2,1,11,11,19,13,12,4,2,3,2,3,6,1,8,11,1,4,2,9,5,2,1,11,2,9,1,1,2,
        14,31,9,3,4,21,14,4,8,1,7,2,2,2,5,1,4,20,3,3,4,10,1,11,9,8,2,1,4,5,14,12,14,2,17,9,6,31,4,14,1,20,13,26,5,2,7,3,6,13,2,4,2,19,6,2,2,18,9,3,5,
        12,12,14,4,6,2,3,6,9,5,22,4,5,25,6,4,8,5,2,6,27,2,35,2,16,3,7,8,8,6,6,5,9,17,2,20,6,19,2,13,3,1,1,1,4,17,12,2,14,7,1,4,18,12,38,33,2,10,1,1,
        2,13,14,17,11,50,6,33,20,26,74,16,23,45,50,13,38,33,6,6,7,4,4,2,1,3,2,5,8,7,8,9,3,11,21,9,13,1,3,10,6,7,1,2,2,18,5,5,1,9,9,2,68,9,19,13,2,5,
        1,4,4,7,4,13,3,9,10,21,17,3,26,2,1,5,2,4,5,4,1,7,4,7,3,4,2,1,6,1,1,20,4,1,9,2,2,1,3,3,2,3,2,1,1,1,20,2,3,1,6,2,3,6,2,4,8,1,3,2,10,3,5,3,4,4,
        3,4,16,1,6,1,10,2,4,2,1,1,2,10,11,2,2,3,1,24,31,4,10,10,2,5,12,16,164,15,4,16,7,9,15,19,17,1,2,1,1,5,1,1,1,1,1,3,1,4,3,1,3,1,3,1,2,1,1,3,3,7,
        2,8,1,2,2,2,1,3,4,3,7,8,12,92,2,10,3,1,3,14,5,25,16,42,4,7,7,4,2,21,5,27,26,27,21,25,30,31,2,1,5,13,3,22,5,6,6,11,9,12,1,5,9,7,5,5,22,60,3,5,
        13,1,1,8,1,1,3,3,2,1,9,3,3,18,4,1,2,3,7,6,3,1,2,3,9,1,3,1,3,2,1,3,1,1,1,2,1,11,3,1,6,9,1,3,2,3,1,2,1,5,1,1,4,3,4,1,2,2,4,4,1,7,2,1,2,2,3,5,13,
        18,3,4,14,9,9,4,16,3,7,5,8,2,6,48,28,3,1,1,4,2,14,8,2,9,2,1,15,2,4,3,2,10,16,12,8,7,1,1,3,1,1,1,2,7,4,1,6,4,38,39,16,23,7,15,15,3,2,12,7,21,
        37,27,6,5,4,8,2,10,8,8,6,5,1,2,1,3,24,1,16,17,9,23,10,17,6,1,51,55,44,13,294,9,3,6,2,4,2,2,15,1,1,1,13,21,17,68,14,8,9,4,1,4,9,3,11,7,1,1,1,
        5,6,3,2,1,1,1,2,3,8,1,2,2,4,1,5,5,2,1,4,3,7,13,4,1,4,1,3,1,1,1,5,5,10,1,6,1,5,2,1,5,2,4,1,4,5,7,3,18,2,9,11,32,4,3,3,2,4,7,11,16,9,11,8,13,38,
        32,8,4,2,1,1,2,1,2,4,4,1,1,1,4,1,21,3,11,1,16,1,1,6,1,3,2,4,9,8,57,7,44,1,3,3,13,3,10,1,1,7,5,2,7,21,47,63,3,15,4,7,1,16,1,1,2,8,2,3,42,15,4,
        1,29,7,22,10,3,78,16,12,20,18,4,67,11,5,1,3,15,6,21,31,32,27,18,13,71,35,5,142,4,10,1,2,50,19,33,16,35,37,16,19,27,7,1,133,19,1,4,8,7,20,1,4,
        4,1,10,3,1,6,1,2,51,5,40,15,24,43,22928,11,1,13,154,70,3,1,1,7,4,10,1,2,1,1,2,1,2,1,2,2,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,
        3,2,1,1,1,1,2,1,1,
    };
    static ImWchar base_ranges[] = // not zero-terminated
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
        0x31F0, 0x31FF, // Katakana Phonetic Extensions
        0xFF00, 0xFFEF, // Half-width characters
        0xFFFD, 0xFFFD  // Invalid
    };
    static ImWchar full_ranges[IM_ARRAYSIZE(base_ranges) + IM_ARRAYSIZE(accumulative_offsets_from_0x4E00)*2 + 1] = { 0 };
    if (!full_ranges[0])
    {
        memcpy(full_ranges, base_ranges, sizeof(base_ranges));
        UnpackAccumulativeOffsetsIntoRanges(0x4E00, accumulative_offsets_from_0x4E00, IM_ARRAYSIZE(accumulative_offsets_from_0x4E00), full_ranges + IM_ARRAYSIZE(base_ranges));
    }
    return &full_ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesCyrillic()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
        0x2DE0, 0x2DFF, // Cyrillic Extended-A
        0xA640, 0xA69F, // Cyrillic Extended-B
        0,
    };
    return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesThai()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin
        0x2010, 0x205E, // Punctuations
        0x0E00, 0x0E7F, // Thai
        0,
    };
    return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesVietnamese()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin
        0x0102, 0x0103,
        0x0110, 0x0111,
        0x0128, 0x0129,
        0x0168, 0x0169,
        0x01A0, 0x01A1,
        0x01AF, 0x01B0,
        0x1EA0, 0x1EF9,
        0,
    };
    return &ranges[0];
}

//-----------------------------------------------------------------------------
// [SECTION] ImFontGlyphRangesBuilder
//-----------------------------------------------------------------------------

void ImFontGlyphRangesBuilder::AddText(const char* text, const char* text_end)
{
    while (text_end ? (text < text_end) : *text)
    {
        unsigned int c = 0;
        int c_len = ImTextCharFromUtf8(&c, text, text_end);
        text += c_len;
        if (c_len == 0)
            break;
        AddChar((ImWchar)c);
    }
}

void ImFontGlyphRangesBuilder::AddRanges(const ImWchar* ranges)
{
    for (; ranges[0]; ranges += 2)
        for (unsigned int c = ranges[0]; c <= ranges[1] && c <= IM_UNICODE_CODEPOINT_MAX; c++) //-V560
            AddChar((ImWchar)c);
}

void ImFontGlyphRangesBuilder::BuildRanges(ImVector<ImWchar>* out_ranges)
{
    const int max_codepoint = IM_UNICODE_CODEPOINT_MAX;
    for (int n = 0; n <= max_codepoint; n++)
        if (GetBit(n))
        {
            out_ranges->push_back((ImWchar)n);
            while (n < max_codepoint && GetBit(n + 1))
                n++;
            out_ranges->push_back((ImWchar)n);
        }
    out_ranges->push_back(0);
}

//-----------------------------------------------------------------------------
// [SECTION] ImFont
//-----------------------------------------------------------------------------

ImFont::ImFont()
{
    FontSize = 0.0f;
    FallbackAdvanceX = 0.0f;
    FallbackChar = (ImWchar)-1;
    EllipsisChar = (ImWchar)-1;
    EllipsisWidth = EllipsisCharStep = 0.0f;
    EllipsisCharCount = 0;
    FallbackGlyph = NULL;
    ContainerAtlas = NULL;
    ConfigData = NULL;
    ConfigDataCount = 0;
    DirtyLookupTables = false;
    Scale = 1.0f;
    Ascent = Descent = 0.0f;
    MetricsTotalSurface = 0;
    memset(Used4kPagesMap, 0, sizeof(Used4kPagesMap));
}

ImFont::~ImFont()
{
    ClearOutputData();
}

void    ImFont::ClearOutputData()
{
    FontSize = 0.0f;
    FallbackAdvanceX = 0.0f;
    Glyphs.clear();
    IndexAdvanceX.clear();
    IndexLookup.clear();
    FallbackGlyph = NULL;
    ContainerAtlas = NULL;
    DirtyLookupTables = true;
    Ascent = Descent = 0.0f;
    MetricsTotalSurface = 0;
    memset(Used4kPagesMap, 0, sizeof(Used4kPagesMap));
}

static ImWchar FindFirstExistingGlyph(ImFont* font, const ImWchar* candidate_chars, int candidate_chars_count)
{
    for (int n = 0; n < candidate_chars_count; n++)
        if (font->FindGlyphNoFallback(candidate_chars[n]) != NULL)
            return candidate_chars[n];
    return (ImWchar)-1;
}

void ImFont::BuildLookupTable()
{
    int max_codepoint = 0;
    for (int i = 0; i != Glyphs.Size; i++)
        max_codepoint = ImMax(max_codepoint, (int)Glyphs[i].Codepoint);

    // Build lookup table
    IM_ASSERT(Glyphs.Size > 0 && "Font has not loaded glyph!");
    IM_ASSERT(Glyphs.Size < 0xFFFF); // -1 is reserved
    IndexAdvanceX.clear();
    IndexLookup.clear();
    DirtyLookupTables = false;
    memset(Used4kPagesMap, 0, sizeof(Used4kPagesMap));
    GrowIndex(max_codepoint + 1);
    for (int i = 0; i < Glyphs.Size; i++)
    {
        int codepoint = (int)Glyphs[i].Codepoint;
        IndexAdvanceX[codepoint] = Glyphs[i].AdvanceX;
        IndexLookup[codepoint] = (ImWchar)i;

        // Mark 4K page as used
        const int page_n = codepoint / 4096;
        Used4kPagesMap[page_n >> 3] |= 1 << (page_n & 7);
    }

    // Create a glyph to handle TAB
    // FIXME: Needs proper TAB handling but it needs to be contextualized (or we could arbitrary say that each string starts at "column 0" ?)
    if (FindGlyph((ImWchar)' '))
    {
        if (Glyphs.back().Codepoint != '\t')   // So we can call this function multiple times (FIXME: Flaky)
            Glyphs.resize(Glyphs.Size + 1);
        ImFontGlyph& tab_glyph = Glyphs.back();
        tab_glyph = *FindGlyph((ImWchar)' ');
        tab_glyph.Codepoint = '\t';
        tab_glyph.AdvanceX *= IM_TABSIZE;
        IndexAdvanceX[(int)tab_glyph.Codepoint] = (float)tab_glyph.AdvanceX;
        IndexLookup[(int)tab_glyph.Codepoint] = (ImWchar)(Glyphs.Size - 1);
    }

    // Mark special glyphs as not visible (note that AddGlyph already mark as non-visible glyphs with zero-size polygons)
    SetGlyphVisible((ImWchar)' ', false);
    SetGlyphVisible((ImWchar)'\t', false);

    // Setup Fallback character
    const ImWchar fallback_chars[] = { (ImWchar)IM_UNICODE_CODEPOINT_INVALID, (ImWchar)'?', (ImWchar)' ' };
    FallbackGlyph = FindGlyphNoFallback(FallbackChar);
    if (FallbackGlyph == NULL)
    {
        FallbackChar = FindFirstExistingGlyph(this, fallback_chars, IM_ARRAYSIZE(fallback_chars));
        FallbackGlyph = FindGlyphNoFallback(FallbackChar);
        if (FallbackGlyph == NULL)
        {
            FallbackGlyph = &Glyphs.back();
            FallbackChar = (ImWchar)FallbackGlyph->Codepoint;
        }
    }
    FallbackAdvanceX = FallbackGlyph->AdvanceX;
    for (int i = 0; i < max_codepoint + 1; i++)
        if (IndexAdvanceX[i] < 0.0f)
            IndexAdvanceX[i] = FallbackAdvanceX;

    // Setup Ellipsis character. It is required for rendering elided text. We prefer using U+2026 (horizontal ellipsis).
    // However some old fonts may contain ellipsis at U+0085. Here we auto-detect most suitable ellipsis character.
    // FIXME: Note that 0x2026 is rarely included in our font ranges. Because of this we are more likely to use three individual dots.
    const ImWchar ellipsis_chars[] = { (ImWchar)0x2026, (ImWchar)0x0085 };
    const ImWchar dots_chars[] = { (ImWchar)'.', (ImWchar)0xFF0E };
    if (EllipsisChar == (ImWchar)-1)
        EllipsisChar = FindFirstExistingGlyph(this, ellipsis_chars, IM_ARRAYSIZE(ellipsis_chars));
    const ImWchar dot_char = FindFirstExistingGlyph(this, dots_chars, IM_ARRAYSIZE(dots_chars));
    if (EllipsisChar != (ImWchar)-1)
    {
        EllipsisCharCount = 1;
        EllipsisWidth = EllipsisCharStep = FindGlyph(EllipsisChar)->X1;
    }
    else if (dot_char != (ImWchar)-1)
    {
        const ImFontGlyph* glyph = FindGlyph(dot_char);
        EllipsisChar = dot_char;
        EllipsisCharCount = 3;
        EllipsisCharStep = (glyph->X1 - glyph->X0) + 1.0f;
        EllipsisWidth = EllipsisCharStep * 3.0f - 1.0f;
    }
}

// API is designed this way to avoid exposing the 4K page size
// e.g. use with IsGlyphRangeUnused(0, 255)
bool ImFont::IsGlyphRangeUnused(unsigned int c_begin, unsigned int c_last)
{
    unsigned int page_begin = (c_begin / 4096);
    unsigned int page_last = (c_last / 4096);
    for (unsigned int page_n = page_begin; page_n <= page_last; page_n++)
        if ((page_n >> 3) < sizeof(Used4kPagesMap))
            if (Used4kPagesMap[page_n >> 3] & (1 << (page_n & 7)))
                return false;
    return true;
}

void ImFont::SetGlyphVisible(ImWchar c, bool visible)
{
    if (ImFontGlyph* glyph = (ImFontGlyph*)(void*)FindGlyph((ImWchar)c))
        glyph->Visible = visible ? 1 : 0;
}

void ImFont::GrowIndex(int new_size)
{
    IM_ASSERT(IndexAdvanceX.Size == IndexLookup.Size);
    if (new_size <= IndexLookup.Size)
        return;
    IndexAdvanceX.resize(new_size, -1.0f);
    IndexLookup.resize(new_size, (ImWchar)-1);
}

// x0/y0/x1/y1 are offset from the character upper-left layout position, in pixels. Therefore x0/y0 are often fairly close to zero.
// Not to be mistaken with texture coordinates, which are held by u0/v0/u1/v1 in normalized format (0.0..1.0 on each texture axis).
// 'cfg' is not necessarily == 'this->ConfigData' because multiple source fonts+configs can be used to build one target font.
void ImFont::AddGlyph(const ImFontConfig* cfg, ImWchar codepoint, float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, float advance_x)
{
    if (cfg != NULL)
    {
        // Clamp & recenter if needed
        const float advance_x_original = advance_x;
        advance_x = ImClamp(advance_x, cfg->GlyphMinAdvanceX, cfg->GlyphMaxAdvanceX);
        if (advance_x != advance_x_original)
        {
            float char_off_x = cfg->PixelSnapH ? ImTrunc((advance_x - advance_x_original) * 0.5f) : (advance_x - advance_x_original) * 0.5f;
            x0 += char_off_x;
            x1 += char_off_x;
        }

        // Snap to pixel
        if (cfg->PixelSnapH)
            advance_x = IM_ROUND(advance_x);

        // Bake spacing
        advance_x += cfg->GlyphExtraSpacing.x;
    }

    int glyph_idx = Glyphs.Size;
    Glyphs.resize(Glyphs.Size + 1);
    ImFontGlyph& glyph = Glyphs[glyph_idx];
    glyph.Codepoint = (unsigned int)codepoint;
    glyph.Visible = (x0 != x1) && (y0 != y1);
    glyph.Colored = false;
    glyph.X0 = x0;
    glyph.Y0 = y0;
    glyph.X1 = x1;
    glyph.Y1 = y1;
    glyph.U0 = u0;
    glyph.V0 = v0;
    glyph.U1 = u1;
    glyph.V1 = v1;
    glyph.AdvanceX = advance_x;

    // Compute rough surface usage metrics (+1 to account for average padding, +0.99 to round)
    // We use (U1-U0)*TexWidth instead of X1-X0 to account for oversampling.
    float pad = ContainerAtlas->TexGlyphPadding + 0.99f;
    DirtyLookupTables = true;
    MetricsTotalSurface += (int)((glyph.U1 - glyph.U0) * ContainerAtlas->TexWidth + pad) * (int)((glyph.V1 - glyph.V0) * ContainerAtlas->TexHeight + pad);
}

void ImFont::AddRemapChar(ImWchar dst, ImWchar src, bool overwrite_dst)
{
    IM_ASSERT(IndexLookup.Size > 0);    // Currently this can only be called AFTER the font has been built, aka after calling ImFontAtlas::GetTexDataAs*() function.
    unsigned int index_size = (unsigned int)IndexLookup.Size;

    if (dst < index_size && IndexLookup.Data[dst] == (ImWchar)-1 && !overwrite_dst) // 'dst' already exists
        return;
    if (src >= index_size && dst >= index_size) // both 'dst' and 'src' don't exist -> no-op
        return;

    GrowIndex(dst + 1);
    IndexLookup[dst] = (src < index_size) ? IndexLookup.Data[src] : (ImWchar)-1;
    IndexAdvanceX[dst] = (src < index_size) ? IndexAdvanceX.Data[src] : 1.0f;
}

// Find glyph, return fallback if missing
const ImFontGlyph* ImFont::FindGlyph(ImWchar c)
{
    if (c >= (size_t)IndexLookup.Size)
        return FallbackGlyph;
    const ImWchar i = IndexLookup.Data[c];
    if (i == (ImWchar)-1)
        return FallbackGlyph;
    return &Glyphs.Data[i];
}

const ImFontGlyph* ImFont::FindGlyphNoFallback(ImWchar c)
{
    if (c >= (size_t)IndexLookup.Size)
        return NULL;
    const ImWchar i = IndexLookup.Data[c];
    if (i == (ImWchar)-1)
        return NULL;
    return &Glyphs.Data[i];
}

// Trim trailing space and find beginning of next line
static inline const char* CalcWordWrapNextLineStartA(const char* text, const char* text_end)
{
    while (text < text_end && ImCharIsBlankA(*text))
        text++;
    if (*text == '\n')
        text++;
    return text;
}

#define ImFontGetCharAdvanceX(_FONT, _CH)  ((int)(_CH) < (_FONT)->IndexAdvanceX.Size ? (_FONT)->IndexAdvanceX.Data[_CH] : (_FONT)->FallbackAdvanceX)

// Simple word-wrapping for English, not full-featured. Please submit failing cases!
// This will return the next location to wrap from. If no wrapping if necessary, this will fast-forward to e.g. text_end.
// FIXME: Much possible improvements (don't cut things like "word !", "word!!!" but cut within "word,,,,", more sensible support for punctuations, support for Unicode punctuations, etc.)
const char* ImFont::CalcWordWrapPositionA(float scale, const char* text, const char* text_end, float wrap_width)
{
    // For references, possible wrap point marked with ^
    //  "aaa bbb, ccc,ddd. eee   fff. ggg!"
    //      ^    ^    ^   ^   ^__    ^    ^

    // List of hardcoded separators: .,;!?'"

    // Skip extra blanks after a line returns (that includes not counting them in width computation)
    // e.g. "Hello    world" --> "Hello" "World"

    // Cut words that cannot possibly fit within one line.
    // e.g.: "The tropical fish" with ~5 characters worth of width --> "The tr" "opical" "fish"
    float line_width = 0.0f;
    float word_width = 0.0f;
    float blank_width = 0.0f;
    wrap_width /= scale; // We work with unscaled widths to avoid scaling every characters

    const char* word_end = text;
    const char* prev_word_end = NULL;
    bool inside_word = true;

    const char* s = text;
    IM_ASSERT(text_end != NULL);
    while (s < text_end)
    {
        unsigned int c = (unsigned int)*s;
        const char* next_s;
        if (c < 0x80)
            next_s = s + 1;
        else
            next_s = s + ImTextCharFromUtf8(&c, s, text_end);

        if (c < 32)
        {
            if (c == '\n')
            {
                line_width = word_width = blank_width = 0.0f;
                inside_word = true;
                s = next_s;
                continue;
            }
            if (c == '\r')
            {
                s = next_s;
                continue;
            }
        }

        const float char_width = ImFontGetCharAdvanceX(this, c);
        if (ImCharIsBlankW(c))
        {
            if (inside_word)
            {
                line_width += blank_width;
                blank_width = 0.0f;
                word_end = s;
            }
            blank_width += char_width;
            inside_word = false;
        }
        else
        {
            word_width += char_width;
            if (inside_word)
            {
                word_end = next_s;
            }
            else
            {
                prev_word_end = word_end;
                line_width += word_width + blank_width;
                word_width = blank_width = 0.0f;
            }

            // Allow wrapping after punctuation.
            inside_word = (c != '.' && c != ',' && c != ';' && c != '!' && c != '?' && c != '\"');
        }

        // We ignore blank width at the end of the line (they can be skipped)
        if (line_width + word_width > wrap_width)
        {
            // Words that cannot possibly fit within an entire line will be cut anywhere.
            if (word_width < wrap_width)
                s = prev_word_end ? prev_word_end : word_end;
            break;
        }

        s = next_s;
    }

    // Wrap_width is too small to fit anything. Force displaying 1 character to minimize the height discontinuity.
    // +1 may not be a character start point in UTF-8 but it's ok because caller loops use (text >= word_wrap_eol).
    if (s == text && text < text_end)
        return s + 1;
    return s;
}

ImVec2 ImFont::CalcTextSizeA(float size, float max_width, float wrap_width, const char* text_begin, const char* text_end, const char** remaining)
{
    if (!text_end)
        text_end = text_begin + strlen(text_begin); // FIXME-OPT: Need to avoid this.

    const float line_height = size;
    const float scale = size / FontSize;

    ImVec2 text_size = ImVec2(0, 0);
    float line_width = 0.0f;

    const bool word_wrap_enabled = (wrap_width > 0.0f);
    const char* word_wrap_eol = NULL;

    const char* s = text_begin;
    while (s < text_end)
    {
        if (word_wrap_enabled)
        {
            // Calculate how far we can render. Requires two passes on the string data but keeps the code simple and not intrusive for what's essentially an uncommon feature.
            if (!word_wrap_eol)
                word_wrap_eol = CalcWordWrapPositionA(scale, s, text_end, wrap_width - line_width);

            if (s >= word_wrap_eol)
            {
                if (text_size.x < line_width)
                    text_size.x = line_width;
                text_size.y += line_height;
                line_width = 0.0f;
                word_wrap_eol = NULL;
                s = CalcWordWrapNextLineStartA(s, text_end); // Wrapping skips upcoming blanks
                continue;
            }
        }

        // Decode and advance source
        const char* prev_s = s;
        unsigned int c = (unsigned int)*s;
        if (c < 0x80)
            s += 1;
        else
            s += ImTextCharFromUtf8(&c, s, text_end);

        if (c < 32)
        {
            if (c == '\n')
            {
                text_size.x = ImMax(text_size.x, line_width);
                text_size.y += line_height;
                line_width = 0.0f;
                continue;
            }
            if (c == '\r')
                continue;
        }

        const float char_width = ImFontGetCharAdvanceX(this, c) * scale;
        if (line_width + char_width >= max_width)
        {
            s = prev_s;
            break;
        }

        line_width += char_width;
    }

    if (text_size.x < line_width)
        text_size.x = line_width;

    if (line_width > 0 || text_size.y == 0.0f)
        text_size.y += line_height;

    if (remaining)
        *remaining = s;

    return text_size;
}

// Note: as with every ImDrawList drawing function, this expects that the font atlas texture is bound.
void ImFont::RenderChar(ImDrawList* draw_list, float size, const ImVec2& pos, ImU32 col, ImWchar c)
{
    const ImFontGlyph* glyph = FindGlyph(c);
    if (!glyph || !glyph->Visible)
        return;
    if (glyph->Colored)
        col |= ~IM_COL32_A_MASK;
    float scale = (size >= 0.0f) ? (size / FontSize) : 1.0f;
    float x = IM_TRUNC(pos.x);
    float y = IM_TRUNC(pos.y);
    draw_list->PrimReserve(6, 4);
    draw_list->PrimRectUV(ImVec2(x + glyph->X0 * scale, y + glyph->Y0 * scale), ImVec2(x + glyph->X1 * scale, y + glyph->Y1 * scale), ImVec2(glyph->U0, glyph->V0), ImVec2(glyph->U1, glyph->V1), col);
}

// Note: as with every ImDrawList drawing function, this expects that the font atlas texture is bound.
void ImFont::RenderText(ImDrawList* draw_list, float size, const ImVec2& pos, ImU32 col, const ImVec4& clip_rect, const char* text_begin, const char* text_end, float wrap_width, bool cpu_fine_clip)
{
    if (!text_end)
        text_end = text_begin + strlen(text_begin); // ImGui:: functions generally already provides a valid text_end, so this is merely to handle direct calls.

    // Align to be pixel perfect
    float x = IM_TRUNC(pos.x);
    float y = IM_TRUNC(pos.y);
    if (y > clip_rect.w)
        return;

    const float scale = size / FontSize;
    const float line_height = FontSize * scale;
    const float origin_x = x;
    const bool word_wrap_enabled = (wrap_width > 0.0f);

    // Fast-forward to first visible line
    const char* s = text_begin;
    if (y + line_height < clip_rect.y)
        while (y + line_height < clip_rect.y && s < text_end)
        {
            const char* line_end = (const char*)memchr(s, '\n', text_end - s);
            if (word_wrap_enabled)
            {
                // FIXME-OPT: This is not optimal as do first do a search for \n before calling CalcWordWrapPositionA().
                // If the specs for CalcWordWrapPositionA() were reworked to optionally return on \n we could combine both.
                // However it is still better than nothing performing the fast-forward!
                s = CalcWordWrapPositionA(scale, s, line_end ? line_end : text_end, wrap_width);
                s = CalcWordWrapNextLineStartA(s, text_end);
            }
            else
            {
                s = line_end ? line_end + 1 : text_end;
            }
            y += line_height;
        }

    // For large text, scan for the last visible line in order to avoid over-reserving in the call to PrimReserve()
    // Note that very large horizontal line will still be affected by the issue (e.g. a one megabyte string buffer without a newline will likely crash atm)
    if (text_end - s > 10000 && !word_wrap_enabled)
    {
        const char* s_end = s;
        float y_end = y;
        while (y_end < clip_rect.w && s_end < text_end)
        {
            s_end = (const char*)memchr(s_end, '\n', text_end - s_end);
            s_end = s_end ? s_end + 1 : text_end;
            y_end += line_height;
        }
        text_end = s_end;
    }
    if (s == text_end)
        return;

    // Reserve vertices for remaining worse case (over-reserving is useful and easily amortized)
    const int vtx_count_max = (int)(text_end - s) * 4;
    const int idx_count_max = (int)(text_end - s) * 6;
    const int idx_expected_size = draw_list->IdxBuffer.Size + idx_count_max;
    draw_list->PrimReserve(idx_count_max, vtx_count_max);
    ImDrawVert*  vtx_write = draw_list->_VtxWritePtr;
    ImDrawIdx*   idx_write = draw_list->_IdxWritePtr;
    unsigned int vtx_index = draw_list->_VtxCurrentIdx;

    const ImU32 col_untinted = col | ~IM_COL32_A_MASK;
    const char* word_wrap_eol = NULL;

    while (s < text_end)
    {
        if (word_wrap_enabled)
        {
            // Calculate how far we can render. Requires two passes on the string data but keeps the code simple and not intrusive for what's essentially an uncommon feature.
            if (!word_wrap_eol)
                word_wrap_eol = CalcWordWrapPositionA(scale, s, text_end, wrap_width - (x - origin_x));

            if (s >= word_wrap_eol)
            {
                x = origin_x;
                y += line_height;
                if (y > clip_rect.w)
                    break; // break out of main loop
                word_wrap_eol = NULL;
                s = CalcWordWrapNextLineStartA(s, text_end); // Wrapping skips upcoming blanks
                continue;
            }
        }

        // Decode and advance source
        unsigned int c = (unsigned int)*s;
        if (c < 0x80)
            s += 1;
        else
            s += ImTextCharFromUtf8(&c, s, text_end);

        if (c < 32)
        {
            if (c == '\n')
            {
                x = origin_x;
                y += line_height;
                if (y > clip_rect.w)
                    break; // break out of main loop
                continue;
            }
            if (c == '\r')
                continue;
        }

        const ImFontGlyph* glyph = FindGlyph((ImWchar)c);
        if (glyph == NULL)
            continue;

        float char_width = glyph->AdvanceX * scale;
        if (glyph->Visible)
        {
            // We don't do a second finer clipping test on the Y axis as we've already skipped anything before clip_rect.y and exit once we pass clip_rect.w
            float x1 = x + glyph->X0 * scale;
            float x2 = x + glyph->X1 * scale;
            float y1 = y + glyph->Y0 * scale;
            float y2 = y + glyph->Y1 * scale;
            if (x1 <= clip_rect.z && x2 >= clip_rect.x)
            {
                // Render a character
                float u1 = glyph->U0;
                float v1 = glyph->V0;
                float u2 = glyph->U1;
                float v2 = glyph->V1;

                // CPU side clipping used to fit text in their frame when the frame is too small. Only does clipping for axis aligned quads.
                if (cpu_fine_clip)
                {
                    if (x1 < clip_rect.x)
                    {
                        u1 = u1 + (1.0f - (x2 - clip_rect.x) / (x2 - x1)) * (u2 - u1);
                        x1 = clip_rect.x;
                    }
                    if (y1 < clip_rect.y)
                    {
                        v1 = v1 + (1.0f - (y2 - clip_rect.y) / (y2 - y1)) * (v2 - v1);
                        y1 = clip_rect.y;
                    }
                    if (x2 > clip_rect.z)
                    {
                        u2 = u1 + ((clip_rect.z - x1) / (x2 - x1)) * (u2 - u1);
                        x2 = clip_rect.z;
                    }
                    if (y2 > clip_rect.w)
                    {
                        v2 = v1 + ((clip_rect.w - y1) / (y2 - y1)) * (v2 - v1);
                        y2 = clip_rect.w;
                    }
                    if (y1 >= y2)
                    {
                        x += char_width;
                        continue;
                    }
                }

                // Support for untinted glyphs
                ImU32 glyph_col = glyph->Colored ? col_untinted : col;

                // We are NOT calling PrimRectUV() here because non-inlined causes too much overhead in a debug builds. Inlined here:
                {
                    vtx_write[0].pos.x = x1; vtx_write[0].pos.y = y1; vtx_write[0].col = glyph_col; vtx_write[0].uv.x = u1; vtx_write[0].uv.y = v1;
                    vtx_write[1].pos.x = x2; vtx_write[1].pos.y = y1; vtx_write[1].col = glyph_col; vtx_write[1].uv.x = u2; vtx_write[1].uv.y = v1;
                    vtx_write[2].pos.x = x2; vtx_write[2].pos.y = y2; vtx_write[2].col = glyph_col; vtx_write[2].uv.x = u2; vtx_write[2].uv.y = v2;
                    vtx_write[3].pos.x = x1; vtx_write[3].pos.y = y2; vtx_write[3].col = glyph_col; vtx_write[3].uv.x = u1; vtx_write[3].uv.y = v2;
                    idx_write[0] = (ImDrawIdx)(vtx_index); idx_write[1] = (ImDrawIdx)(vtx_index + 1); idx_write[2] = (ImDrawIdx)(vtx_index + 2);
                    idx_write[3] = (ImDrawIdx)(vtx_index); idx_write[4] = (ImDrawIdx)(vtx_index + 2); idx_write[5] = (ImDrawIdx)(vtx_index + 3);
                    vtx_write += 4;
                    vtx_index += 4;
                    idx_write += 6;
                }
            }
        }
        x += char_width;
    }

    // Give back unused vertices (clipped ones, blanks) ~ this is essentially a PrimUnreserve() action.
    draw_list->VtxBuffer.Size = (int)(vtx_write - draw_list->VtxBuffer.Data); // Same as calling shrink()
    draw_list->IdxBuffer.Size = (int)(idx_write - draw_list->IdxBuffer.Data);
    draw_list->CmdBuffer[draw_list->CmdBuffer.Size - 1].ElemCount -= (idx_expected_size - draw_list->IdxBuffer.Size);
    draw_list->_VtxWritePtr = vtx_write;
    draw_list->_IdxWritePtr = idx_write;
    draw_list->_VtxCurrentIdx = vtx_index;
}

//-----------------------------------------------------------------------------
// [SECTION] ImGui Internal Render Helpers
//-----------------------------------------------------------------------------
// Vaguely redesigned to stop accessing ImGui global state:
// - RenderArrow()
// - RenderBullet()
// - RenderCheckMark()
// - RenderArrowPointingAt()
// - RenderRectFilledRangeH()
// - RenderRectFilledWithHole()
//-----------------------------------------------------------------------------
// Function in need of a redesign (legacy mess)
// - RenderColorRectWithAlphaCheckerboard()
//-----------------------------------------------------------------------------

// Render an arrow aimed to be aligned with text (p_min is a position in the same space text would be positioned). To e.g. denote expanded/collapsed state
void ImGui::RenderArrow(ImDrawList* draw_list, ImVec2 pos, ImU32 col, ImGuiDir dir, float scale)
{
    const float h = draw_list->_Data->FontSize * 1.00f;
    float r = h * 0.40f * scale;
    ImVec2 center = pos + ImVec2(h * 0.50f, h * 0.50f * scale);

    ImVec2 a, b, c;
    switch (dir)
    {
    case ImGuiDir_Up:
    case ImGuiDir_Down:
        if (dir == ImGuiDir_Up) r = -r;
        a = ImVec2(+0.000f, +0.750f) * r;
        b = ImVec2(-0.866f, -0.750f) * r;
        c = ImVec2(+0.866f, -0.750f) * r;
        break;
    case ImGuiDir_Left:
    case ImGuiDir_Right:
        if (dir == ImGuiDir_Left) r = -r;
        a = ImVec2(+0.750f, +0.000f) * r;
        b = ImVec2(-0.750f, +0.866f) * r;
        c = ImVec2(-0.750f, -0.866f) * r;
        break;
    case ImGuiDir_None:
    case ImGuiDir_COUNT:
        IM_ASSERT(0);
        break;
    }
    draw_list->AddTriangleFilled(center + a, center + b, center + c, col);
}

void ImGui::RenderBullet(ImDrawList* draw_list, ImVec2 pos, ImU32 col)
{
    // FIXME-OPT: This should be baked in font.
    draw_list->AddCircleFilled(pos, draw_list->_Data->FontSize * 0.20f, col, 8);
}

void ImGui::RenderCheckMark(ImDrawList* draw_list, ImVec2 pos, ImU32 col, float sz)
{
    float thickness = ImMax(sz / 5.0f, 1.0f);
    sz -= thickness * 0.5f;
    pos += ImVec2(thickness * 0.25f, thickness * 0.25f);

    float third = sz / 3.0f;
    float bx = pos.x + third;
    float by = pos.y + sz - third * 0.5f;
    draw_list->PathLineTo(ImVec2(bx - third, by - third));
    draw_list->PathLineTo(ImVec2(bx, by));
    draw_list->PathLineTo(ImVec2(bx + third * 2.0f, by - third * 2.0f));
    draw_list->PathStroke(col, 0, thickness);
}

// Render an arrow. 'pos' is position of the arrow tip. half_sz.x is length from base to tip. half_sz.y is length on each side.
void ImGui::RenderArrowPointingAt(ImDrawList* draw_list, ImVec2 pos, ImVec2 half_sz, ImGuiDir direction, ImU32 col)
{
    switch (direction)
    {
    case ImGuiDir_Left:  draw_list->AddTriangleFilled(ImVec2(pos.x + half_sz.x, pos.y - half_sz.y), ImVec2(pos.x + half_sz.x, pos.y + half_sz.y), pos, col); return;
    case ImGuiDir_Right: draw_list->AddTriangleFilled(ImVec2(pos.x - half_sz.x, pos.y + half_sz.y), ImVec2(pos.x - half_sz.x, pos.y - half_sz.y), pos, col); return;
    case ImGuiDir_Up:    draw_list->AddTriangleFilled(ImVec2(pos.x + half_sz.x, pos.y + half_sz.y), ImVec2(pos.x - half_sz.x, pos.y + half_sz.y), pos, col); return;
    case ImGuiDir_Down:  draw_list->AddTriangleFilled(ImVec2(pos.x - half_sz.x, pos.y - half_sz.y), ImVec2(pos.x + half_sz.x, pos.y - half_sz.y), pos, col); return;
    case ImGuiDir_None: case ImGuiDir_COUNT: break; // Fix warnings
    }
}

static inline float ImAcos01(float x)
{
    if (x <= 0.0f) return IM_PI * 0.5f;
    if (x >= 1.0f) return 0.0f;
    return ImAcos(x);
    //return (-0.69813170079773212f * x * x - 0.87266462599716477f) * x + 1.5707963267948966f; // Cheap approximation, may be enough for what we do.
}

// FIXME: Cleanup and move code to ImDrawList.
void ImGui::RenderRectFilledRangeH(ImDrawList* draw_list, const ImRect& rect, ImU32 col, float x_start_norm, float x_end_norm, float rounding)
{
    if (x_end_norm == x_start_norm)
        return;
    if (x_start_norm > x_end_norm)
        ImSwap(x_start_norm, x_end_norm);

    ImVec2 p0 = ImVec2(ImLerp(rect.Min.x, rect.Max.x, x_start_norm), rect.Min.y);
    ImVec2 p1 = ImVec2(ImLerp(rect.Min.x, rect.Max.x, x_end_norm), rect.Max.y);
    if (rounding == 0.0f)
    {
        draw_list->AddRectFilled(p0, p1, col, 0.0f);
        return;
    }

    rounding = ImClamp(ImMin((rect.Max.x - rect.Min.x) * 0.5f, (rect.Max.y - rect.Min.y) * 0.5f) - 1.0f, 0.0f, rounding);
    const float inv_rounding = 1.0f / rounding;
    const float arc0_b = ImAcos01(1.0f - (p0.x - rect.Min.x) * inv_rounding);
    const float arc0_e = ImAcos01(1.0f - (p1.x - rect.Min.x) * inv_rounding);
    const float half_pi = IM_PI * 0.5f; // We will == compare to this because we know this is the exact value ImAcos01 can return.
    const float x0 = ImMax(p0.x, rect.Min.x + rounding);
    if (arc0_b == arc0_e)
    {
        draw_list->PathLineTo(ImVec2(x0, p1.y));
        draw_list->PathLineTo(ImVec2(x0, p0.y));
    }
    else if (arc0_b == 0.0f && arc0_e == half_pi)
    {
        draw_list->PathArcToFast(ImVec2(x0, p1.y - rounding), rounding, 3, 6); // BL
        draw_list->PathArcToFast(ImVec2(x0, p0.y + rounding), rounding, 6, 9); // TR
    }
    else
    {
        draw_list->PathArcTo(ImVec2(x0, p1.y - rounding), rounding, IM_PI - arc0_e, IM_PI - arc0_b); // BL
        draw_list->PathArcTo(ImVec2(x0, p0.y + rounding), rounding, IM_PI + arc0_b, IM_PI + arc0_e); // TR
    }
    if (p1.x > rect.Min.x + rounding)
    {
        const float arc1_b = ImAcos01(1.0f - (rect.Max.x - p1.x) * inv_rounding);
        const float arc1_e = ImAcos01(1.0f - (rect.Max.x - p0.x) * inv_rounding);
        const float x1 = ImMin(p1.x, rect.Max.x - rounding);
        if (arc1_b == arc1_e)
        {
            draw_list->PathLineTo(ImVec2(x1, p0.y));
            draw_list->PathLineTo(ImVec2(x1, p1.y));
        }
        else if (arc1_b == 0.0f && arc1_e == half_pi)
        {
            draw_list->PathArcToFast(ImVec2(x1, p0.y + rounding), rounding, 9, 12); // TR
            draw_list->PathArcToFast(ImVec2(x1, p1.y - rounding), rounding, 0, 3);  // BR
        }
        else
        {
            draw_list->PathArcTo(ImVec2(x1, p0.y + rounding), rounding, -arc1_e, -arc1_b); // TR
            draw_list->PathArcTo(ImVec2(x1, p1.y - rounding), rounding, +arc1_b, +arc1_e); // BR
        }
    }
    draw_list->PathFillConvex(col);
}

void ImGui::RenderRectFilledWithHole(ImDrawList* draw_list, const ImRect& outer, const ImRect& inner, ImU32 col, float rounding)
{
    const bool fill_L = (inner.Min.x > outer.Min.x);
    const bool fill_R = (inner.Max.x < outer.Max.x);
    const bool fill_U = (inner.Min.y > outer.Min.y);
    const bool fill_D = (inner.Max.y < outer.Max.y);
    if (fill_L) draw_list->AddRectFilled(ImVec2(outer.Min.x, inner.Min.y), ImVec2(inner.Min.x, inner.Max.y), col, rounding, ImDrawFlags_RoundCornersNone | (fill_U ? 0 : ImDrawFlags_RoundCornersTopLeft)    | (fill_D ? 0 : ImDrawFlags_RoundCornersBottomLeft));
    if (fill_R) draw_list->AddRectFilled(ImVec2(inner.Max.x, inner.Min.y), ImVec2(outer.Max.x, inner.Max.y), col, rounding, ImDrawFlags_RoundCornersNone | (fill_U ? 0 : ImDrawFlags_RoundCornersTopRight)   | (fill_D ? 0 : ImDrawFlags_RoundCornersBottomRight));
    if (fill_U) draw_list->AddRectFilled(ImVec2(inner.Min.x, outer.Min.y), ImVec2(inner.Max.x, inner.Min.y), col, rounding, ImDrawFlags_RoundCornersNone | (fill_L ? 0 : ImDrawFlags_RoundCornersTopLeft)    | (fill_R ? 0 : ImDrawFlags_RoundCornersTopRight));
    if (fill_D) draw_list->AddRectFilled(ImVec2(inner.Min.x, inner.Max.y), ImVec2(inner.Max.x, outer.Max.y), col, rounding, ImDrawFlags_RoundCornersNone | (fill_L ? 0 : ImDrawFlags_RoundCornersBottomLeft) | (fill_R ? 0 : ImDrawFlags_RoundCornersBottomRight));
    if (fill_L && fill_U) draw_list->AddRectFilled(ImVec2(outer.Min.x, outer.Min.y), ImVec2(inner.Min.x, inner.Min.y), col, rounding, ImDrawFlags_RoundCornersTopLeft);
    if (fill_R && fill_U) draw_list->AddRectFilled(ImVec2(inner.Max.x, outer.Min.y), ImVec2(outer.Max.x, inner.Min.y), col, rounding, ImDrawFlags_RoundCornersTopRight);
    if (fill_L && fill_D) draw_list->AddRectFilled(ImVec2(outer.Min.x, inner.Max.y), ImVec2(inner.Min.x, outer.Max.y), col, rounding, ImDrawFlags_RoundCornersBottomLeft);
    if (fill_R && fill_D) draw_list->AddRectFilled(ImVec2(inner.Max.x, inner.Max.y), ImVec2(outer.Max.x, outer.Max.y), col, rounding, ImDrawFlags_RoundCornersBottomRight);
}

// Helper for ColorPicker4()
// NB: This is rather brittle and will show artifact when rounding this enabled if rounded corners overlap multiple cells. Caller currently responsible for avoiding that.
// Spent a non reasonable amount of time trying to getting this right for ColorButton with rounding+anti-aliasing+ImGuiColorEditFlags_HalfAlphaPreview flag + various grid sizes and offsets, and eventually gave up... probably more reasonable to disable rounding altogether.
// FIXME: uses ImGui::GetColorU32
void ImGui::RenderColorRectWithAlphaCheckerboard(ImDrawList* draw_list, ImVec2 p_min, ImVec2 p_max, ImU32 col, float grid_step, ImVec2 grid_off, float rounding, ImDrawFlags flags)
{
    if ((flags & ImDrawFlags_RoundCornersMask_) == 0)
        flags = ImDrawFlags_RoundCornersDefault_;
    if (((col & IM_COL32_A_MASK) >> IM_COL32_A_SHIFT) < 0xFF)
    {
        ImU32 col_bg1 = GetColorU32(ImAlphaBlendColors(IM_COL32(204, 204, 204, 255), col));
        ImU32 col_bg2 = GetColorU32(ImAlphaBlendColors(IM_COL32(128, 128, 128, 255), col));
        draw_list->AddRectFilled(p_min, p_max, col_bg1, rounding, flags);

        int yi = 0;
        for (float y = p_min.y + grid_off.y; y < p_max.y; y += grid_step, yi++)
        {
            float y1 = ImClamp(y, p_min.y, p_max.y), y2 = ImMin(y + grid_step, p_max.y);
            if (y2 <= y1)
                continue;
            for (float x = p_min.x + grid_off.x + (yi & 1) * grid_step; x < p_max.x; x += grid_step * 2.0f)
            {
                float x1 = ImClamp(x, p_min.x, p_max.x), x2 = ImMin(x + grid_step, p_max.x);
                if (x2 <= x1)
                    continue;
                ImDrawFlags cell_flags = ImDrawFlags_RoundCornersNone;
                if (y1 <= p_min.y) { if (x1 <= p_min.x) cell_flags |= ImDrawFlags_RoundCornersTopLeft; if (x2 >= p_max.x) cell_flags |= ImDrawFlags_RoundCornersTopRight; }
                if (y2 >= p_max.y) { if (x1 <= p_min.x) cell_flags |= ImDrawFlags_RoundCornersBottomLeft; if (x2 >= p_max.x) cell_flags |= ImDrawFlags_RoundCornersBottomRight; }

                // Combine flags
                cell_flags = (flags == ImDrawFlags_RoundCornersNone || cell_flags == ImDrawFlags_RoundCornersNone) ? ImDrawFlags_RoundCornersNone : (cell_flags & flags);
                draw_list->AddRectFilled(ImVec2(x1, y1), ImVec2(x2, y2), col_bg2, rounding, cell_flags);
            }
        }
    }
    else
    {
        draw_list->AddRectFilled(p_min, p_max, col, rounding, flags);
    }
}

//-----------------------------------------------------------------------------
// [SECTION] Decompression code
//-----------------------------------------------------------------------------
// Compressed with stb_compress() then converted to a C array and encoded as base85.
// Use the program in misc/fonts/binary_to_compressed_c.cpp to create the array from a TTF file.
// The purpose of encoding as base85 instead of "0x00,0x01,..." style is only save on _source code_ size.
// Decompression from stb.h (public domain) by Sean Barrett https://github.com/nothings/stb/blob/master/stb.h
//-----------------------------------------------------------------------------

static unsigned int stb_decompress_length(const unsigned char *input)
{
    return (input[8] << 24) + (input[9] << 16) + (input[10] << 8) + input[11];
}

static unsigned char *stb__barrier_out_e, *stb__barrier_out_b;
static const unsigned char *stb__barrier_in_b;
static unsigned char *stb__dout;
static void stb__match(const unsigned char *data, unsigned int length)
{
    // INVERSE of memmove... write each byte before copying the next...
    IM_ASSERT(stb__dout + length <= stb__barrier_out_e);
    if (stb__dout + length > stb__barrier_out_e) { stb__dout += length; return; }
    if (data < stb__barrier_out_b) { stb__dout = stb__barrier_out_e+1; return; }
    while (length--) *stb__dout++ = *data++;
}

static void stb__lit(const unsigned char *data, unsigned int length)
{
    IM_ASSERT(stb__dout + length <= stb__barrier_out_e);
    if (stb__dout + length > stb__barrier_out_e) { stb__dout += length; return; }
    if (data < stb__barrier_in_b) { stb__dout = stb__barrier_out_e+1; return; }
    memcpy(stb__dout, data, length);
    stb__dout += length;
}

#define stb__in2(x)   ((i[x] << 8) + i[(x)+1])
#define stb__in3(x)   ((i[x] << 16) + stb__in2((x)+1))
#define stb__in4(x)   ((i[x] << 24) + stb__in3((x)+1))

static const unsigned char *stb_decompress_token(const unsigned char *i)
{
    if (*i >= 0x20) { // use fewer if's for cases that expand small
        if (*i >= 0x80)       stb__match(stb__dout-i[1]-1, i[0] - 0x80 + 1), i += 2;
        else if (*i >= 0x40)  stb__match(stb__dout-(stb__in2(0) - 0x4000 + 1), i[2]+1), i += 3;
        else /* *i >= 0x20 */ stb__lit(i+1, i[0] - 0x20 + 1), i += 1 + (i[0] - 0x20 + 1);
    } else { // more ifs for cases that expand large, since overhead is amortized
        if (*i >= 0x18)       stb__match(stb__dout-(stb__in3(0) - 0x180000 + 1), i[3]+1), i += 4;
        else if (*i >= 0x10)  stb__match(stb__dout-(stb__in3(0) - 0x100000 + 1), stb__in2(3)+1), i += 5;
        else if (*i >= 0x08)  stb__lit(i+2, stb__in2(0) - 0x0800 + 1), i += 2 + (stb__in2(0) - 0x0800 + 1);
        else if (*i == 0x07)  stb__lit(i+3, stb__in2(1) + 1), i += 3 + (stb__in2(1) + 1);
        else if (*i == 0x06)  stb__match(stb__dout-(stb__in3(1)+1), i[4]+1), i += 5;
        else if (*i == 0x04)  stb__match(stb__dout-(stb__in3(1)+1), stb__in2(4)+1), i += 6;
    }
    return i;
}

static unsigned int stb_adler32(unsigned int adler32, unsigned char *buffer, unsigned int buflen)
{
    const unsigned long ADLER_MOD = 65521;
    unsigned long s1 = adler32 & 0xffff, s2 = adler32 >> 16;
    unsigned long blocklen = buflen % 5552;

    unsigned long i;
    while (buflen) {
        for (i=0; i + 7 < blocklen; i += 8) {
            s1 += buffer[0], s2 += s1;
            s1 += buffer[1], s2 += s1;
            s1 += buffer[2], s2 += s1;
            s1 += buffer[3], s2 += s1;
            s1 += buffer[4], s2 += s1;
            s1 += buffer[5], s2 += s1;
            s1 += buffer[6], s2 += s1;
            s1 += buffer[7], s2 += s1;

            buffer += 8;
        }

        for (; i < blocklen; ++i)
            s1 += *buffer++, s2 += s1;

        s1 %= ADLER_MOD, s2 %= ADLER_MOD;
        buflen -= blocklen;
        blocklen = 5552;
    }
    return (unsigned int)(s2 << 16) + (unsigned int)s1;
}

static unsigned int stb_decompress(unsigned char *output, const unsigned char *i, unsigned int /*length*/)
{
    if (stb__in4(0) != 0x57bC0000) return 0;
    if (stb__in4(4) != 0)          return 0; // error! stream is > 4GB
    const unsigned int olen = stb_decompress_length(i);
    stb__barrier_in_b = i;
    stb__barrier_out_e = output + olen;
    stb__barrier_out_b = output;
    i += 16;

    stb__dout = output;
    for (;;) {
        const unsigned char *old_i = i;
        i = stb_decompress_token(i);
        if (i == old_i) {
            if (*i == 0x05 && i[1] == 0xfa) {
                IM_ASSERT(stb__dout == output + olen);
                if (stb__dout != output + olen) return 0;
                if (stb_adler32(1, output, olen) != (unsigned int) stb__in4(2))
                    return 0;
                return olen;
            } else {
                IM_ASSERT(0); /* NOTREACHED */
                return 0;
            }
        }
        IM_ASSERT(stb__dout <= output + olen);
        if (stb__dout > output + olen)
            return 0;
    }
}

//-----------------------------------------------------------------------------
// [SECTION] Default font data (ProggyClean.ttf)
//-----------------------------------------------------------------------------
// ProggyClean.ttf
// Copyright (c) 2004, 2005 Tristan Grimmer
// MIT license (see License.txt in http://www.proggyfonts.net/index.php?menu=download)
// Download and more information at http://www.proggyfonts.net or http://upperboundsinteractive.com/fonts.php
//-----------------------------------------------------------------------------

#ifndef IMGUI_DISABLE_DEFAULT_FONT

// File: 'OpenSans-Medium.ttf' (162588 bytes)
// Exported using binary_to_compressed_c.exe -u8 "OpenSans-Medium.ttf" open_sans_medium
static const unsigned int open_sans_medium_ttf_compressed_size = 99863;
static const unsigned char open_sans_medium_ttf_compressed_data[99863] =
{
    87,188,0,0,0,0,0,0,0,1,255,160,0,4,0,0,37,0,1,0,0,0,18,130,4,8,62,4,0,32,71,68,69,70,175,171,181,85,0,0,3,112,0,0,1,190,71,80,79,83,155,221,232,118,0,0,118,116,0,0,57,244,71,83,85,
    66,19,99,31,101,0,0,30,156,0,0,15,6,79,83,47,50,150,164,131,65,0,0,2,20,130,69,40,96,83,84,65,84,95,18,64,221,130,59,32,184,130,15,44,90,99,109,97,112,195,56,31,128,0,0,7,208,130,83,
    40,246,99,118,116,32,61,66,44,200,130,47,130,79,45,0,252,102,112,103,109,226,25,158,90,0,0,45,164,130,79,40,148,103,97,115,112,0,21,0,35,130,63,32,44,130,63,56,16,103,108,121,102,96,
    3,94,78,0,0,176,104,0,1,79,54,104,101,97,100,31,185,234,141,130,31,130,83,34,0,54,104,130,16,35,13,224,9,30,130,15,63,92,0,0,0,36,104,109,116,120,179,181,54,215,0,0,61,56,0,0,17,206,
    108,111,99,97,149,227,68,21,0,0,11,130,115,41,8,234,109,97,120,112,7,251,16,165,130,47,32,60,130,47,55,32,110,97,109,101,109,227,121,145,0,0,20,180,0,0,9,232,112,111,115,116,38,172,
    156,130,219,8,37,79,8,0,0,39,106,112,114,101,112,133,253,123,233,0,0,5,48,0,0,2,159,0,1,0,3,0,8,0,10,0,13,0,7,255,255,0,15,130,15,50,0,4,116,0,145,0,22,0,95,0,5,0,2,0,16,0,47,0,154,
    130,41,34,190,15,131,130,41,32,1,131,31,35,8,141,253,168,130,91,38,224,251,120,253,34,9,216,131,17,130,121,136,2,33,4,115,132,17,46,3,0,197,120,126,219,124,95,15,60,245,0,11,8,0,131,
    0,35,217,204,194,247,131,7,35,225,123,219,169,130,59,36,200,9,216,8,101,130,15,32,6,130,101,130,6,130,2,34,1,0,1,130,139,32,3,130,9,32,20,130,107,48,0,0,44,0,2,119,100,116,104,1,1,
    0,0,119,103,104,116,130,6,42,1,105,116,97,108,1,28,0,2,0,34,130,161,32,6,130,37,36,2,0,2,1,29,130,49,32,0,130,59,131,3,32,1,130,4,34,27,1,244,130,17,130,9,130,27,34,26,0,100,130,11,
    36,0,0,4,4,166,130,21,32,5,130,247,34,51,4,205,130,17,33,154,5,132,7,37,2,205,0,50,2,146,130,15,136,2,39,224,0,2,255,64,0,32,27,130,16,32,40,130,3,42,0,71,79,79,71,1,192,0,0,255,253,
    133,247,35,8,254,2,139,130,99,32,159,131,27,35,4,72,5,182,130,7,38,32,0,4,6,20,0,11,130,11,32,22,131,3,35,4,75,0,20,130,47,32,234,130,3,32,236,130,3,36,234,254,22,255,254,130,23,32,
    21,130,11,32,235,130,45,36,168,0,170,0,150,130,1,34,166,0,130,130,1,32,171,130,9,8,32,113,0,159,0,143,0,169,0,166,0,200,0,109,0,138,0,154,0,107,0,142,0,155,0,122,0,164,0,141,1,58,0,
    132,130,17,32,162,130,23,38,238,0,133,0,120,1,72,130,5,32,122,130,17,32,158,130,71,32,179,132,61,38,133,0,144,0,153,0,159,130,45,34,169,0,176,130,55,35,166,0,172,0,131,73,32,122,130,
    95,133,77,32,130,130,61,32,146,130,25,32,160,130,99,44,122,0,163,0,171,0,175,0,131,0,140,0,152,130,93,36,113,0,128,0,135,130,127,42,155,0,165,0,125,0,134,0,139,0,149,130,41,48,165,
    0,174,0,238,0,120,0,126,0,136,0,147,1,72,0,121,130,37,131,27,32,148,130,121,52,167,6,194,3,122,5,10,0,20,255,56,2,158,3,167,0,1,0,2,0,112,130,217,131,2,32,14,130,13,32,3,130,8,32,62,
    130,3,32,22,130,3,32,16,130,15,34,1,4,35,130,33,8,36,6,1,83,1,84,0,0,3,116,3,116,0,2,3,118,3,118,0,3,4,1,4,13,0,4,4,17,4,20,0,17,4,30,4,30,0,21,130,45,8,32,16,2,53,4,14,4,15,4,16,4,
    21,4,22,4,23,4,24,4,25,4,26,4,27,4,28,4,29,4,32,4,34,4,37,130,75,36,55,0,36,0,61,130,43,34,68,0,93,130,5,34,108,0,108,130,5,34,124,0,124,130,5,34,130,0,141,130,5,34,146,0,152,130,5,
    34,154,0,184,130,5,34,186,0,222,130,5,34,224,0,224,130,5,34,226,0,226,130,5,34,228,0,228,130,5,34,230,0,233,130,5,34,235,0,235,130,5,34,237,0,237,130,5,34,239,0,239,130,5,34,241,0,
    241,130,5,36,244,1,73,0,1,132,177,36,3,1,85,1,85,130,11,34,87,1,88,130,5,34,90,1,101,130,5,34,103,1,117,130,5,34,119,1,159,130,5,46,162,2,0,0,1,2,53,2,53,0,3,2,74,2,74,130,11,34,77,
    2,77,130,5,34,79,2,82,130,5,34,84,2,87,130,5,34,89,2,118,130,5,34,125,2,126,130,5,34,130,2,176,130,5,34,178,2,181,130,5,34,183,2,196,130,5,40,198,3,49,0,1,3,51,3,51,130,5,34,53,3,97,
    130,5,34,109,3,115,130,5,39,116,3,116,0,3,3,117,3,130,107,65,47,5,35,3,122,3,132,130,23,40,138,3,142,0,2,3,143,3,143,130,11,34,148,3,149,130,5,34,151,3,164,130,5,34,166,3,172,130,5,
    33,174,3,130,95,35,3,179,3,179,130,11,34,182,3,190,130,5,34,192,3,192,130,5,46,201,3,227,0,1,4,1,4,37,0,3,4,111,4,112,130,11,32,114,67,171,5,8,164,64,255,122,60,121,85,121,89,118,56,
    79,31,117,56,255,31,116,56,171,31,115,54,205,31,114,54,255,31,113,54,171,31,112,55,255,31,111,53,255,31,110,51,94,31,109,51,255,31,108,52,171,31,107,52,255,31,106,50,255,31,105,48,
    103,31,104,48,255,31,103,48,114,31,102,48,69,31,101,49,255,31,100,49,205,31,99,49,79,31,98,47,94,31,97,47,255,31,96,46,79,31,95,46,171,31,94,46,255,31,93,46,54,31,92,45,255,31,91,44,
    94,31,90,44,255,31,89,44,103,31,88,43,94,31,87,43,147,31,86,43,255,31,85,42,255,31,84,41,94,31,83,41,171,31,82,41,255,31,81,40,128,31,80,40,255,31,79,130,7,8,115,78,39,255,31,77,38,
    255,31,76,37,255,31,75,37,128,31,74,37,64,31,73,36,255,31,72,35,255,31,71,34,171,31,70,34,255,31,69,34,94,31,68,33,147,31,67,33,255,31,66,31,205,31,65,31,255,31,64,31,171,31,63,32,
    255,31,62,32,103,31,61,30,255,31,60,29,255,31,59,28,114,31,58,28,255,31,57,28,79,31,55,64,194,54,94,31,52,51,79,31,49,48,43,31,41,40,79,31,40,21,27,25,92,39,27,45,31,38,130,98,53,37,
    14,26,25,92,36,26,49,31,35,25,31,31,34,25,255,31,33,31,103,31,32,130,85,59,31,28,24,22,92,30,24,28,31,29,23,255,31,28,22,255,31,27,50,25,31,91,24,56,22,55,91,26,131,9,32,23,131,9,8,
    125,21,25,62,22,255,90,19,49,18,85,17,49,16,85,18,89,16,89,13,50,12,85,5,50,4,85,12,89,4,89,15,4,127,4,239,4,3,15,255,14,85,11,50,10,85,7,50,6,85,1,95,0,85,14,89,10,89,6,89,207,6,239,
    6,2,0,89,111,0,127,0,175,0,239,0,4,16,0,1,9,50,8,85,3,50,2,85,8,89,2,89,15,2,127,2,239,2,3,16,0,3,64,64,5,1,184,1,144,176,84,43,75,184,7,255,82,75,176,9,80,91,176,1,136,176,37,83,131,
    5,62,64,81,90,176,6,136,176,0,85,90,91,88,177,1,1,142,89,133,141,141,0,29,66,75,176,144,83,88,178,3,0,130,10,52,89,177,2,2,67,81,88,177,4,3,142,89,66,115,0,43,0,43,43,43,115,130,7,
    132,10,131,12,130,14,133,21,133,5,33,1,43,137,1,130,30,32,1,130,46,130,3,135,27,130,28,134,20,133,58,130,29,130,18,130,5,137,14,133,48,32,43,132,104,130,11,136,33,131,8,140,37,130,
    12,33,24,0,130,0,35,2,0,0,0,70,23,7,32,1,130,11,36,20,0,4,3,226,130,7,38,224,0,128,0,6,0,96,130,9,8,220,13,0,126,1,48,1,49,1,97,1,99,1,127,1,146,1,161,1,176,1,237,1,240,1,255,2,27,
    2,55,2,89,2,188,2,199,2,201,2,221,2,243,3,4,3,12,3,15,3,18,3,35,3,40,3,138,3,140,3,161,3,206,3,210,3,214,4,0,4,12,4,13,4,79,4,80,4,92,4,95,4,130,4,134,4,143,4,145,5,19,5,189,5,190,
    5,194,5,199,5,234,30,1,30,63,30,133,30,158,30,241,30,243,30,249,31,77,31,222,32,11,32,21,32,30,32,34,32,38,32,48,32,51,32,58,32,60,32,68,32,112,32,122,32,127,32,137,32,138,32,142,32,
    156,32,164,32,167,32,172,33,5,33,19,33,22,33,32,33,34,33,38,33,46,33,94,34,2,34,6,34,15,34,18,34,21,34,26,34,30,34,43,34,72,34,96,34,101,37,202,167,181,171,83,251,4,251,54,251,60,251,
    62,251,65,251,68,251,75,254,255,255,253,255,255,130,223,38,0,0,13,0,32,0,160,130,225,36,50,1,98,1,100,130,225,36,160,1,175,1,234,130,225,34,250,2,24,134,225,32,198,130,225,32,216,130,
    225,34,0,3,6,134,225,34,38,3,132,130,225,36,142,3,163,3,209,132,225,32,1,130,225,32,14,130,225,46,81,4,93,4,96,4,131,4,136,4,144,4,146,5,176,130,225,32,193,130,225,38,208,30,0,30,62,
    30,128,130,225,36,160,30,242,30,244,132,225,38,0,32,19,32,23,32,32,132,225,34,50,32,57,134,225,36,116,32,124,32,128,130,225,36,140,32,149,32,163,130,225,32,170,142,225,32,91,134,225,
    32,17,140,225,32,100,130,225,32,179,130,225,36,0,251,42,251,56,130,225,36,64,251,67,251,70,130,225,32,252,130,225,8,52,1,255,245,255,227,255,194,2,126,255,193,2,11,255,193,255,175,
    0,180,0,167,1,133,0,90,255,72,0,0,1,121,1,26,255,143,254,132,254,131,254,117,255,96,1,1,0,0,0,253,0,251,0,235,130,6,54,207,253,206,253,205,253,204,254,123,254,120,254,89,253,154,254,
    77,253,153,254,11,253,152,130,25,32,253,130,3,8,54,248,253,103,253,246,254,101,254,165,254,98,254,94,253,249,228,81,228,17,227,121,228,241,228,106,227,13,228,104,228,40,227,152,226,
    59,225,238,225,237,225,236,225,233,225,224,225,223,225,218,225,217,225,210,227,7,130,95,8,77,0,227,217,227,224,0,0,227,44,225,117,225,115,0,0,225,23,225,10,225,8,227,78,224,253,224,
    250,224,243,224,199,224,36,224,33,224,25,224,24,226,87,224,17,224,14,224,2,223,230,223,207,223,204,220,104,0,0,88,95,8,138,8,186,8,185,8,184,8,183,8,182,8,181,3,72,2,76,0,131,177,131,
    84,149,3,32,196,144,22,32,186,134,17,32,192,134,7,145,6,32,172,130,18,32,174,130,3,181,2,34,124,0,136,132,56,32,138,132,5,32,0,134,13,134,12,156,6,73,77,5,144,34,8,81,1,72,1,73,1,35,
    1,36,4,6,4,7,4,8,3,116,4,9,4,10,4,11,2,53,4,15,4,16,2,92,1,245,1,246,4,19,4,20,4,17,4,18,2,55,2,56,3,120,2,57,2,58,3,121,4,104,4,105,4,100,4,102,2,23,4,107,4,101,4,103,4,109,3,98,2,
    27,3,144,3,145,3,177,140,98,12,108,41,0,71,0,159,1,5,1,101,1,217,1,237,2,20,2,62,2,111,2,142,2,171,2,188,2,215,2,236,3,44,3,82,3,142,3,225,4,26,4,97,4,185,4,213,5,61,5,150,5,195,5,
    243,6,17,6,43,6,73,6,150,7,34,7,87,7,162,7,222,8,14,8,51,8,83,8,152,8,188,8,206,8,243,9,31,9,53,9,110,9,156,9,222,10,17,10,96,10,155,10,240,11,11,11,55,11,97,11,181,11,224,12,3,12,
    38,12,65,12,86,12,112,12,140,12,158,12,192,13,17,13,94,13,149,13,225,14,33,14,87,14,218,15,18,15,56,15,111,15,165,15,183,16,6,16,55,16,114,16,195,17,16,17,65,17,142,17,196,17,248,18,
    32,18,115,18,160,18,223,19,2,19,74,19,91,19,162,19,219,19,219,20,2,20,67,20,140,21,1,21,63,21,88,21,202,21,248,22,105,22,182,22,238,23,5,23,13,23,127,23,145,23,197,23,240,24,37,24,
    111,24,145,24,208,24,250,25,3,25,49,25,87,25,135,25,187,26,25,26,118,26,248,27,68,27,86,27,104,27,122,27,140,27,159,27,171,27,231,27,243,28,5,28,23,28,41,28,60,28,78,28,96,28,114,28,
    133,28,199,28,217,28,235,28,253,29,15,29,33,29,52,29,103,29,215,29,233,29,251,30,13,30,32,30,50,30,107,30,202,30,220,30,238,31,0,31,17,31,35,31,55,31,185,31,197,31,215,31,233,31,251,
    32,13,32,30,32,47,32,65,32,84,32,183,32,201,32,219,32,237,32,255,33,17,33,35,33,90,33,194,33,212,33,230,33,248,34,10,34,28,34,109,34,127,34,145,34,163,34,181,34,199,34,211,34,223,34,
    241,35,3,35,21,35,39,35,57,35,75,35,93,35,111,35,129,35,148,35,156,35,243,36,5,36,23,36,41,36,59,36,77,36,95,36,107,36,209,36,227,36,245,37,7,37,25,37,43,37,62,37,80,37,99,37,111,37,
    128,37,146,37,165,37,221,38,34,38,53,38,71,38,90,38,107,38,126,38,144,38,155,38,166,38,185,38,197,38,209,38,227,38,245,39,1,39,12,39,64,39,82,39,100,39,111,39,123,39,142,39,160,39,
    172,39,184,39,225,40,6,40,24,40,42,40,54,40,66,40,84,40,102,40,114,40,182,40,254,41,16,41,34,41,52,41,70,41,89,41,108,41,197,42,53,42,71,42,89,42,101,42,113,42,131,42,148,42,166,42,
    184,42,202,42,219,42,231,42,243,43,5,43,22,43,33,43,44,43,62,43,74,43,120,43,185,43,203,43,221,43,239,44,1,44,19,44,37,44,57,44,77,44,96,44,115,44,196,44,208,44,226,44,244,45,6,45,
    23,45,42,45,60,45,78,45,96,45,114,45,132,45,149,45,187,46,5,46,111,46,251,47,13,47,31,47,49,47,67,47,78,47,89,47,134,47,181,47,200,47,239,48,11,48,56,48,98,48,153,48,210,48,242,49,
    52,49,64,49,73,49,86,49,99,49,112,49,124,49,137,49,149,49,170,49,178,49,186,49,209,50,3,50,11,50,19,50,27,50,104,50,112,50,120,50,162,50,170,50,178,50,216,50,224,50,250,51,2,51,57,
    51,65,51,73,51,156,51,164,51,226,52,47,52,66,52,85,52,102,52,119,52,136,52,154,52,173,53,13,53,105,53,159,53,250,54,74,54,151,54,203,55,16,55,53,55,61,55,147,55,155,55,199,56,46,56,
    54,56,108,56,178,56,247,57,55,57,103,57,150,57,233,58,60,58,128,58,217,58,236,58,254,59,15,59,32,59,50,59,69,59,135,59,153,59,223,59,231,59,239,60,2,60,10,60,100,60,165,60,214,60,232,
    60,250,61,28,61,36,61,87,61,95,61,103,61,162,61,170,61,226,62,53,62,104,62,122,62,159,62,218,62,226,62,234,62,242,62,250,63,2,63,10,63,18,63,82,63,90,63,98,63,134,63,181,63,216,64,
    3,64,55,64,112,64,159,64,229,65,58,65,116,65,124,65,223,66,41,66,64,66,120,66,128,66,184,67,12,67,59,67,76,67,114,67,164,67,218,67,255,68,7,68,34,68,42,68,50,68,79,68,87,68,168,68,
    176,68,213,69,6,69,43,69,88,69,143,69,200,69,251,70,58,70,136,70,190,70,208,71,40,71,58,71,122,71,130,71,138,71,157,71,165,71,246,72,57,72,65,72,83,72,100,72,134,72,162,72,190,72,208,
    72,226,72,244,73,6,73,25,73,44,73,62,73,80,73,97,73,114,73,122,73,148,73,178,73,209,73,218,73,248,74,42,74,93,74,102,74,142,74,207,74,239,74,255,75,131,75,150,75,181,75,211,75,241,
    75,253,76,18,76,65,76,111,76,193,77,29,77,129,77,232,78,62,78,156,78,226,78,234,79,47,79,176,80,86,80,243,81,105,81,200,81,208,81,234,82,20,82,37,82,72,82,168,82,221,82,234,83,34,83,
    46,83,58,83,100,83,138,83,170,83,179,83,210,84,4,84,68,84,95,84,184,153,1,12,88,85,164,85,248,86,10,86,18,86,143,86,205,87,47,87,65,87,83,87,95,87,116,87,162,87,248,88,72,88,137,88,
    210,89,1,89,19,89,37,89,55,89,73,89,164,89,251,90,59,90,123,90,208,91,32,91,96,91,158,91,240,92,62,92,142,92,221,93,65,93,162,94,62,94,218,94,226,94,234,95,53,95,122,95,185,95,246,
    96,9,96,28,96,149,96,161,97,13,97,112,98,42,98,215,98,233,98,251,99,56,99,114,99,165,100,90,100,234,101,63,101,146,101,210,102,19,102,100,102,206,102,247,103,32,103,106,103,175,103,
    240,104,49,104,61,104,73,104,121,104,169,104,219,105,13,105,60,105,117,105,160,105,203,105,248,106,37,106,78,106,119,106,198,107,15,107,150,108,17,108,29,108,41,108,77,108,112,108,
    120,108,165,108,217,109,19,109,73,109,127,109,172,109,219,110,19,110,72,110,143,110,213,111,5,111,13,111,108,111,198,112,43,112,137,112,145,112,163,112,181,113,12,113,83,113,151,113,
    211,114,10,114,66,114,112,114,158,114,213,115,13,115,83,115,147,115,155,115,173,115,190,115,209,115,227,115,235,115,243,116,5,116,22,116,97,116,105,116,124,116,142,116,161,116,180,
    116,199,116,217,117,29,117,96,117,114,117,132,117,151,117,169,117,188,117,206,117,214,117,222,117,241,118,3,118,22,118,41,118,59,118,77,118,96,118,114,118,133,118,152,118,171,118,189,
    118,221,118,253,119,16,119,35,119,47,119,113,119,186,120,3,120,60,120,119,120,168,120,176,121,2,121,101,121,194,122,30,122,105,122,178,123,7,123,84,123,146,123,211,124,26,124,93,124,
    145,124,200,125,28,125,36,125,123,125,201,125,213,125,225,125,243,126,5,126,25,126,45,126,65,126,85,126,105,126,125,126,145,126,165,126,187,126,209,126,229,126,249,127,13,127,33,127,
    53,127,73,127,93,127,113,127,135,127,157,127,169,127,181,127,199,127,217,127,235,127,252,128,15,128,34,128,54,128,74,128,94,128,114,128,134,128,154,128,176,128,198,128,216,128,234,
    128,246,129,2,129,14,129,26,129,44,129,62,129,81,129,100,129,120,129,140,129,160,129,180,129,200,129,220,129,242,130,8,130,26,130,44,130,62,130,80,130,98,130,116,130,134,130,152,130,
    164,130,176,130,188,130,200,130,218,130,236,130,254,131,16,131,34,131,52,131,70,131,88,131,106,131,124,131,136,131,148,131,160,131,172,131,190,131,208,131,226,131,243,132,5,132,71,
    132,137,132,223,133,53,133,110,133,166,133,238,134,57,134,96,134,136,134,148,134,160,134,172,134,184,134,206,134,228,135,37,135,45,135,67,135,128,135,173,135,252,136,74,136,96,136,
    118,136,140,136,162,136,184,136,206,136,228,136,250,137,66,137,74,137,150,137,244,138,85,138,162,138,240,139,24,139,36,139,48,139,60,139,76,139,92,139,177,140,3,140,81,140,100,140,
    119,140,131,140,143,140,161,140,173,140,217,140,255,141,17,141,35,141,53,141,72,141,90,141,108,141,126,141,138,141,149,141,167,141,179,141,197,141,215,141,227,141,240,141,248,142,11,
    142,19,142,38,142,46,142,54,142,72,142,109,142,117,142,125,142,142,142,160,142,180,143,43,143,55,143,66,143,153,143,247,144,9,144,27,144,45,144,127,144,213,144,221,145,15,145,63,145,
    80,145,156,145,203,146,18,146,92,146,144,146,217,147,17,147,88,147,128,147,192,147,210,148,1,148,61,148,154,148,176,148,236,149,73,149,108,149,170,150,12,150,50,150,112,150,215,151,
    19,151,105,151,230,152,26,152,90,152,152,152,201,153,28,153,110,153,128,153,146,153,168,153,190,153,202,153,214,153,226,153,238,153,250,154,6,154,18,154,29,154,40,154,52,154,64,154,
    76,154,88,154,100,154,112,154,124,154,136,154,148,154,160,154,172,154,184,154,196,154,208,154,220,154,238,154,247,155,0,155,9,155,18,155,27,155,36,155,45,155,54,155,63,155,72,155,81,
    155,141,155,150,155,177,155,186,155,195,155,233,156,15,156,57,156,102,156,147,156,245,157,45,157,107,157,132,157,177,157,243,158,4,158,26,158,51,158,77,158,141,158,166,158,184,158,
    209,158,235,159,5,159,14,159,23,159,32,159,41,159,50,159,59,159,68,159,77,159,86,159,95,159,159,159,198,160,1,160,82,160,137,160,207,161,38,161,64,161,167,161,254,162,7,162,16,162,
    25,162,34,162,43,162,52,162,61,162,70,162,79,162,88,162,141,162,178,162,237,163,62,163,117,163,187,164,21,164,48,164,153,164,239,165,61,165,69,165,77,165,85,165,93,165,101,165,109,
    165,117,165,125,165,133,165,141,165,150,165,159,165,168,165,177,165,186,165,195,165,204,165,213,165,222,165,231,165,248,166,24,166,33,166,66,166,75,166,105,166,130,166,139,166,148,
    166,156,166,225,167,86,167,94,167,102,167,132,167,143,167,155,0,131,0,40,46,2,46,0,3,0,1,4,9,130,12,34,172,7,14,134,11,36,1,0,32,6,238,134,11,36,2,0,14,6,224,134,11,36,3,0,52,6,172,
    134,11,32,4,138,35,36,5,0,26,6,146,134,23,36,6,0,30,6,116,134,11,36,7,0,164,5,208,134,11,36,8,0,42,5,166,134,11,36,9,0,40,5,126,134,11,36,10,0,66,5,60,134,11,36,11,0,62,4,254,134,11,
    36,12,0,60,4,194,134,11,36,13,1,34,3,160,134,11,36,14,0,52,3,108,134,11,36,16,0,18,3,90,134,11,36,17,0,12,3,78,134,11,36,25,0,26,3,52,133,11,33,1,0,130,23,32,40,134,11,36,1,0,10,3,
    30,134,11,32,3,130,11,32,20,134,11,32,4,137,227,37,1,5,0,16,3,4,134,23,36,6,0,8,2,252,134,11,36,7,0,18,2,234,134,11,36,9,0,30,2,204,134,11,36,10,0,34,2,170,134,11,36,11,0,36,2,134,
    134,11,36,12,0,28,2,106,134,11,36,13,0,38,2,68,134,11,32,14,130,11,135,131,36,15,0,42,1,244,134,23,36,16,0,44,1,200,134,11,36,17,0,36,1,164,134,11,36,18,0,46,1,118,134,11,36,19,0,56,
    1,62,134,11,36,20,0,60,1,2,134,11,36,21,0,62,0,196,134,11,36,22,0,54,0,142,134,11,35,23,0,64,0,65,19,6,37,1,24,0,18,0,60,134,23,36,25,0,26,0,34,134,11,36,26,0,12,0,22,134,11,32,27,
    65,67,9,33,1,28,130,23,32,10,134,23,50,29,0,10,0,0,0,82,0,111,0,109,0,97,0,110,0,73,0,116,130,7,36,108,0,105,0,99,130,95,34,111,0,114,132,23,36,108,0,83,0,101,130,9,34,105,0,67,130,
    41,34,110,0,100,130,13,34,110,0,115,130,5,32,100,146,17,34,79,0,112,132,31,32,83,130,73,131,37,137,93,32,45,146,45,34,69,0,120,130,115,32,114,130,43,32,66,130,91,32,108,130,91,173,
    63,181,53,135,207,181,61,32,82,130,223,36,103,0,117,0,108,130,181,33,114,0,173,175,38,76,0,105,0,103,0,104,130,243,155,55,65,21,45,163,197,171,241,169,223,137,205,66,25,17,33,32,0,
    145,197,147,37,135,189,147,27,143,181,147,35,141,173,147,33,137,165,145,145,135,125,143,105,137,51,44,87,0,105,0,100,0,116,0,104,0,87,0,101,65,189,34,32,77,130,37,32,100,130,39,34,
    117,0,109,66,27,8,33,32,0,67,13,7,32,104,130,73,38,116,0,112,0,58,0,47,130,1,36,115,0,99,0,114,130,43,32,112,130,21,36,115,0,46,0,115,130,11,32,108,130,7,32,111,130,21,32,103,130,31,
    38,79,0,70,0,76,0,84,130,123,32,105,130,27,32,32,130,13,34,111,0,110,130,43,131,79,34,111,0,102,130,9,34,119,0,97,130,43,34,101,0,32,130,57,32,115,130,5,67,187,5,32,101,130,37,67,159,
    5,34,32,0,117,130,11,32,100,130,147,32,114,130,29,32,116,130,73,131,43,34,83,0,73,130,87,32,32,138,159,33,70,0,135,89,131,249,32,99,67,225,8,32,44,130,53,32,86,132,61,32,115,130,97,
    131,35,34,32,0,49,130,161,131,3,32,32,138,153,141,119,135,141,34,97,0,118,130,159,34,105,0,108,130,5,32,98,130,5,131,123,32,119,130,71,131,135,32,32,130,19,131,209,34,65,0,81,130,99,
    36,97,0,116,0,58,130,7,32,104,65,31,6,32,115,65,33,44,65,85,13,33,119,0,131,1,34,46,0,109,132,211,32,111,130,81,34,121,0,112,130,195,34,46,0,99,130,17,36,109,0,47,0,115,130,19,34,117,
    0,100,132,209,149,59,32,103,130,41,34,111,0,103,132,179,137,55,32,103,130,69,32,116,130,63,32,110,130,29,32,116,130,3,34,47,0,68,130,17,32,115,130,71,34,103,0,110,130,9,32,100,130,
    193,34,98,0,121,130,5,32,77,142,123,32,32,130,109,32,101,130,119,32,105,130,83,32,110,130,31,32,116,130,45,32,97,130,161,33,46,0,145,41,139,83,34,32,0,84,134,41,145,39,32,73,130,63,
    32,97,130,79,32,105,130,123,32,103,130,83,32,73,130,7,34,99,0,46,66,117,18,65,143,9,32,32,69,109,6,32,100,130,85,38,109,0,97,0,114,0,107,130,57,34,111,0,102,130,5,32,71,138,243,32,
    32,130,27,32,110,130,179,32,32,132,99,131,209,32,98,130,49,32,32,130,47,32,101,132,113,32,115,130,71,32,101,132,13,32,100,130,57,32,105,130,121,34,32,0,99,130,33,32,114,130,23,33,97,
    0,133,15,34,106,0,117,130,33,32,105,130,245,32,100,130,17,32,99,130,25,32,105,130,97,32,110,130,15,137,163,67,23,7,33,45,0,67,55,11,66,113,15,32,51,130,213,36,48,0,48,0,51,130,1,32,
    46,130,7,131,9,36,59,0,71,0,79,130,1,34,71,0,59,69,131,16,141,77,68,33,13,69,119,7,67,147,9,32,32,140,123,32,67,130,163,34,112,0,121,132,185,69,169,5,34,32,0,50,130,115,131,3,34,32,
    0,84,67,65,6,147,69,32,80,130,51,37,111,0,106,0,101,0,131,235,34,32,0,65,130,255,32,116,130,47,32,111,130,23,36,115,0,32,0,40,130,11,37,116,0,116,0,112,0,66,221,7,32,103,67,11,6,35,
    117,0,98,0,66,101,11,32,111,130,131,34,103,0,108,130,73,32,102,130,9,32,110,130,51,34,115,0,47,132,151,67,217,5,34,97,0,110,130,67,8,59,41,0,1,0,0,0,10,2,194,4,96,0,5,68,70,76,84,2,
    136,99,121,114,108,2,28,103,114,101,107,1,236,104,101,98,114,1,188,108,97,116,110,0,32,1,112,0,7,65,80,80,72,1,66,67,65,84,32,1,20,73,130,11,55,0,230,77,65,72,32,0,184,77,79,76,32,
    0,138,78,65,86,32,0,92,82,79,77,32,130,137,36,0,255,255,0,20,130,91,44,2,0,6,0,7,0,8,0,9,0,17,0,19,130,17,53,21,0,22,0,23,0,24,0,25,0,26,0,27,0,28,0,29,0,30,0,31,0,145,45,32,16,172,
    45,32,15,172,45,32,13,162,45,32,1,136,183,32,12,162,45,137,229,32,11,162,45,137,91,32,10,158,45,36,19,0,0,0,5,136,137,65,63,25,32,4,130,39,34,0,255,255,130,77,34,0,0,4,174,47,32,3,
    162,47,46,112,0,2,77,75,68,32,0,62,83,82,66,32,0,16,65,175,8,137,201,32,18,158,201,32,20,130,161,137,45,32,14,154,45,139,203,137,49,153,251,44,32,97,97,108,116,1,150,99,99,109,112,
    1,142,132,5,32,126,132,5,32,116,132,5,32,106,132,5,62,94,100,110,111,109,1,88,102,114,97,99,1,78,108,105,103,97,1,72,108,110,117,109,1,66,108,111,99,108,1,60,132,5,32,54,132,5,32,48,
    132,5,32,42,132,5,32,36,132,5,32,30,132,5,32,24,132,5,32,18,132,5,32,12,130,58,35,114,1,6,111,130,6,40,1,0,111,114,100,110,0,250,112,130,11,34,0,244,115,130,137,37,0,236,115,115,48,
    49,132,5,34,50,0,230,130,11,34,51,0,224,130,5,47,52,0,218,115,117,98,115,0,212,115,117,112,115,0,206,116,131,47,44,200,122,101,114,111,0,194,0,0,0,1,0,35,132,5,32,32,132,5,65,43,5,
    32,19,132,11,32,38,132,5,32,37,132,5,32,36,130,5,35,2,0,36,0,133,13,32,31,132,19,32,28,132,5,32,33,132,5,32,21,132,5,32,17,132,5,32,10,132,5,32,8,132,5,32,9,132,5,32,18,132,5,32,11,
    132,5,32,15,132,5,32,12,132,5,32,16,132,5,32,30,132,5,32,34,130,5,32,3,67,159,6,36,0,0,1,0,22,130,15,36,4,0,2,0,5,132,3,34,0,0,3,132,7,32,7,130,21,133,9,32,6,130,9,32,6,136,31,131,
    39,32,0,130,13,135,7,32,0,130,65,40,39,9,108,7,74,6,238,6,212,130,1,40,34,5,164,5,20,4,226,4,192,130,1,40,158,4,92,4,60,4,28,3,250,130,1,50,230,3,230,3,124,3,46,3,22,3,8,2,244,3,22,
    2,172,2,158,130,1,52,96,2,62,2,38,1,224,1,154,1,84,1,16,0,252,0,214,0,100,0,80,130,81,131,85,32,8,130,7,34,6,2,125,130,5,34,1,1,65,136,19,8,52,2,0,54,0,24,3,152,3,153,3,154,3,155,3,
    156,3,157,3,158,3,159,3,160,3,161,3,163,3,164,3,165,3,168,3,169,3,170,3,171,3,172,3,173,3,174,3,192,3,166,3,167,3,162,130,61,8,48,24,0,44,0,45,0,142,0,143,0,144,0,145,0,234,0,236,0,
    238,0,240,0,242,0,243,0,245,1,89,1,102,1,118,1,160,1,161,1,162,2,182,2,197,3,50,3,52,3,150,138,113,44,16,0,5,3,186,3,187,3,188,3,189,3,179,130,23,42,5,0,74,0,223,0,225,0,227,0,229,
    136,37,36,1,0,6,4,59,130,13,41,1,0,19,0,4,0,0,0,1,0,131,191,32,54,132,7,50,5,0,38,0,30,0,24,0,18,0,12,3,140,0,2,0,79,3,139,130,5,34,76,3,138,130,5,38,73,3,142,0,3,0,73,130,19,32,141,
    132,7,32,76,132,67,32,73,136,87,8,44,2,0,46,0,20,4,79,4,80,4,81,4,82,4,83,4,84,4,85,4,86,4,87,4,88,4,68,4,69,4,70,4,71,4,72,4,73,4,74,4,75,4,76,4,77,130,79,32,2,130,127,40,28,0,0,4,
    48,4,57,0,10,141,69,67,143,19,147,89,131,69,132,63,32,0,130,99,32,77,143,69,130,25,48,49,4,50,4,51,4,52,4,53,4,54,4,55,4,56,4,57,158,139,34,79,4,88,138,69,36,1,0,6,255,236,130,99,32,
    1,132,93,32,0,138,163,38,14,0,4,0,108,0,124,132,3,32,1,130,11,46,36,0,50,0,68,0,82,0,6,0,0,0,2,0,36,130,67,32,3,130,45,32,186,130,3,66,231,5,34,0,0,29,130,11,32,2,130,37,33,82,0,131,
    25,32,160,144,25,32,36,130,63,32,1,65,141,8,32,62,130,119,133,75,32,38,134,75,36,18,0,1,0,46,132,31,34,0,0,27,132,147,34,38,4,47,130,15,131,77,32,28,138,77,32,26,132,27,34,58,4,67,
    130,27,36,1,0,1,2,22,66,163,12,32,4,130,13,33,1,0,131,75,32,0,130,9,32,8,130,3,34,20,4,19,66,25,12,32,39,131,63,65,153,5,136,23,131,209,46,15,4,100,4,102,4,104,3,119,0,123,0,116,0,
    117,90,167,12,34,105,2,23,130,43,37,15,0,11,0,12,0,69,75,21,34,32,0,81,138,77,8,46,50,0,22,4,101,4,103,4,106,4,89,4,90,4,91,4,92,4,93,4,94,4,95,4,96,4,97,4,98,4,107,3,193,3,194,3,195,
    3,196,3,197,3,198,3,199,3,200,130,57,32,22,156,91,38,75,0,78,0,79,0,80,130,99,36,83,0,86,0,87,136,105,36,1,0,6,1,245,130,13,34,1,1,202,65,187,13,39,3,145,3,144,3,177,3,178,130,21,40,
    4,1,95,1,115,1,126,1,147,67,11,10,32,18,65,27,6,38,4,1,0,0,2,0,121,130,13,34,1,0,47,148,31,32,1,136,31,34,79,0,6,65,189,8,32,10,130,21,33,38,0,131,69,32,2,130,55,34,79,0,1,132,59,133,
    73,131,13,34,0,0,13,140,19,32,47,132,157,32,14,142,163,38,146,3,180,3,147,3,181,132,41,38,252,0,253,1,6,1,7,141,33,92,85,7,132,197,38,31,1,32,3,109,3,110,138,33,50,22,0,8,3,148,3,182,
    3,149,3,183,3,150,3,184,3,151,3,185,132,215,46,198,0,199,0,218,0,219,0,240,0,241,1,51,1,52,138,215,62,114,0,9,0,104,0,94,0,84,0,74,0,64,0,54,0,44,0,34,0,24,0,1,0,4,3,255,0,2,4,33,132,
    9,32,254,136,9,32,253,136,9,32,252,136,9,32,250,136,9,32,249,136,9,32,248,136,9,32,238,136,9,32,234,134,9,50,9,3,201,3,205,3,218,3,220,3,221,3,224,3,225,3,226,3,227,138,143,38,110,
    0,2,0,60,0,10,130,17,50,40,0,30,0,20,0,10,3,126,0,4,4,8,4,5,4,1,3,127,134,9,34,2,3,128,132,9,32,6,130,19,32,129,134,9,32,2,138,49,32,122,136,49,32,123,136,49,32,124,136,49,32,125,136,
    49,38,1,0,2,1,133,1,145,138,125,32,146,130,121,56,136,0,126,0,116,0,96,0,86,0,76,0,66,0,56,0,36,0,26,0,1,0,4,1,52,130,191,32,16,130,157,36,12,0,6,3,112,131,11,39,3,114,0,3,4,16,1,76,
    131,29,33,0,241,132,29,130,39,33,0,219,136,9,32,199,135,9,33,1,51,138,59,32,111,132,59,32,113,138,59,32,240,136,49,32,218,136,9,32,198,134,9,32,10,130,127,36,40,0,44,0,50,130,137,34,
    68,0,72,130,147,34,82,0,88,65,241,10,32,94,130,141,43,175,3,176,4,114,4,111,4,112,4,115,0,68,65,5,32,42,130,199,36,3,0,0,0,1,130,193,34,2,0,20,130,61,32,1,130,13,32,4,130,15,33,4,2,
    102,75,6,32,0,133,31,32,34,130,19,32,18,132,79,32,3,130,219,42,2,3,116,3,116,0,0,4,1,4,13,130,19,32,1,130,77,42,76,0,77,0,241,1,238,1,240,3,53,134,49,32,8,130,25,8,98,102,0,48,2,20,
    2,14,2,8,1,248,1,234,1,220,1,206,1,192,1,178,1,164,1,150,1,136,1,122,1,116,1,110,1,104,1,98,1,90,1,84,1,78,1,72,1,66,1,60,1,54,1,48,1,42,1,36,1,30,1,24,1,18,1,12,1,6,1,0,0,250,0,244,
    0,238,0,232,0,226,0,220,0,214,0,208,0,202,0,196,0,190,0,184,0,178,0,172,0,166,130,141,36,10,0,11,0,12,130,183,34,14,0,14,130,13,38,19,0,28,0,3,0,32,130,1,34,13,0,81,130,1,8,34,14,0,
    240,0,241,0,15,1,11,1,11,0,17,4,48,4,57,0,18,4,68,4,77,0,28,4,79,4,88,0,38,0,2,4,57,130,13,36,2,4,56,4,76,130,11,34,55,4,75,130,5,34,54,4,74,130,5,34,53,4,73,130,5,34,52,4,72,130,5,
    34,51,4,71,130,5,34,50,4,70,130,5,34,49,4,69,130,5,34,48,4,68,132,59,32,88,130,5,34,56,4,87,132,59,32,86,132,59,32,85,132,59,32,84,132,59,32,83,132,59,32,82,132,59,32,81,132,59,32,
    80,132,59,34,79,0,2,130,129,32,77,130,5,32,27,131,119,33,0,26,131,119,33,0,25,131,119,33,0,24,131,119,33,0,23,131,119,33,0,22,131,119,33,0,21,131,119,33,0,20,131,119,33,0,19,130,119,
    44,3,3,130,3,131,3,132,0,2,3,184,4,114,130,5,52,150,3,161,0,2,2,23,3,197,0,2,4,105,4,107,0,6,3,121,4,47,130,215,50,67,4,88,4,98,0,6,2,58,4,46,4,56,4,66,4,87,4,97,130,13,42,57,4,45,
    4,55,4,65,4,86,4,96,130,41,42,120,4,44,4,54,4,64,4,85,4,95,130,27,42,56,4,43,4,53,4,63,4,84,4,94,130,13,56,55,4,42,4,52,4,62,4,83,4,93,0,6,0,117,4,41,4,51,4,61,4,82,4,92,130,13,42,
    116,4,40,4,50,4,60,4,81,4,91,130,13,58,123,4,39,4,49,4,59,4,80,4,90,0,7,3,119,4,38,4,48,4,58,4,78,4,79,4,89,130,147,34,104,4,106,130,5,34,102,4,103,130,5,34,100,4,101,66,151,10,44,
    154,0,74,2,22,0,108,3,152,3,153,0,124,130,7,38,186,3,193,3,175,3,176,69,229,5,33,0,124,69,229,5,73,37,8,68,179,7,72,197,6,73,53,6,73,51,5,69,19,6,53,1,72,1,73,3,151,3,185,3,168,3,145,
    3,169,3,144,3,170,3,177,3,178,73,75,6,36,191,4,111,4,112,73,81,7,39,4,115,1,35,1,36,3,162,130,171,32,39,130,203,32,41,130,235,40,43,4,44,4,45,4,46,4,47,130,161,52,74,0,18,0,36,0,44,
    0,45,0,50,0,68,0,74,0,75,0,76,0,77,70,77,6,33,82,0,70,77,5,73,137,7,69,55,7,73,79,7,73,153,5,37,242,0,243,0,245,0,69,159,6,53,1,31,1,32,1,51,1,52,1,89,1,95,1,102,1,115,1,118,1,126,
    1,147,73,175,5,37,1,202,1,238,1,240,73,181,8,8,120,53,3,109,3,110,3,150,4,58,4,59,4,60,4,61,4,62,4,63,4,64,4,65,4,66,4,67,0,0,64,74,153,152,151,150,135,134,133,132,131,130,129,128,
    127,126,125,124,123,122,121,120,119,118,117,116,115,114,113,112,111,110,109,108,107,106,105,104,103,102,101,100,99,98,97,96,95,94,93,92,91,90,89,88,87,86,85,84,83,81,80,79,78,77,76,
    75,74,73,72,71,70,40,31,16,10,9,44,1,177,11,10,67,35,67,101,10,45,44,0,177,10,11,130,10,40,11,45,44,1,176,6,67,176,7,132,20,58,176,79,43,32,176,64,81,88,33,75,82,88,69,68,27,33,33,
    89,27,35,33,176,64,176,4,37,69,131,3,35,97,100,138,99,135,24,32,89,130,66,130,53,130,59,130,66,42,75,83,35,75,81,90,88,32,69,138,96,132,53,35,45,44,75,84,140,13,133,31,32,56,131,81,
    132,27,134,9,39,176,2,67,84,88,176,70,43,130,23,130,107,33,45,44,133,15,32,71,132,15,130,96,133,14,32,72,142,30,32,73,132,15,130,30,43,35,32,176,0,80,138,138,100,177,0,3,37,130,70,
    35,64,27,177,1,132,8,35,5,67,139,89,130,201,41,89,35,176,98,43,35,33,35,88,101,130,43,43,177,8,0,12,33,84,96,67,45,44,177,12,135,9,34,1,32,71,130,97,36,32,184,16,0,98,130,3,36,99,87,
    35,184,1,134,9,39,90,88,176,32,96,102,89,72,130,45,35,0,2,37,176,132,2,37,83,184,0,53,35,120,133,11,51,96,176,32,99,32,32,176,6,37,35,98,80,88,138,33,176,1,96,35,27,134,15,34,82,88,
    35,130,15,44,97,27,138,33,35,33,32,89,89,184,255,193,28,131,45,33,35,33,130,75,46,2,0,66,177,35,1,136,81,177,64,1,136,83,90,88,130,116,46,176,32,136,84,88,178,2,1,2,67,96,66,89,177,
    36,130,27,37,88,184,32,0,176,64,132,21,32,2,131,21,131,20,131,34,32,32,131,12,36,0,75,1,75,82,130,47,32,8,132,47,37,27,184,64,0,176,128,132,42,32,4,132,16,132,15,32,99,130,197,132,
    19,133,36,40,185,64,0,1,0,99,184,2,0,132,19,32,16,132,39,33,177,38,131,104,130,25,32,2,130,25,32,4,133,25,32,64,132,25,130,19,32,4,130,19,32,8,133,19,32,128,133,45,32,40,134,45,32,
    8,130,25,32,16,131,25,37,185,0,2,1,0,176,132,28,32,89,132,0,51,177,0,2,67,84,88,64,10,5,64,8,64,9,64,12,2,13,2,27,177,130,211,130,183,130,17,45,186,1,0,0,9,1,0,179,12,1,13,1,27,177,
    130,83,130,193,130,22,39,184,1,128,177,9,64,27,184,132,73,133,18,34,186,1,128,130,41,131,19,32,128,130,93,134,38,33,2,0,131,38,32,178,137,72,130,113,132,232,33,136,85,137,193,37,85,
    90,88,179,12,0,130,95,132,5,130,33,32,66,131,0,39,45,44,69,177,2,78,43,35,66,214,9,46,81,88,176,2,37,69,177,1,78,43,96,89,27,35,75,130,14,42,3,37,69,32,100,138,99,176,64,83,88,131,
    43,35,96,27,33,89,130,2,8,33,89,68,45,44,32,176,0,80,32,88,35,101,27,35,89,177,20,20,138,112,69,177,16,16,67,75,138,67,81,90,88,176,64,27,130,83,63,89,35,177,97,6,38,96,43,138,88,176,
    5,67,139,89,35,88,101,89,35,16,58,45,44,176,3,37,73,99,35,70,96,130,34,35,35,176,4,37,130,2,32,73,130,18,39,99,86,32,96,176,98,96,43,130,10,35,32,16,70,138,130,30,34,32,99,97,131,45,
    59,0,22,177,2,3,37,177,1,4,37,1,62,0,62,177,1,2,6,12,176,10,35,101,66,176,11,35,66,136,25,34,63,0,63,133,25,32,6,131,25,37,7,35,66,176,1,22,65,117,5,54,69,35,69,32,24,105,138,99,35,
    98,32,32,176,64,80,88,103,27,102,89,97,176,32,130,209,47,35,97,176,4,35,66,27,177,4,0,66,33,33,89,24,1,130,210,36,69,177,0,78,43,130,219,58,75,81,177,64,79,43,80,91,88,32,69,177,1,
    78,43,32,138,138,68,32,177,64,4,38,97,99,97,131,15,37,68,33,27,35,33,138,134,26,37,35,68,68,89,45,44,136,49,39,69,32,138,176,64,97,99,96,130,33,33,69,89,132,44,33,45,44,130,130,40,
    138,69,35,97,32,100,176,64,81,130,238,36,32,176,0,83,35,130,10,33,90,90,131,101,47,84,90,88,138,12,100,35,100,35,83,88,177,64,64,138,97,130,246,44,27,32,99,89,27,138,89,99,177,2,78,
    43,96,130,142,130,154,35,0,45,44,5,68,157,9,68,156,6,39,2,45,44,176,2,37,99,102,130,4,38,184,32,0,98,96,35,98,133,16,34,176,32,96,144,19,32,103,143,36,32,102,130,37,139,19,33,35,74,
    131,110,130,172,32,74,131,181,130,7,36,138,74,35,69,100,130,33,131,3,42,97,100,176,3,67,82,88,33,32,100,89,66,50,5,35,0,80,88,101,130,246,32,35,151,36,36,1,78,43,35,176,134,36,35,32,
    176,3,37,132,92,36,138,16,59,45,44,133,13,130,31,132,13,130,26,130,2,35,138,176,103,43,140,14,32,104,136,14,32,70,130,30,37,70,96,176,4,37,46,130,3,132,2,33,38,32,131,88,39,33,176,
    106,27,176,108,89,43,131,30,132,34,42,97,176,128,98,32,138,32,16,35,58,35,131,4,33,45,44,130,21,32,71,131,3,40,96,176,5,37,71,176,128,99,97,130,203,37,176,6,37,73,99,35,130,16,32,74,
    130,16,43,32,88,98,27,33,89,176,4,38,70,96,138,66,113,7,130,54,33,4,38,136,105,34,176,110,43,140,82,38,35,32,176,1,84,88,33,130,72,44,177,2,78,43,176,128,80,32,96,89,32,96,96,130,21,
    39,81,88,33,33,27,32,176,5,130,7,43,32,102,97,176,64,35,97,177,0,3,37,80,133,222,34,80,90,88,131,245,34,97,138,83,130,59,33,0,89,130,116,36,27,176,7,84,88,130,40,42,101,35,33,27,33,
    33,176,0,89,89,89,65,125,5,130,90,130,125,45,74,176,0,83,88,176,0,27,138,138,35,138,176,1,130,162,33,37,70,130,44,130,94,39,38,176,6,38,73,176,5,38,131,2,39,112,43,35,97,101,176,32,
    96,130,25,35,176,32,97,101,130,185,32,2,130,38,32,138,65,29,5,131,77,36,27,69,35,33,89,130,34,35,2,37,16,59,132,215,36,32,184,2,0,98,131,4,8,38,99,138,35,97,32,176,93,96,43,176,5,37,
    17,138,18,138,32,57,138,88,185,0,93,16,0,176,4,38,99,86,96,43,35,33,32,16,32,70,32,131,68,34,35,97,27,130,13,35,138,32,16,73,131,13,32,89,130,72,133,40,33,9,37,131,40,130,63,132,2,
    39,38,176,109,43,177,93,7,37,132,81,132,17,32,37,131,5,33,111,43,133,44,32,8,132,85,130,153,35,82,88,176,80,131,116,131,28,34,7,37,176,130,2,43,5,37,176,113,43,176,2,23,56,176,0,82,
    131,252,44,1,82,90,88,176,4,37,176,6,37,73,176,3,131,68,40,73,96,32,176,64,82,88,33,27,130,32,36,88,32,176,2,84,132,30,130,33,132,63,32,73,131,58,32,27,131,48,130,17,134,54,130,76,
    32,89,131,0,32,33,131,0,33,45,44,133,130,32,11,133,175,132,52,130,92,131,2,34,12,37,176,130,2,41,9,37,176,8,37,176,110,43,176,4,130,131,133,30,32,7,131,206,66,4,9,33,109,43,131,180,
    33,6,37,131,51,130,146,130,174,32,5,131,152,131,158,130,106,32,32,134,77,132,200,32,96,134,12,34,4,37,101,132,211,33,2,37,130,211,131,192,38,83,88,33,176,64,97,35,131,3,40,27,184,255,
    192,80,88,176,64,96,130,13,35,96,35,89,89,131,126,130,129,33,4,38,132,49,132,90,32,138,131,10,65,46,5,130,118,32,8,65,10,20,134,105,133,150,35,11,37,176,11,130,37,131,194,133,16,133,
    5,34,10,37,176,130,2,34,7,37,176,130,179,131,27,65,35,5,32,5,65,120,13,65,61,6,130,149,131,117,35,89,89,89,33,134,0,34,45,44,176,130,48,34,3,37,135,131,233,33,3,37,66,89,6,39,176,101,
    27,176,104,89,43,100,65,31,5,32,6,133,6,35,73,32,32,99,130,37,47,32,99,81,177,0,3,37,84,91,88,33,33,35,33,7,27,130,20,42,2,37,32,99,97,32,176,83,43,138,99,66,23,5,32,135,132,49,33,
    38,74,68,29,5,42,176,4,38,32,1,70,35,0,70,176,5,135,8,39,0,22,0,176,0,35,72,1,131,4,39,0,32,176,1,35,72,176,2,130,14,136,9,39,35,178,2,0,1,8,35,56,131,6,55,9,35,56,177,2,1,7,176,1,
    22,89,45,44,35,16,13,12,138,99,35,138,99,96,100,71,168,5,38,80,88,176,0,56,27,60,130,26,36,176,6,37,176,9,132,2,45,7,38,176,118,43,35,176,0,84,88,5,27,4,89,131,139,36,6,38,176,119,
    43,132,155,32,38,134,5,33,118,43,136,31,39,119,43,45,44,176,7,37,176,65,73,5,32,8,131,60,32,138,136,28,34,5,37,176,130,78,130,60,130,93,130,69,133,5,34,118,43,8,130,79,145,53,33,138,
    8,165,108,35,8,37,176,11,132,2,32,9,131,108,34,176,4,38,130,2,32,8,131,152,153,106,32,3,65,160,5,32,74,65,174,5,33,74,2,133,198,32,74,68,39,5,130,21,49,38,99,138,138,99,97,45,44,177,
    93,14,37,96,43,176,12,38,17,130,26,32,18,130,211,32,57,130,218,32,57,134,219,35,9,37,176,124,130,247,32,80,131,125,130,131,32,10,134,14,33,84,88,130,35,130,19,65,182,5,33,37,11,130,
    45,39,16,176,9,37,193,176,2,37,130,2,32,11,130,28,37,16,176,6,37,193,27,133,37,130,40,37,184,255,255,176,118,43,131,138,32,4,130,45,65,58,5,33,119,43,130,54,66,231,5,134,30,134,61,
    66,157,5,34,119,43,89,130,31,32,70,131,3,32,96,130,37,32,70,131,3,67,73,6,33,11,176,67,165,5,32,12,69,245,14,33,4,37,130,97,32,11,65,211,6,32,9,141,28,32,35,136,76,36,97,176,32,99,
    35,136,81,131,13,40,177,1,12,37,84,88,4,27,5,130,113,41,38,32,16,176,3,37,58,176,6,38,130,2,130,201,130,15,37,138,58,177,1,7,38,134,32,32,5,130,16,130,227,35,58,138,138,11,70,82,6,
    49,35,176,1,84,88,185,0,0,64,0,27,184,64,0,176,0,89,138,143,16,40,176,125,43,45,44,138,138,8,13,149,25,32,8,143,47,32,13,132,48,65,229,6,32,13,136,7,131,69,67,23,6,55,10,67,176,11,
    67,138,99,35,98,97,45,44,176,9,43,176,6,37,46,176,5,37,125,197,130,8,130,7,72,78,5,33,80,88,71,14,8,33,5,37,66,5,5,71,36,12,38,24,176,8,37,176,7,37,131,50,35,10,37,176,111,131,68,132,
    59,65,55,7,37,102,27,176,104,89,43,132,18,130,62,141,21,34,84,88,125,130,81,42,16,176,3,37,197,176,2,37,16,176,1,130,7,35,5,38,33,176,130,3,35,27,176,6,38,133,109,34,176,8,38,130,86,
    36,89,177,0,2,67,131,47,35,2,37,176,130,132,78,46,130,43,32,32,105,97,176,4,67,1,35,97,176,96,96,131,11,34,32,97,32,131,45,41,8,38,138,176,2,23,56,138,138,97,130,20,32,97,131,10,76,
    229,5,50,24,45,44,75,82,177,1,2,67,83,90,88,35,16,32,1,60,0,60,130,24,35,89,45,44,35,66,113,5,34,83,88,32,130,114,49,88,60,27,57,89,176,1,96,184,255,233,28,89,33,33,33,45,44,130,29,
    32,71,131,3,37,84,138,32,32,16,17,130,26,40,138,32,18,176,1,97,176,133,43,130,27,33,4,37,133,27,32,35,132,19,33,35,32,130,183,134,37,130,9,130,34,33,138,138,133,39,131,189,50,12,2,
    138,75,83,176,4,38,75,81,90,88,10,56,27,10,33,33,89,77,127,8,34,152,43,88,154,30,33,32,176,130,63,50,176,1,35,184,0,104,35,120,33,177,0,2,67,184,0,94,35,121,33,130,22,37,35,176,32,
    32,92,88,130,168,41,176,0,184,0,77,28,89,138,138,32,130,1,38,35,184,16,0,99,86,88,133,5,131,28,36,1,184,0,48,28,130,110,35,89,176,128,98,137,47,35,29,28,89,35,140,16,32,12,130,64,130,
    219,36,184,255,171,28,35,130,250,137,119,32,129,136,119,32,119,130,119,131,129,32,138,135,120,34,184,0,103,148,118,130,233,34,176,1,91,66,120,5,130,5,132,234,37,184,0,56,176,0,35,133,
    136,33,4,38,134,123,42,138,92,138,90,35,33,35,33,184,0,30,130,72,135,143,131,15,34,14,28,89,132,69,35,97,184,255,147,131,144,36,4,205,0,193,0,130,0,35,2,20,0,0,132,3,62,40,0,142,3,
    86,0,135,5,43,0,50,4,147,0,117,6,195,0,92,5,223,0,104,1,217,0,135,2,115,0,82,130,3,36,62,4,101,0,83,130,27,42,99,2,32,0,78,2,147,0,77,2,38,130,51,34,7,0,19,130,19,32,96,130,3,32,169,
    134,7,32,89,130,7,32,42,130,3,32,124,130,3,32,105,130,3,32,84,130,3,32,95,130,3,32,94,131,47,130,3,32,65,130,11,32,99,130,3,32,108,130,3,58,99,3,139,0,25,7,44,0,114,5,45,0,0,5,56,0,
    196,5,14,0,124,5,214,0,196,4,116,130,3,32,50,130,15,32,208,130,15,36,244,0,196,2,87,130,3,36,70,255,95,5,1,130,23,40,67,0,196,7,72,0,196,6,37,130,3,36,66,0,124,4,222,133,7,33,5,6,130,
    27,50,100,0,102,4,119,0,24,5,227,0,182,4,225,0,0,7,121,0,23,130,235,45,5,4,154,0,0,4,152,0,71,2,160,0,160,3,130,179,130,7,56,51,4,147,0,72,3,115,255,252,2,99,0,82,4,139,0,92,4,240,
    0,171,3,231,0,109,130,7,48,109,4,141,0,109,2,203,0,33,4,99,0,25,4,255,0,171,130,243,38,157,2,32,255,139,4,101,132,11,36,171,7,133,0,171,131,23,33,4,217,132,47,32,171,130,51,34,108,
    3,92,130,63,38,219,0,101,3,2,0,36,130,27,38,161,4,35,0,0,6,93,130,127,50,76,0,32,4,37,0,2,3,202,0,74,3,10,0,51,4,102,1,227,130,7,32,71,130,127,32,99,65,123,7,130,11,32,175,130,3,32,
    72,130,3,32,119,130,3,32,25,131,35,51,4,14,0,118,4,177,1,46,6,168,0,100,2,226,0,63,4,52,0,80,132,51,34,147,0,77,131,19,39,4,0,255,250,3,109,0,108,132,19,38,216,0,51,2,216,0,43,131,
    203,47,5,8,0,171,5,61,0,120,2,38,0,142,1,190,0,12,130,23,36,80,3,5,0,65,130,63,52,78,6,26,0,61,6,91,0,44,6,92,0,47,3,139,0,54,5,45,0,0,147,3,41,7,29,255,255,5,14,0,124,4,116,65,139,
    6,134,7,39,2,87,255,224,2,87,0,184,130,7,32,195,130,7,36,4,5,214,0,55,65,135,7,35,6,66,0,124,139,3,35,4,147,0,132,131,15,35,5,227,0,182,139,3,47,4,154,0,0,4,222,0,196,5,40,0,171,4,
    139,0,92,147,3,41,7,2,0,92,3,231,0,109,4,141,141,3,39,2,32,255,231,2,32,0,140,130,7,32,170,130,3,34,230,4,214,130,255,32,255,130,71,32,217,130,39,143,3,35,147,0,99,4,131,19,35,255,
    0,161,4,139,3,36,37,0,2,4,240,130,51,130,7,131,251,131,127,144,7,130,251,131,127,32,5,151,7,34,214,0,196,130,67,32,109,131,255,33,4,250,130,123,32,116,130,15,131,171,158,7,39,5,208,
    0,124,4,99,0,25,152,7,32,244,130,71,46,255,255,173,5,244,0,0,4,255,0,16,2,87,255,175,130,231,32,141,130,7,32,246,130,7,32,219,130,7,32,224,130,7,40,204,2,87,0,100,2,32,0,60,130,7,34,
    185,4,157,130,51,38,64,0,157,2,70,255,95,130,27,34,139,5,1,130,15,32,101,130,223,131,3,34,67,0,163,130,39,36,140,4,67,0,196,130,7,32,134,134,7,32,171,132,7,32,75,130,35,34,67,0,20,
    130,51,34,239,6,37,130,51,34,255,0,171,143,7,35,5,153,0,3,136,19,32,66,130,187,35,217,0,109,6,142,7,47,7,127,0,124,7,165,0,107,5,6,0,196,3,92,0,171,134,7,32,131,134,7,40,134,4,100,
    0,102,3,219,0,101,152,7,39,119,0,24,3,2,0,36,4,142,7,39,5,227,0,182,4,255,0,161,167,7,47,7,121,0,23,6,93,0,23,4,154,0,0,4,37,0,2,132,7,39,152,0,71,3,202,0,74,4,142,7,33,2,180,130,251,
    50,147,0,192,5,46,255,255,4,139,0,92,7,29,255,255,7,2,0,92,135,219,135,171,35,3,118,0,82,132,3,32,15,130,7,40,59,0,82,1,141,0,82,2,112,130,3,32,2,130,15,32,160,130,3,38,161,0,82,4,
    158,1,254,130,3,50,11,5,55,0,10,2,38,0,142,5,41,255,255,6,169,255,255,3,18,130,7,36,162,255,255,5,227,130,7,39,158,255,246,2,218,255,209,5,68,231,6,41,4,62,0,196,4,203,0,42,4,116,130,
    7,130,155,33,5,244,68,195,5,37,2,87,0,196,5,1,130,19,34,235,0,0,68,227,7,35,4,114,0,71,131,151,33,5,220,130,23,32,222,130,3,32,150,130,207,34,119,0,24,131,223,43,6,128,0,102,4,205,
    0,5,6,137,0,111,130,35,32,73,130,63,32,4,132,247,51,246,0,109,3,250,0,86,4,255,0,171,2,218,0,166,4,243,0,158,4,130,19,33,5,21,130,247,38,52,0,7,4,215,0,107,131,31,35,3,226,0,109,131,
    35,35,4,200,0,108,132,39,32,101,130,31,36,113,255,249,5,8,130,7,35,112,0,2,3,130,39,45,4,217,0,109,5,98,0,25,4,216,0,153,3,228,130,47,32,250,130,91,32,234,130,15,130,83,33,5,227,130,
    15,42,115,255,232,6,51,0,158,6,95,0,114,130,227,32,235,132,107,130,51,131,7,131,19,32,4,130,227,33,6,3,130,175,130,243,39,5,48,0,124,4,100,0,102,130,167,32,196,130,3,42,4,2,70,255,
    95,7,156,0,5,7,193,130,255,130,35,131,251,130,123,32,22,131,235,41,5,45,0,0,4,243,0,196,5,56,130,243,131,59,34,162,0,12,132,75,42,239,0,3,4,203,0,86,6,53,0,198,131,3,132,51,32,182,
    130,67,32,72,130,43,65,71,6,65,43,7,33,5,14,130,111,34,119,0,24,131,83,65,43,7,33,5,252,130,39,40,158,0,153,8,89,0,196,8,115,130,11,36,146,0,11,6,231,130,7,32,20,130,3,36,28,0,65,8,
    109,130,7,60,32,0,32,4,139,0,92,4,208,0,112,4,180,0,171,3,139,0,171,4,199,0,37,4,141,0,109,6,40,130,127,38,3,0,70,5,65,0,171,131,3,33,4,87,130,27,36,185,0,10,6,23,130,15,32,31,130,
    11,130,251,33,5,6,130,7,32,240,70,79,5,49,3,238,0,43,4,37,0,2,5,236,0,107,4,76,0,32,5,34,130,27,40,244,0,146,7,71,0,171,7,91,130,51,36,139,0,29,6,79,130,19,32,196,130,47,36,253,0,67,
    6,187,130,11,34,130,0,26,131,107,35,4,255,0,16,132,123,38,2,0,109,3,219,0,101,70,135,6,42,230,2,32,255,139,6,208,0,10,7,29,130,43,130,35,132,131,131,99,32,19,130,15,36,83,0,196,3,160,
    130,91,32,121,67,15,5,67,23,7,67,31,16,35,0,0,82,8,134,3,39,3,71,255,252,1,115,0,27,130,3,36,26,2,10,0,65,130,7,35,27,2,251,0,131,3,40,26,3,123,0,65,4,21,0,128,130,3,56,119,3,2,0,153,
    6,97,0,142,9,150,0,92,1,253,0,84,3,119,0,84,2,138,0,80,130,3,32,78,130,31,44,142,1,7,254,129,3,63,0,112,4,147,0,79,130,3,36,73,6,101,0,165,130,7,60,56,6,127,0,117,4,36,0,94,8,43,0,
    182,6,33,0,32,6,66,0,73,4,244,0,102,6,183,0,60,130,3,32,43,130,3,32,78,130,3,48,85,4,163,0,91,4,203,0,42,5,233,0,191,5,10,0,66,130,59,44,99,4,100,0,37,5,164,0,116,3,32,0,7,132,15,34,
    147,0,99,136,7,46,169,0,103,4,158,0,189,4,0,1,126,0,0,255,119,130,7,36,116,2,216,0,19,130,3,32,68,130,3,32,58,130,3,37,50,4,0,0,0,8,130,3,135,7,45,2,170,0,0,2,0,0,0,1,86,0,0,4,147,
    130,11,32,38,130,11,32,84,130,16,32,205,130,3,134,2,35,8,0,0,84,131,3,61,2,32,255,139,1,115,0,26,5,41,0,19,4,168,0,0,7,13,0,29,7,72,0,196,7,133,0,171,5,45,130,63,34,139,0,92,130,79,
    8,54,110,6,86,0,124,5,6,0,109,6,104,0,182,5,127,0,161,0,0,252,250,4,116,0,196,6,53,0,198,4,141,0,109,5,65,0,171,7,102,0,50,6,117,0,39,5,101,0,15,5,30,0,15,7,110,130,31,32,20,130,71,
    32,128,130,71,36,163,0,5,7,132,130,15,32,97,130,15,40,242,0,28,5,13,0,14,8,2,130,15,52,205,0,171,4,194,0,58,4,3,0,28,6,137,0,111,6,51,0,158,6,69,69,135,5,39,5,50,0,0,4,67,0,0,135,7,
    37,9,224,0,124,8,214,130,131,32,145,130,139,42,66,0,109,8,74,0,124,7,77,0,119,136,119,54,48,0,123,3,252,0,109,4,222,0,108,7,233,0,43,7,166,0,43,6,122,0,196,130,175,44,171,4,240,0,45,
    4,203,0,23,4,231,0,196,130,11,42,171,4,69,0,46,3,147,0,13,5,90,130,15,130,147,39,7,95,0,3,6,140,0,3,130,35,32,86,130,139,34,70,5,123,130,23,32,153,130,171,32,4,130,7,32,87,130,7,48,
    1,0,35,4,101,0,14,5,147,0,10,5,12,0,29,6,46,130,87,36,100,0,171,6,134,130,7,36,231,0,171,8,153,130,203,32,245,130,15,32,54,130,155,34,32,0,109,71,87,7,40,4,119,0,22,3,236,0,43,4,69,
    179,5,32,0,135,7,61,5,49,0,5,4,139,0,32,7,3,0,23,5,228,0,43,5,204,0,153,5,36,0,146,5,158,0,153,4,236,132,7,34,197,4,255,130,79,39,240,0,42,5,100,0,34,6,134,7,33,2,87,130,103,32,239,
    130,171,36,40,0,3,5,167,130,155,32,163,130,155,32,243,130,75,32,239,130,151,32,232,130,139,32,10,130,51,32,56,130,7,32,107,130,23,131,75,35,244,0,146,7,130,163,35,6,77,0,171,131,59,
    72,7,15,70,23,7,32,4,71,215,6,39,6,8,0,131,4,141,0,102,136,7,134,107,65,23,7,47,4,175,0,69,4,28,0,36,6,53,0,198,5,65,0,171,136,7,71,51,7,65,187,6,65,195,8,46,28,0,65,3,253,0,67,5,8,
    0,22,4,37,0,2,144,7,134,163,37,4,70,0,196,3,139,130,79,32,231,130,223,33,79,0,65,159,9,32,44,65,39,5,8,53,4,205,0,4,4,76,0,32,4,241,0,118,4,240,0,109,7,56,0,117,7,52,0,107,7,55,0,63,
    6,138,0,74,5,36,0,63,4,122,0,79,7,236,0,3,6,250,0,10,8,38,0,196,7,94,130,79,46,34,0,124,5,38,0,109,5,196,0,22,5,93,0,43,130,71,38,105,3,250,0,86,5,225,130,87,34,236,0,10,65,15,15,142,
    15,32,23,142,15,32,92,175,15,65,87,7,154,7,36,56,4,141,0,47,151,31,39,2,87,0,143,2,32,0,119,130,7,32,181,130,7,32,157,72,155,15,142,15,32,64,142,15,72,203,9,67,191,7,158,7,72,139,15,
    67,239,7,159,7,72,147,12,72,155,7,131,7,42,250,0,109,0,0,252,95,0,0,251,120,133,7,33,252,94,130,7,32,100,138,3,40,92,1,165,0,39,1,220,0,26,73,31,7,143,195,35,4,141,0,102,130,47,44,
    250,7,159,0,1,4,164,1,88,2,216,0,41,130,3,32,44,130,3,36,37,2,218,255,225,130,3,32,226,130,3,32,209,130,3,36,210,4,243,0,158,139,3,47,5,211,0,195,6,37,0,196,5,194,0,183,0,0,0,87,134,
    3,130,79,43,0,102,4,164,0,179,5,150,0,33,4,235,133,3,35,7,181,0,33,131,3,47,5,191,0,183,5,59,255,233,5,44,0,192,4,67,0,196,132,63,54,45,0,0,4,116,0,196,2,87,0,100,5,227,0,182,2,199,
    0,82,3,95,0,56,130,7,32,59,130,3,36,82,2,199,255,255,130,7,32,58,130,7,32,225,130,7,32,46,130,3,32,32,133,23,32,0,130,27,35,0,82,5,12,136,47,132,15,33,3,208,132,47,131,11,130,51,132,
    35,32,58,134,79,52,82,2,32,0,171,2,32,255,139,5,21,0,171,4,115,255,232,4,240,0,109,130,19,34,73,4,255,76,79,6,32,141,132,15,32,60,130,15,32,161,131,27,139,3,39,3,126,255,240,4,215,
    0,107,132,159,47,63,0,112,2,219,0,112,1,98,0,112,4,228,0,112,3,130,15,33,3,53,130,19,54,129,0,66,1,245,0,24,4,211,0,113,4,75,0,87,3,92,0,54,4,31,0,44,131,111,33,2,34,130,123,34,71,
    0,75,131,11,8,47,5,14,0,162,2,5,0,158,4,7,0,36,3,240,0,69,3,255,0,58,5,16,0,166,5,27,0,100,2,20,0,93,3,113,0,111,4,213,0,106,4,205,0,63,4,242,0,84,130,7,42,106,4,12,0,4,4,150,0,76,
    4,218,130,171,32,14,130,83,39,207,0,87,5,54,0,48,5,131,7,138,3,132,123,131,127,148,131,37,255,188,2,71,255,202,133,127,33,255,190,140,127,130,123,136,119,135,115,146,111,131,199,43,
    0,0,252,4,0,0,253,120,0,0,254,152,130,11,32,10,130,7,32,203,130,3,36,183,0,0,255,141,130,7,32,214,130,7,32,31,130,7,32,247,130,3,130,31,33,251,198,130,15,32,88,130,47,32,34,130,7,32,
    49,130,3,32,82,130,11,32,60,130,3,32,63,130,59,32,81,130,3,32,89,130,19,32,188,130,43,32,158,130,3,32,208,133,3,33,255,182,130,19,32,32,134,3,32,69,130,7,32,71,130,3,131,95,32,187,
    130,7,32,30,130,3,131,7,130,43,32,255,131,7,131,43,32,80,66,187,6,131,7,78,207,5,71,203,9,32,44,71,207,7,130,7,50,37,4,181,0,114,3,161,0,45,4,110,0,77,4,139,0,85,4,157,130,11,46,139,
    0,120,4,160,0,114,4,31,0,19,4,185,0,116,130,11,32,100,168,79,54,185,0,108,3,78,0,34,4,96,0,84,4,84,0,60,4,151,0,46,4,131,0,117,130,87,40,111,4,76,0,42,4,147,0,95,130,11,32,92,130,7,
    36,96,4,109,0,69,130,3,32,133,130,3,32,93,130,3,32,71,130,3,32,14,130,3,32,98,130,3,32,85,130,3,32,67,130,3,32,77,130,3,32,68,167,123,39,2,147,0,77,1,212,0,79,134,3,32,60,130,7,36,
    60,2,186,0,72,139,3,43,3,7,0,19,6,141,0,174,6,95,0,102,66,255,7,35,4,0,1,111,130,11,39,61,0,162,0,0,0,2,0,132,0,35,255,156,0,50,132,8,142,4,43,4,116,0,0,1,2,1,3,0,3,0,4,110,245,10,
    47,10,0,11,0,12,0,13,0,14,0,15,0,16,0,17,0,110,105,27,8,34,32,0,33,0,34,0,35,0,36,0,37,0,38,0,39,0,40,0,41,0,42,0,43,0,44,0,45,0,46,0,47,0,48,0,49,130,123,8,44,51,0,52,0,53,0,54,0,
    55,0,56,0,57,0,58,0,59,0,60,0,61,0,62,0,63,0,64,0,65,0,66,0,67,0,68,0,69,0,70,0,71,0,72,0,73,98,163,14,14,70,81,0,82,0,83,0,84,0,85,0,86,0,87,0,88,0,89,0,90,0,91,0,92,0,93,0,94,0,95,
    0,96,0,97,1,4,0,163,0,132,0,133,0,189,0,150,0,232,0,134,0,142,0,139,0,157,0,169,0,164,1,5,0,138,1,6,0,131,0,147,1,7,1,8,0,141,1,9,0,136,0,195,0,222,1,10,0,158,0,170,0,245,0,244,0,246,
    0,162,0,173,0,201,0,199,0,174,0,98,0,99,0,144,0,100,0,203,0,101,0,200,0,202,0,207,0,204,0,205,0,206,0,233,0,102,0,211,0,208,0,209,0,175,0,103,0,240,0,145,0,214,0,212,0,213,0,104,0,
    235,0,237,0,137,0,106,0,105,0,107,0,109,0,108,0,110,0,160,0,111,0,113,0,112,0,114,0,115,0,117,0,116,0,118,0,119,0,234,0,120,0,122,0,121,0,123,0,125,0,124,0,184,0,161,0,127,0,126,0,
    128,0,129,0,236,0,238,0,186,1,11,1,12,1,13,1,14,1,15,1,16,0,253,0,254,1,17,1,18,1,19,1,20,0,255,1,0,1,21,1,22,1,23,1,1,1,24,1,25,1,26,1,27,1,28,1,29,1,30,1,31,1,32,1,33,1,34,1,35,0,
    248,0,249,1,36,1,37,1,38,1,39,1,40,1,41,1,42,1,43,1,44,1,45,1,46,1,47,1,48,1,49,1,50,1,51,0,250,1,52,1,53,1,54,1,55,1,56,1,57,1,58,1,59,1,60,1,61,1,62,1,63,1,64,1,65,1,66,0,226,0,227,
    1,67,1,68,1,69,1,70,1,71,1,72,1,73,1,74,1,75,1,76,1,77,1,78,1,79,1,80,1,81,0,176,0,177,1,82,1,83,1,84,1,85,1,86,1,87,1,88,1,89,1,90,1,91,0,251,0,252,0,228,0,229,1,92,1,93,1,94,1,95,
    1,96,1,97,1,98,1,99,1,100,1,101,1,102,1,103,1,104,1,105,1,106,1,107,1,108,1,109,1,110,1,111,1,112,1,113,0,187,1,114,1,115,1,116,1,117,0,230,0,231,1,118,0,166,1,119,1,120,1,121,1,122,
    1,123,1,124,1,125,1,126,0,216,0,225,0,218,0,219,0,220,0,221,0,224,0,217,0,223,1,127,1,128,1,129,1,130,1,131,1,132,1,133,1,134,1,135,1,136,1,137,1,138,1,139,1,140,1,141,1,142,1,143,
    1,144,1,145,1,146,1,147,1,148,1,149,1,150,1,151,1,152,1,153,1,154,1,155,1,156,1,157,1,158,1,159,1,160,1,161,1,162,1,163,1,164,1,165,1,166,1,167,1,168,1,169,1,170,1,171,1,172,1,173,
    1,174,1,175,1,176,1,177,1,178,1,179,1,180,1,181,1,182,1,183,0,155,1,184,1,185,1,186,1,187,1,188,1,189,1,190,1,191,1,192,1,193,1,194,1,195,1,196,1,197,1,198,1,199,1,200,1,201,1,202,
    1,203,1,204,1,205,1,206,1,207,1,208,1,209,1,210,1,211,1,212,1,213,1,214,1,215,1,216,1,217,1,218,1,219,1,220,1,221,1,222,1,223,1,224,1,225,1,226,1,227,1,228,1,229,1,230,1,231,1,232,
    1,233,1,234,1,235,1,236,1,237,1,238,1,239,1,240,1,241,1,242,1,243,1,244,1,245,1,246,1,247,1,248,1,249,1,250,1,251,1,252,1,253,1,254,1,255,2,0,2,1,2,2,2,3,2,4,2,5,2,6,2,7,2,8,2,9,2,
    10,2,11,2,12,2,13,2,14,2,15,2,16,2,17,2,18,2,19,2,20,2,21,2,22,2,23,2,24,2,25,2,26,2,27,2,28,2,29,2,30,2,31,2,32,2,33,2,34,2,35,2,36,2,37,2,38,2,39,2,40,2,41,2,42,2,43,0,178,0,179,
    2,44,2,45,0,182,0,183,0,196,2,46,0,180,0,181,0,197,0,130,0,194,0,135,0,171,0,198,2,47,2,48,0,190,0,191,2,49,0,188,2,50,0,247,2,51,2,52,2,53,2,54,2,55,2,56,0,140,2,57,2,58,2,59,2,60,
    2,61,2,62,0,152,2,63,0,154,0,153,0,239,0,165,0,146,0,156,0,167,0,143,0,148,0,149,0,185,2,64,2,65,2,66,2,67,2,68,2,69,2,70,2,71,2,72,2,73,2,74,2,75,2,76,2,77,2,78,2,79,2,80,2,81,2,82,
    2,83,2,84,2,85,2,86,2,87,2,88,2,89,2,90,2,91,2,92,2,93,2,94,2,95,2,96,2,97,2,98,2,99,2,100,2,101,2,102,2,103,2,104,2,105,2,106,2,107,2,108,2,109,2,110,2,111,2,112,2,113,2,114,2,115,
    2,116,2,117,2,118,2,119,2,120,2,121,2,122,2,123,2,124,2,125,2,126,2,127,2,128,2,129,2,130,2,131,2,132,2,133,2,134,2,135,2,136,2,137,2,138,2,139,2,140,2,141,2,142,2,143,2,144,2,145,
    2,146,2,147,2,148,2,149,2,150,2,151,2,152,2,153,2,154,2,155,2,156,2,157,2,158,2,159,2,160,2,161,2,162,2,163,2,164,2,165,2,166,2,167,2,168,2,169,2,170,2,171,2,172,2,173,2,174,2,175,
    2,176,2,177,2,178,2,179,2,180,2,181,2,182,2,183,2,184,2,185,2,186,2,187,2,188,2,189,2,190,2,191,2,192,2,193,2,194,2,195,2,196,2,197,2,198,2,199,2,200,2,201,2,202,2,203,2,204,2,205,
    2,206,2,207,2,208,2,209,2,210,2,211,2,212,2,213,2,214,2,215,2,216,2,217,2,218,2,219,2,220,2,221,2,222,2,223,2,224,2,225,2,226,2,227,2,228,2,229,2,230,2,231,2,232,2,233,2,234,2,235,
    2,236,2,237,2,238,2,239,2,240,2,241,2,242,2,243,2,244,2,245,2,246,2,247,2,248,2,249,2,250,2,251,2,252,2,253,2,254,2,255,3,0,3,1,3,2,3,3,3,4,3,5,3,6,3,7,3,8,3,9,3,10,3,11,3,12,3,13,
    3,14,3,15,3,16,3,17,3,18,3,19,3,20,3,21,3,22,3,23,3,24,3,25,3,26,3,27,3,28,3,29,3,30,3,31,3,32,3,33,3,34,3,35,3,36,3,37,3,38,3,39,3,40,3,41,3,42,3,43,3,44,3,45,3,46,3,47,3,48,3,49,
    3,50,3,51,3,52,3,53,3,54,3,55,3,56,3,57,3,58,3,59,3,60,3,61,3,62,3,63,3,64,3,65,3,66,3,67,3,68,3,69,3,70,3,71,3,72,3,73,3,74,3,75,3,76,3,77,3,78,3,79,3,80,3,81,3,82,3,83,3,84,3,85,
    3,86,3,87,3,88,3,89,3,90,3,91,3,92,3,93,3,94,3,95,3,96,3,97,3,98,3,99,3,100,3,101,3,102,3,103,3,104,3,105,3,106,3,107,3,108,3,109,3,110,3,111,3,112,3,113,3,114,3,115,3,116,3,117,3,
    118,3,119,3,120,3,121,3,122,3,123,3,124,3,125,3,126,3,127,3,128,3,129,106,105,5,8,41,3,133,3,134,3,135,3,136,3,137,3,138,3,139,3,140,3,141,3,142,3,143,3,144,3,145,3,146,3,147,3,148,
    3,149,3,150,3,151,0,192,0,193,114,213,20,33,162,3,105,163,5,34,166,3,167,114,219,14,54,175,3,176,3,177,3,178,3,179,3,180,3,181,3,182,3,183,3,184,3,185,0,215,114,169,8,36,190,3,191,
    3,192,111,245,15,8,45,3,201,3,202,3,203,3,204,3,205,3,206,3,207,3,208,3,209,3,210,3,211,3,212,3,213,3,214,3,215,3,216,3,217,3,218,3,219,3,220,3,221,3,222,3,223,110,61,7,8,97,3,228,
    3,229,3,230,3,231,3,232,3,233,3,234,3,235,3,236,3,237,3,238,3,239,3,240,3,241,3,242,3,243,3,244,3,245,3,246,3,247,3,248,3,249,3,250,3,251,3,252,3,253,3,254,3,255,4,0,4,1,4,2,4,3,4,
    4,4,5,4,6,4,7,4,8,4,9,4,10,4,11,4,12,4,13,4,14,4,15,4,16,4,17,4,18,4,19,4,20,24,83,119,18,46,30,4,31,4,32,4,33,4,34,4,35,4,36,4,37,106,107,19,114,139,20,105,251,18,114,159,19,33,4,
    78,114,251,19,113,59,20,8,68,99,4,100,4,101,4,102,4,103,4,104,4,105,4,106,4,107,4,108,4,109,4,110,4,111,4,112,4,113,4,114,4,115,4,116,4,117,4,118,4,119,4,120,4,121,4,122,4,123,4,124,
    4,125,4,78,85,76,76,2,67,82,7,117,110,105,48,48,65,48,134,7,42,68,9,111,118,101,114,115,99,111,114,101,133,17,33,66,50,134,7,32,51,134,7,32,53,134,7,42,57,7,65,109,97,99,114,111,110,
    7,97,133,7,40,6,65,98,114,101,118,101,6,97,132,6,41,7,65,111,103,111,110,101,107,7,97,133,7,45,11,67,99,105,114,99,117,109,102,108,101,120,11,99,137,11,38,4,67,100,111,116,4,99,130,
    4,35,6,68,99,97,130,78,33,6,100,133,6,39,68,99,114,111,97,116,7,69,133,92,33,7,101,133,7,33,6,69,132,93,33,6,101,132,6,33,10,69,130,55,39,97,99,99,101,110,116,10,101,136,10,33,7,69,
    133,114,33,7,101,133,7,33,6,69,133,81,32,101,132,6,33,11,71,137,124,33,11,103,137,11,33,4,71,130,64,33,4,103,130,4,132,224,33,49,50,133,248,36,49,50,51,11,72,137,37,33,11,104,137,11,
    38,4,72,98,97,114,4,104,130,4,40,6,73,116,105,108,100,101,6,105,132,6,33,7,73,133,171,33,7,105,133,7,33,6,73,132,172,33,6,105,132,6,33,7,73,133,149,33,7,105,133,7,39,2,73,74,2,105,
    106,11,74,137,87,33,11,106,137,11,133,139,33,51,54,134,7,54,55,12,107,103,114,101,101,110,108,97,110,100,105,99,6,76,97,99,117,116,101,6,108,132,6,134,34,32,66,134,7,34,67,6,76,132,
    239,33,6,108,132,6,33,4,76,130,217,33,4,108,130,4,33,6,78,133,53,32,110,138,53,32,52,65,213,5,36,49,52,54,6,78,132,46,33,6,110,132,6,53,11,110,97,112,111,115,116,114,111,112,104,101,
    3,69,110,103,3,101,110,103,7,79,133,214,33,7,111,133,7,33,6,79,132,215,33,6,111,132,6,47,13,79,104,117,110,103,97,114,117,109,108,97,117,116,13,111,139,13,33,6,82,132,114,33,6,114,
    138,121,32,53,134,218,35,53,55,6,82,132,114,33,6,114,133,6,32,83,132,36,33,6,115,132,6,33,11,83,65,18,9,33,11,115,65,30,14,34,50,49,65,132,243,36,50,49,66,6,84,133,60,32,116,132,6,
    38,4,84,98,97,114,4,116,130,4,40,6,85,116,105,108,100,101,6,117,132,6,33,7,85,133,185,33,7,117,133,7,33,6,85,132,186,33,6,117,132,6,39,5,85,114,105,110,103,5,117,131,5,33,13,85,140,
    205,32,117,139,13,33,7,85,65,192,5,33,7,117,133,7,33,11,87,137,151,33,11,119,138,11,32,89,138,11,32,121,137,11,33,6,90,132,218,33,6,122,132,6,33,10,90,66,191,8,33,10,122,136,10,39,
    5,108,111,110,103,115,10,65,131,139,133,38,32,97,136,10,34,7,65,69,132,18,34,7,97,101,132,7,38,11,79,115,108,97,115,104,133,11,32,111,137,11,65,27,6,32,56,134,7,47,57,5,116,111,110,
    111,115,13,100,105,101,114,101,115,105,115,132,13,37,10,65,108,112,104,97,132,10,46,9,97,110,111,116,101,108,101,105,97,12,69,112,115,105,130,128,132,22,34,8,69,116,134,31,33,73,111,
    134,9,39,12,79,109,105,99,114,111,110,132,31,33,12,85,138,44,36,10,79,109,101,103,133,46,33,17,105,130,46,140,107,32,5,132,107,42,4,66,101,116,97,5,71,97,109,109,97,132,152,35,51,57,
    52,7,134,111,33,4,90,130,26,32,3,130,111,34,5,84,104,130,9,32,4,131,112,54,5,75,97,112,112,97,6,76,97,109,98,100,97,2,77,117,2,78,117,2,88,105,7,134,129,49,2,80,105,3,82,104,111,5,
    83,105,103,109,97,3,84,97,117,7,134,141,37,3,80,104,105,3,67,130,3,34,80,115,105,133,102,34,65,57,12,131,79,135,145,32,15,134,40,135,15,33,10,97,65,8,8,33,12,101,138,209,32,8,130,136,
    132,231,32,9,131,207,132,9,33,20,117,133,31,141,220,132,63,33,4,98,130,46,33,5,103,131,220,38,5,100,101,108,116,97,7,134,75,33,4,122,130,24,32,3,130,3,33,5,116,132,218,131,76,33,5,
    107,132,218,32,108,132,218,133,164,41,66,67,2,110,117,2,120,105,7,111,65,97,5,35,3,114,104,111,133,25,35,67,50,5,115,132,228,32,116,130,228,134,128,33,3,112,130,224,32,99,130,3,130,
    143,38,5,111,109,101,103,97,12,65,116,11,32,15,142,167,32,12,134,84,132,196,32,12,134,28,132,12,32,10,132,60,132,10,132,109,34,52,48,49,134,7,69,11,5,34,52,48,51,134,15,32,52,134,7,
    68,54,5,33,52,48,67,196,5,34,52,48,55,134,23,66,108,5,34,52,48,57,134,15,67,160,5,33,52,48,68,156,5,34,52,48,67,134,23,32,69,134,7,32,70,133,7,33,49,48,134,7,32,49,134,7,134,119,32,
    49,134,119,32,49,134,119,32,49,134,119,32,49,134,119,32,49,134,119,32,49,134,119,32,49,134,119,32,49,134,119,32,49,134,119,32,49,134,119,33,49,68,134,95,134,127,32,49,134,127,33,50,
    48,133,23,33,50,49,134,7,134,127,32,50,134,127,32,50,134,127,32,50,134,127,32,50,134,127,32,50,134,127,32,50,134,127,32,50,134,127,32,50,134,127,32,50,134,127,32,50,134,127,32,50,134,
    127,32,50,134,127,32,50,134,127,33,51,48,133,119,33,51,49,134,7,134,127,32,51,134,127,32,51,134,127,32,51,134,127,70,23,6,33,52,51,134,127,32,51,134,127,32,51,134,127,32,51,134,127,
    32,51,134,127,32,51,134,127,32,51,134,127,32,51,134,127,32,51,134,127,33,52,48,133,119,33,52,49,134,7,134,127,32,52,134,127,32,52,134,127,70,46,6,33,52,52,134,255,32,52,134,127,32,
    52,134,127,32,52,134,127,32,52,134,127,32,52,134,127,32,52,134,127,32,52,134,127,32,52,134,127,32,52,134,127,33,53,49,133,119,32,53,134,119,32,53,134,119,32,53,134,119,32,53,134,247,
    70,52,6,33,52,53,134,119,32,53,134,119,32,53,134,119,32,53,134,119,32,53,134,119,32,53,134,119,32,53,134,111,32,53,134,111,33,57,48,133,111,42,57,49,6,87,103,114,97,118,101,6,119,133,
    6,36,87,97,99,117,116,130,13,132,6,33,9,87,67,226,7,33,9,119,135,9,33,6,89,133,40,32,121,132,6,131,69,39,50,48,49,53,13,117,110,100,73,61,6,43,100,98,108,13,113,117,111,116,101,114,
    101,118,130,18,37,101,100,6,109,105,110,131,83,44,115,101,99,111,110,100,9,101,120,99,108,97,109,130,37,133,59,55,55,70,9,97,102,105,105,48,56,57,52,49,6,112,101,115,101,116,97,4,69,
    117,114,111,132,29,33,49,48,132,247,33,50,49,67,7,5,130,7,32,54,133,23,58,50,54,9,101,115,116,105,109,97,116,101,100,9,111,110,101,101,105,103,104,116,104,12,116,104,114,101,134,11,
    36,115,11,102,105,118,135,11,33,12,115,130,139,32,110,133,36,32,115,132,65,44,50,48,54,13,99,121,114,105,108,108,105,99,98,131,169,42,16,99,97,114,111,110,99,111,109,109,97,73,87,5,
    131,38,36,48,51,50,54,17,138,19,34,114,111,116,130,113,132,64,34,48,55,52,134,7,133,162,34,48,55,55,134,15,32,56,133,7,33,48,48,134,7,32,49,134,7,32,50,134,7,32,51,134,7,134,63,134,
    226,32,48,68,74,5,33,50,48,68,74,5,130,7,135,71,32,57,134,47,32,65,134,7,32,66,131,7,35,70,69,70,70,132,7,34,70,70,67,134,7,66,90,5,34,49,70,48,132,185,33,50,66,132,23,35,48,51,68,
    49,132,15,33,51,68,132,127,130,15,32,54,131,15,33,49,69,67,2,5,130,7,132,71,33,49,69,133,175,130,7,133,47,42,50,70,51,5,79,104,111,114,110,5,111,132,5,32,85,132,5,32,117,131,5,36,4,
    104,111,111,107,132,84,32,52,133,52,34,48,52,48,133,124,33,52,53,133,124,33,52,53,134,15,32,54,134,15,32,54,133,84,33,52,54,133,132,34,52,54,51,133,63,33,54,52,134,7,32,53,134,7,132,
    156,35,48,52,54,55,134,15,32,56,134,7,32,57,134,7,32,65,134,7,32,66,134,7,133,228,33,52,54,135,111,66,231,6,32,54,132,212,35,48,52,55,48,133,39,33,55,49,134,7,134,127,32,55,134,127,
    65,196,5,130,39,134,127,32,55,134,127,65,204,5,130,23,134,127,32,55,134,127,32,55,134,127,32,55,134,127,32,55,134,127,32,55,134,127,32,55,134,127,32,55,134,127,33,56,48,133,119,33,
    56,49,134,7,134,127,32,56,134,87,32,56,134,87,32,56,134,87,32,56,134,87,32,56,134,87,32,56,134,87,32,56,134,87,32,56,134,87,32,57,134,71,32,57,134,199,32,57,65,71,6,32,57,134,199,32,
    57,134,199,32,57,65,71,6,32,57,134,111,32,57,134,111,32,57,134,111,32,57,134,111,32,57,134,111,32,57,134,111,32,57,134,111,32,57,134,111,77,38,6,34,52,65,49,133,199,32,65,134,127,32,
    65,134,127,32,65,134,127,32,65,134,127,32,65,134,127,32,65,134,127,32,65,134,127,32,65,134,127,32,65,134,127,32,65,134,127,32,65,134,127,32,65,134,127,32,65,134,127,32,65,134,127,33,
    66,48,133,119,33,66,49,134,7,134,127,32,66,134,127,32,66,134,127,32,66,134,127,32,66,134,127,32,66,134,127,32,66,134,127,32,66,134,127,32,66,134,127,32,66,134,127,67,44,6,33,52,66,
    134,127,32,66,134,127,32,66,134,127,33,67,48,133,119,33,67,49,134,7,134,127,32,67,134,127,32,67,134,127,32,67,134,127,32,67,134,127,32,67,134,127,32,67,134,127,32,67,134,127,32,67,
    134,127,32,67,134,127,32,67,134,255,32,67,134,127,32,67,134,127,32,67,134,127,33,68,48,133,119,67,204,6,32,52,67,204,6,33,52,68,134,127,32,68,134,127,32,68,134,127,32,68,134,127,32,
    68,134,127,32,68,134,127,32,68,134,127,32,68,134,127,32,68,134,127,32,68,134,127,32,68,134,127,32,68,134,127,32,68,134,127,32,69,134,127,33,69,49,133,135,32,69,134,255,32,69,134,127,
    32,69,134,127,32,69,134,127,32,69,134,127,32,69,134,127,32,69,134,127,32,69,134,127,32,69,134,127,32,69,134,127,32,69,134,127,32,69,134,127,32,69,134,127,32,69,134,127,68,212,6,34,
    52,70,49,133,127,32,70,134,127,32,70,134,127,32,70,134,127,32,70,134,127,32,70,134,127,32,70,134,127,32,70,134,127,32,70,134,127,32,70,134,127,32,70,134,127,69,68,5,34,48,52,70,134,
    127,32,70,134,127,69,100,5,33,48,53,68,231,6,34,53,48,49,132,127,33,53,48,133,127,32,53,74,23,6,33,53,48,133,127,32,53,69,204,5,130,47,133,127,32,53,69,204,5,130,15,133,127,33,53,48,
    133,127,33,53,48,133,127,33,53,48,133,127,33,53,48,133,255,33,53,48,133,127,33,53,48,133,127,33,53,48,133,255,34,53,49,48,133,119,32,49,134,127,32,49,134,127,71,23,5,33,49,69,67,159,
    5,130,7,132,31,130,7,132,31,130,7,32,51,69,236,5,67,159,5,130,15,32,53,134,15,132,159,130,15,32,55,134,15,132,159,130,15,132,159,130,7,132,159,130,7,132,159,130,7,132,159,130,7,132,
    159,130,7,132,159,130,7,132,159,33,49,69,67,159,5,130,7,32,49,133,79,81,60,5,130,15,134,127,67,159,5,130,15,134,127,67,159,5,130,15,134,127,67,159,5,130,15,134,127,67,159,5,130,15,
    134,127,67,159,5,130,15,134,127,67,159,5,130,15,134,127,67,159,5,33,49,69,67,159,5,130,7,134,255,67,159,5,130,15,32,52,133,151,67,159,5,130,15,134,255,67,159,5,130,15,134,255,67,159,
    5,130,15,134,255,67,159,5,130,15,134,255,67,159,5,130,15,134,255,67,159,5,33,49,69,67,159,5,130,7,32,49,133,103,67,159,5,130,15,134,255,67,159,5,130,15,134,255,67,159,5,130,15,134,
    255,67,159,5,130,15,134,255,67,159,5,130,15,134,255,67,159,5,130,15,134,255,67,159,5,130,15,134,255,67,159,5,33,49,69,67,159,5,130,7,134,255,67,159,5,130,15,134,255,67,159,5,130,15,
    134,255,67,159,5,130,15,134,255,67,159,5,130,15,134,255,67,159,5,130,15,134,255,67,159,5,130,15,134,255,67,159,5,33,49,69,67,159,5,130,7,32,49,133,255,67,143,5,130,15,134,239,67,143,
    5,130,15,134,239,67,143,5,130,15,132,239,36,50,48,65,66,19,80,31,9,40,97,99,117,116,101,99,111,109,98,138,19,35,103,114,97,118,132,19,32,18,137,39,35,104,111,111,107,142,38,35,116,
    105,108,100,132,38,37,14,98,114,101,118,101,136,73,133,14,136,68,32,13,132,29,135,63,133,28,136,58,32,16,74,23,7,131,31,36,108,101,102,116,17,135,16,34,98,105,103,131,19,33,85,67,131,
    228,33,48,49,72,204,6,32,49,72,204,6,32,49,68,212,6,32,49,65,52,5,33,48,49,68,212,6,32,49,65,52,5,36,48,50,53,57,13,131,62,34,97,98,111,133,202,131,69,33,49,70,76,18,5,33,49,70,65,
    210,5,33,50,48,72,178,5,130,7,32,54,73,239,5,45,55,57,19,117,110,105,48,51,66,57,48,51,48,56,130,3,32,52,130,3,32,48,146,19,32,49,142,19,32,54,146,39,131,19,134,39,33,67,53,145,79,
    140,19,143,39,138,79,136,39,132,79,40,8,69,110,103,46,97,108,116,49,135,8,32,50,135,8,33,51,15,132,186,33,48,49,130,106,131,42,32,56,134,15,32,48,142,15,131,31,32,52,130,19,139,31,
    133,15,65,122,7,43,95,111,116,109,97,114,107,3,102,95,102,5,130,3,33,95,105,132,5,33,108,7,130,95,33,49,69,72,173,5,33,65,55,72,13,5,130,7,32,52,132,103,42,49,51,66,46,108,111,99,108,
    77,65,72,133,15,33,52,53,136,15,85,164,6,132,15,35,78,65,86,15,85,57,6,136,15,84,171,6,136,15,82,242,6,135,15,35,6,73,46,115,130,241,33,6,74,132,6,38,11,73,103,114,97,118,101,134,11,
    35,97,99,117,116,133,11,33,16,73,66,172,9,132,28,33,14,73,77,70,7,132,14,32,11,85,61,5,132,11,32,12,85,59,6,132,12,32,11,85,56,5,132,11,32,12,135,138,131,112,32,20,134,12,32,95,131,
    215,130,183,132,33,33,15,73,83,54,8,132,15,33,7,73,133,150,32,16,85,90,10,132,24,39,12,117,110,105,49,69,67,56,139,12,32,65,132,12,32,14,82,219,8,132,14,32,9,131,14,132,9,32,17,82,
    51,11,136,55,35,48,52,48,54,139,12,32,55,139,12,137,94,35,48,52,67,48,132,25,32,7,130,120,33,48,50,79,172,5,33,65,55,73,164,5,36,65,66,53,51,11,131,23,38,49,50,51,46,97,108,116,65,
    162,5,33,51,67,65,178,14,32,54,136,15,87,79,6,65,146,8,86,228,6,136,15,86,86,6,136,15,84,157,6,65,178,8,32,103,132,138,32,16,86,249,10,132,16,36,11,103,98,114,101,65,188,6,36,9,103,
    100,111,116,133,21,41,102,108,111,114,105,110,46,115,115,48,66,196,5,34,52,51,49,132,73,35,83,82,66,12,131,181,34,52,67,70,136,218,33,50,48,75,119,5,130,7,67,180,6,75,119,5,130,15,
    32,56,67,196,5,75,119,5,130,15,32,65,134,15,32,66,134,7,68,74,5,32,53,70,63,5,33,48,53,73,223,6,32,53,70,63,5,130,15,32,51,131,39,130,7,32,52,134,7,32,53,134,7,132,103,130,23,32,55,
    134,15,132,103,130,15,32,57,134,15,132,103,130,15,132,103,130,7,135,103,32,68,134,31,32,69,134,7,32,70,133,7,70,63,5,33,48,53,70,63,5,130,7,32,50,134,23,134,127,73,223,6,32,53,70,63,
    5,130,31,134,127,70,63,5,130,15,134,127,70,63,5,130,15,132,127,33,70,66,82,7,5,130,7,132,135,130,7,132,135,130,7,132,135,130,7,132,135,130,7,132,135,33,70,66,82,7,5,130,7,32,49,131,
    127,130,7,132,135,130,7,132,135,130,7,32,52,134,23,32,53,134,7,132,135,130,23,132,127,130,7,132,255,130,7,134,127,81,255,5,130,15,134,119,78,244,5,33,70,66,81,239,5,130,7,32,49,133,
    71,81,231,5,130,15,134,95,81,223,5,130,15,32,55,134,31,134,95,81,223,5,130,23,134,95,34,52,66,9,70,84,8,32,9,70,109,8,131,51,36,48,51,48,50,9,70,83,8,134,17,74,37,5,32,51,79,234,5,
    130,33,132,93,130,7,132,93,130,7,132,85,130,7,32,66,134,47,132,181,130,15,32,70,133,15,46,49,50,12,100,111,116,98,101,108,111,119,99,111,109,98,133,20,83,130,6,32,51,83,130,7,33,56,
    53,132,23,34,52,56,54,134,7,32,51,134,7,133,124,32,53,73,170,5,33,48,53,77,74,6,32,53,73,170,5,130,15,133,39,32,53,73,170,5,130,15,133,71,32,53,73,170,5,130,15,133,172,32,53,73,170,
    5,130,15,32,57,66,106,5,73,170,5,130,15,133,180,32,53,73,170,5,130,15,32,68,133,31,73,146,5,35,48,53,67,50,134,15,46,55,9,122,101,114,111,46,100,110,111,109,8,111,110,101,133,8,33,
    116,119,133,17,36,10,116,104,114,101,133,19,36,9,102,111,117,114,132,29,35,9,102,105,118,133,19,35,8,115,105,120,132,18,37,10,115,101,118,101,110,133,10,36,101,105,103,104,116,132,
    10,34,9,110,105,134,80,32,7,132,99,34,108,102,6,131,97,130,6,131,95,34,108,102,8,133,93,34,108,102,7,132,91,131,7,131,89,130,31,131,87,130,31,133,85,130,8,133,83,130,32,132,81,33,108,
    102,133,179,35,110,117,109,114,132,179,132,8,131,83,131,8,134,179,131,10,133,179,133,9,131,89,132,39,131,91,132,39,133,93,132,10,133,95,132,40,132,97,132,40,132,179,35,111,115,102,
    7,131,180,131,7,131,97,130,7,32,9,133,182,130,9,32,8,132,183,132,8,131,94,130,8,32,7,131,93,130,7,32,9,133,92,131,9,133,91,130,9,32,8,132,90,130,8,32,10,132,89,36,115,108,97,115,104,
    133,200,32,116,130,20,132,200,132,8,131,102,131,8,134,200,131,10,133,200,133,9,131,105,132,39,131,106,132,39,133,107,132,10,133,108,132,40,132,109,131,9,68,219,5,80,251,5,33,50,48,
    80,251,5,130,7,32,50,134,23,32,51,134,7,32,52,134,7,32,53,134,7,32,54,134,7,32,55,134,7,32,56,134,7,66,104,7,32,69,133,15,81,107,5,130,79,32,68,134,15,134,23,81,35,5,35,50,48,55,65,
    134,23,32,67,133,7,81,91,5,130,47,133,15,33,50,49,134,111,80,131,5,52,50,49,50,48,16,97,102,105,105,49,48,49,48,51,100,111,116,108,101,115,115,136,16,32,53,134,16,32,12,84,4,10,33,
    50,14,70,105,6,134,27,32,14,71,85,5,32,66,134,14,35,0,0,0,1,130,3,8,38,10,0,56,0,86,0,5,68,70,76,84,0,32,99,121,114,108,0,32,103,114,101,107,0,32,104,101,98,114,0,32,108,97,116,110,
    0,32,0,4,130,41,54,0,255,255,0,2,0,0,0,1,0,2,109,97,114,107,0,22,109,107,109,107,0,14,130,17,36,2,0,2,0,3,132,7,131,29,44,4,3,142,2,2,1,88,0,10,0,6,0,16,130,45,32,10,130,3,36,1,1,28,
    0,166,130,7,8,48,194,0,12,0,19,0,148,0,142,0,136,0,130,0,124,0,118,0,112,0,106,0,100,0,94,0,88,0,82,0,76,0,70,0,64,0,58,0,52,0,46,0,40,0,1,253,215,5,232,130,5,34,203,5,185,130,5,34,
    195,6,64,130,5,32,192,131,5,35,255,251,5,182,130,11,34,155,6,33,130,81,34,4,6,32,130,5,32,118,132,11,40,9,6,147,0,1,255,255,5,213,130,17,34,0,5,239,130,5,34,3,5,244,130,5,34,1,5,121,
    130,47,34,135,5,230,131,11,33,6,31,130,11,32,236,132,5,32,32,131,5,35,2,121,6,49,130,17,8,36,173,6,150,0,2,0,4,3,116,3,116,0,0,3,118,3,118,0,1,4,1,4,13,0,2,4,17,4,20,0,15,0,22,0,0,
    56,160,130,3,32,154,130,3,32,142,130,3,32,136,130,3,32,130,130,3,32,124,130,3,32,118,130,3,32,112,134,7,32,106,130,7,32,100,130,3,32,94,130,3,32,88,130,3,32,82,134,23,32,76,130,7,32,
    70,130,3,32,46,130,3,32,40,130,3,32,34,130,3,36,28,0,0,55,236,130,117,32,6,24,116,101,36,65,77,7,32,0,130,197,34,124,0,46,130,5,38,58,0,12,0,4,0,28,130,155,34,16,0,10,130,15,34,0,254,
    52,130,5,34,3,254,20,130,207,34,154,254,141,132,11,32,59,24,78,81,12,32,16,130,113,32,206,130,3,32,122,130,3,32,116,130,3,32,110,130,3,32,80,130,3,32,74,130,3,32,68,134,3,32,62,130,
    7,32,56,134,11,32,50,130,7,32,44,130,3,32,26,130,3,36,14,0,0,54,252,24,116,235,36,33,5,0,131,163,38,8,0,1,1,80,0,172,130,223,32,182,130,167,40,5,0,126,0,98,0,82,0,32,130,11,32,3,65,
    229,10,36,14,0,1,6,166,130,47,36,3,0,44,0,38,130,29,35,26,0,20,0,131,19,32,169,130,19,130,25,37,6,31,0,1,3,236,131,11,33,4,216,131,11,33,1,33,131,11,33,2,13,130,11,36,2,0,72,0,66,130,
    247,32,54,130,33,32,218,132,15,32,56,65,9,7,33,3,219,131,37,33,3,221,132,49,32,61,130,11,32,2,65,37,9,33,3,218,130,15,34,1,5,16,132,27,32,58,132,77,32,70,132,61,36,1,3,138,3,142,130,
    15,36,38,0,0,54,182,130,3,36,176,0,1,54,170,130,7,32,164,130,3,32,158,130,3,32,152,130,3,32,146,130,3,32,140,130,3,32,134,134,7,32,128,130,7,32,122,130,3,32,116,130,3,32,110,130,3,
    32,104,134,23,32,98,130,7,32,92,130,63,32,86,130,3,32,80,130,3,32,74,130,15,32,68,130,3,32,62,130,3,32,56,130,3,32,50,130,19,32,44,130,3,32,38,130,3,32,32,134,3,32,26,130,7,32,20,134,
    11,32,14,130,7,32,8,130,39,36,2,0,1,53,246,130,3,32,234,130,3,36,216,0,2,0,8,66,41,5,8,43,2,53,2,53,0,2,3,116,3,116,0,3,3,118,3,118,0,4,4,1,4,30,0,5,4,32,4,32,0,35,4,34,4,34,0,36,4,
    37,4,37,0,37,0,9,65,139,7,33,0,4,130,9,32,8,130,71,47,222,51,38,0,5,52,80,0,12,2,241,51,20,51,14,51,130,103,40,0,0,51,2,50,252,50,246,0,130,0,37,50,240,50,234,50,228,132,9,46,222,50,
    216,50,210,50,204,0,0,50,198,50,192,50,186,132,19,36,180,50,174,50,168,132,9,36,162,50,156,50,150,132,9,46,144,50,138,50,132,50,126,0,0,50,120,50,114,50,108,132,19,36,102,50,96,50,
    90,132,9,36,84,50,78,50,72,132,9,32,66,130,69,42,60,50,54,0,0,50,48,50,42,50,36,132,19,36,30,50,24,50,18,132,9,46,12,50,6,50,0,49,250,0,0,49,244,49,238,49,232,133,19,34,49,226,50,131,
    8,38,0,49,220,49,214,49,208,131,10,37,49,202,49,196,49,190,132,9,46,184,49,178,49,172,49,166,0,0,49,160,49,154,49,148,132,19,36,142,49,136,49,130,132,9,36,124,49,118,49,112,132,9,36,
    106,49,100,49,94,132,9,36,88,49,82,49,76,132,9,36,70,49,64,49,58,132,9,36,52,49,46,49,40,132,9,36,34,49,28,49,22,131,9,37,53,180,49,16,49,10,132,19,46,4,48,254,48,248,48,242,0,0,53,
    180,48,236,48,230,131,19,37,48,224,48,218,48,212,132,9,36,206,48,200,48,194,132,9,46,188,48,182,48,176,48,170,0,0,48,164,48,158,48,152,132,19,34,164,48,146,134,9,36,140,48,134,48,176,
    132,19,46,128,48,122,48,116,48,110,0,0,48,104,48,98,48,92,132,19,36,86,48,80,48,74,132,9,37,68,48,62,48,56,48,130,242,37,48,44,48,38,48,32,132,19,36,26,48,20,48,14,132,9,36,8,48,2,
    47,252,131,9,37,47,246,47,240,47,234,132,9,46,228,47,222,47,216,47,210,0,0,48,86,47,204,47,198,132,19,36,192,47,186,47,180,132,9,36,174,47,168,47,162,132,9,36,156,47,150,47,144,132,
    9,36,138,47,132,47,126,132,9,36,120,47,114,47,108,132,9,34,102,47,96,131,7,37,0,0,47,90,47,84,134,9,33,78,51,132,117,130,19,137,9,32,72,136,19,32,66,136,9,32,60,136,9,32,54,136,9,34,
    48,47,42,133,69,35,50,240,47,36,133,9,35,47,30,50,192,144,9,32,24,136,19,32,18,136,9,34,12,47,6,134,39,34,0,50,24,133,9,34,46,250,50,134,19,138,9,32,244,136,19,32,238,136,9,32,232,
    136,9,34,226,46,220,134,59,34,214,49,154,144,9,32,208,136,19,32,202,136,9,34,196,49,82,134,39,34,190,49,238,134,9,34,184,46,178,134,9,34,172,49,46,144,9,32,166,136,19,32,160,136,9,
    32,154,136,9,32,148,136,9,34,142,46,136,133,59,35,53,180,46,130,133,9,35,46,124,48,236,144,9,32,118,136,19,32,112,136,9,34,106,46,100,134,39,137,9,32,94,136,19,32,88,136,9,34,82,46,
    76,134,39,34,70,48,80,134,9,34,64,48,62,144,9,32,58,136,19,32,52,136,9,32,46,136,9,33,40,49,134,169,35,46,34,47,204,134,59,137,9,32,28,136,19,32,22,136,9,34,16,47,132,134,39,34,10,
    46,4,133,9,33,45,254,135,19,33,45,248,66,17,7,33,45,242,65,13,7,33,45,236,136,19,32,230,136,19,34,224,45,218,133,59,35,49,52,45,212,133,9,35,45,206,50,234,133,9,35,46,124,49,16,133,
    9,33,45,200,136,19,32,118,136,19,32,194,135,19,33,45,188,156,39,34,182,50,216,133,69,35,49,4,48,254,133,9,32,47,66,107,8,35,45,176,45,170,133,19,33,45,164,66,137,7,33,45,158,65,133,
    7,33,45,152,136,19,32,146,136,19,32,140,136,19,32,188,135,19,35,50,198,45,134,133,69,35,53,180,45,128,133,9,32,47,66,227,8,32,46,65,223,8,35,45,122,50,156,133,29,33,45,116,136,9,32,
    110,135,9,35,50,162,45,104,134,29,34,98,50,138,134,9,34,92,48,182,133,9,35,50,144,45,86,133,9,33,45,80,65,233,7,33,45,74,66,7,7,33,45,68,136,9,32,62,135,9,35,48,164,45,56,134,49,34,
    50,45,44,134,9,33,38,50,67,241,6,35,46,94,48,146,133,19,35,50,84,45,32,133,9,35,48,140,45,26,133,9,34,45,20,48,134,209,33,45,14,136,239,34,8,48,122,133,29,36,50,66,45,2,0,132,0,35,
    48,128,44,252,132,8,34,0,50,66,135,39,33,48,128,137,39,165,19,33,44,246,67,241,8,32,34,135,219,35,50,30,44,240,133,89,34,48,86,44,66,17,6,33,44,228,136,39,32,28,135,39,34,44,222,44,
    65,223,6,130,49,32,210,136,49,32,204,69,89,5,33,44,198,68,25,7,33,44,192,66,237,7,33,44,186,136,19,32,180,67,1,8,68,105,9,67,61,8,35,44,174,44,168,133,79,34,44,162,44,65,183,6,35,44,
    150,49,214,134,19,33,144,48,65,3,6,34,49,220,44,65,173,6,34,48,8,44,67,21,6,33,44,126,136,39,32,120,135,39,36,44,114,49,196,0,132,0,34,44,108,47,134,219,33,44,102,136,19,32,96,135,
    19,35,49,202,44,90,132,38,35,0,47,246,44,69,149,6,148,39,34,184,44,78,134,39,34,228,44,72,133,9,34,44,66,49,68,185,6,35,44,60,47,222,133,19,33,49,184,135,19,33,47,228,135,19,33,44,
    54,68,255,8,32,70,67,211,7,33,44,48,135,19,33,44,42,136,19,32,36,136,19,32,30,136,19,32,24,136,19,32,18,68,15,8,69,109,9,68,55,8,35,49,160,44,12,65,143,8,69,199,6,35,44,0,49,118,133,
    19,34,43,250,47,65,93,6,33,43,244,69,129,7,33,43,238,68,65,7,33,43,232,136,19,34,226,49,64,134,49,34,220,47,114,134,9,32,214,136,19,32,208,136,19,32,202,136,19,32,196,136,19,34,190,
    43,184,134,49,34,178,43,172,134,9,34,166,43,160,134,9,69,179,8,33,43,148,70,183,7,32,43,69,179,8,33,46,250,70,73,8,32,64,69,29,7,35,49,202,43,136,133,59,35,47,246,43,130,133,9,34,43,
    124,43,135,209,34,112,43,106,134,19,34,100,43,94,134,9,34,88,43,82,134,9,34,76,43,70,134,9,34,64,43,58,134,9,34,52,43,46,133,9,33,51,20,69,29,5,35,43,40,43,34,133,9,37,0,0,43,28,43,
    22,133,29,34,49,106,49,70,13,6,33,50,198,67,81,5,35,43,40,49,70,135,239,33,50,144,68,45,5,35,43,40,50,12,66,217,7,34,50,84,50,66,27,6,35,43,16,43,10,133,69,34,50,48,50,71,157,7,32,
    30,67,71,7,34,43,4,42,69,19,6,135,59,37,43,40,42,248,42,242,133,49,33,49,244,71,17,7,35,42,236,42,230,66,77,15,33,49,88,65,163,5,130,49,34,224,42,218,143,169,35,42,212,42,206,133,19,
    35,42,200,42,194,131,9,130,219,65,213,8,33,42,188,136,89,34,182,42,176,131,29,37,0,0,42,170,42,164,134,9,32,158,65,23,7,34,42,152,42,68,165,6,33,42,140,136,49,36,134,42,128,42,122,
    131,41,37,42,116,42,110,42,104,132,9,32,98,67,201,7,33,42,92,136,79,34,86,42,80,131,27,35,0,0,42,74,136,89,36,68,47,204,42,62,131,21,33,42,56,135,99,68,235,9,34,42,50,42,69,39,6,35,
    42,38,42,32,131,37,35,0,0,42,26,136,19,34,20,42,14,133,19,33,48,68,136,109,35,8,42,2,0,132,0,35,41,252,41,246,132,8,35,0,41,240,41,68,175,6,34,41,228,41,67,131,6,34,41,216,41,68,155,
    6,33,41,204,135,209,34,41,198,41,73,25,6,37,41,186,41,180,41,174,131,61,35,41,168,41,162,131,7,36,0,0,47,174,41,68,115,6,33,41,150,135,169,33,41,144,136,59,32,138,135,129,33,41,132,
    136,19,32,126,135,49,32,47,73,85,8,34,41,120,41,67,51,6,33,41,108,66,77,7,34,41,102,41,70,3,6,33,49,202,68,125,7,34,50,102,41,68,115,6,34,41,84,41,66,57,6,35,41,72,41,66,133,139,138,
    69,34,60,41,54,134,19,33,48,41,66,77,6,34,42,248,41,73,235,6,66,197,7,37,0,0,41,30,41,24,133,39,35,51,2,50,252,133,9,66,207,9,34,41,18,41,68,25,6,66,207,7,130,49,33,6,41,133,38,34,
    0,40,250,66,237,7,35,40,244,40,238,133,20,33,40,232,136,9,32,226,135,129,34,40,220,40,69,139,6,66,217,10,67,11,6,33,0,0,67,11,9,66,207,19,33,50,240,72,61,7,69,29,9,33,51,2,136,209,
    66,207,18,35,40,208,40,202,134,139,34,196,40,190,134,9,33,184,40,69,109,6,35,40,172,40,166,134,19,33,160,40,74,119,6,35,40,148,40,142,134,19,33,136,40,68,45,6,34,40,124,40,68,45,6,
    34,40,112,40,68,45,6,34,40,100,40,68,45,6,33,49,52,73,25,7,35,40,88,40,82,134,59,33,76,40,68,55,6,34,40,64,40,68,55,6,34,40,52,40,68,55,6,33,53,180,72,131,7,35,40,40,40,34,134,49,33,
    28,40,68,55,6,35,40,16,40,10,134,19,32,4,135,9,35,39,254,39,248,133,19,34,39,242,39,74,139,6,35,39,230,39,224,134,19,32,218,65,173,7,66,227,9,34,39,212,39,67,201,6,35,48,44,48,38,133,
    39,33,53,180,73,115,7,35,39,200,39,194,133,19,33,47,138,69,189,7,34,39,188,39,72,181,6,35,47,156,47,150,133,29,35,39,176,39,170,134,9,32,164,66,177,7,35,39,158,39,152,134,19,137,9,
    34,146,39,140,134,19,34,134,39,128,134,9,34,122,39,116,134,9,32,110,67,221,7,35,39,104,39,98,134,19,34,92,39,86,133,9,32,46,75,63,8,35,39,80,44,204,133,19,33,39,74,65,53,7,33,39,68,
    65,23,7,33,47,246,71,107,7,35,48,164,48,158,133,39,32,46,75,83,8,130,19,68,115,6,35,39,62,39,56,133,29,35,39,50,39,44,134,9,73,125,8,33,39,38,65,73,8,32,32,136,249,34,26,39,20,134,
    39,35,14,39,8,0,132,0,34,39,2,38,67,1,6,33,38,246,70,253,7,35,38,240,47,168,132,28,32,0,148,19,32,234,136,39,32,228,135,39,32,46,76,167,9,75,123,9,73,195,8,32,38,68,103,6,35,0,0,38,
    216,69,229,7,34,38,210,48,65,33,6,34,51,20,38,65,13,6,35,49,52,38,198,133,119,33,50,12,70,223,7,33,48,68,66,107,7,34,49,160,38,68,145,6,33,48,86,71,217,7,77,191,9,33,38,186,67,131,
    7,76,197,9,33,38,180,66,87,7,34,38,174,38,135,209,33,162,38,68,185,6,35,38,150,38,144,133,109,33,38,138,66,227,7,35,38,132,38,126,134,19,33,120,38,68,155,6,35,38,108,38,102,134,19,
    33,96,38,68,135,6,34,38,84,38,68,135,6,34,38,72,38,68,135,6,34,38,60,38,68,125,6,35,39,176,38,48,134,49,34,42,38,36,134,9,33,30,38,68,115,6,34,38,18,38,68,95,6,33,38,6,66,176,6,32,
    0,70,143,9,69,99,9,65,3,11,71,227,7,35,37,250,37,244,133,79,35,37,238,37,232,134,9,32,226,136,19,32,220,136,19,34,214,37,208,134,29,34,202,37,196,133,9,34,41,102,37,68,25,6,34,37,184,
    37,68,25,6,34,37,172,37,68,25,6,35,37,160,37,154,133,39,34,37,148,37,68,25,6,33,37,136,65,103,7,34,49,142,37,68,35,6,33,48,26,77,81,7,35,37,124,37,118,134,49,33,112,37,68,45,6,34,37,
    100,37,68,45,6,35,37,88,37,82,133,29,34,43,100,37,78,5,6,34,37,70,37,73,45,6,34,49,106,37,68,45,6,35,39,68,37,52,133,39,35,38,108,37,46,133,9,34,37,40,37,68,45,6,33,37,28,76,97,7,33,
    37,22,66,207,7,33,37,16,69,99,7,33,37,10,76,7,7,35,37,4,36,254,133,59,65,123,9,34,36,248,36,72,41,6,34,39,218,36,68,75,6,65,163,10,68,75,8,34,36,230,36,68,95,6,34,36,218,36,77,231,
    6,33,36,206,73,45,7,32,39,136,39,35,50,240,36,200,133,99,34,53,180,36,68,85,6,35,49,184,36,188,133,19,34,41,216,36,68,85,6,72,121,7,36,0,0,36,176,36,68,85,6,147,19,35,41,102,36,164,
    133,59,35,36,158,36,152,133,9,35,43,88,36,146,133,9,34,41,198,36,68,105,6,35,50,222,36,134,133,19,35,36,128,36,122,134,9,32,116,69,229,7,33,36,110,73,65,7,32,36,137,19,33,104,36,67,
    91,6,34,36,92,36,68,125,6,35,36,104,36,80,133,59,35,40,40,36,74,133,9,33,36,68,70,213,7,33,36,62,69,159,7,35,36,56,36,50,134,29,34,44,36,38,133,9,35,38,60,36,32,133,9,34,48,26,36,77,
    131,6,34,36,20,36,72,111,6,36,36,8,36,2,0,132,0,35,36,206,35,252,132,8,36,0,35,246,35,240,133,9,34,36,116,35,72,111,6,34,35,228,35,68,7,6,34,35,216,35,72,111,6,34,35,204,35,67,241,
    6,33,35,192,74,89,7,33,35,186,70,103,7,32,47,81,197,8,32,46,75,13,8,33,35,180,75,13,7,34,35,174,46,74,239,6,33,35,168,74,119,5,35,0,0,35,162,70,113,7,34,35,156,35,69,219,6,33,35,144,
    70,13,7,33,35,138,136,19,32,132,136,19,32,126,65,3,7,33,35,120,65,3,7,33,35,114,71,227,7,33,35,108,69,149,7,35,35,102,35,96,133,209,34,35,90,35,76,247,6,33,35,78,68,115,7,33,35,72,
    68,105,7,33,35,66,136,19,32,60,135,19,32,46,82,1,9,80,213,8,67,221,19,33,35,54,75,213,8,32,46,67,241,7,33,35,48,71,147,7,33,35,42,70,93,7,33,35,36,72,11,7,33,35,30,69,219,7,33,35,24,
    135,19,80,233,9,33,35,18,135,19,32,46,69,149,8,33,35,12,66,57,7,33,35,6,70,233,7,35,35,0,34,250,133,219,34,34,244,34,72,211,6,33,34,232,72,31,7,35,34,226,39,128,133,29,35,37,124,34,
    220,134,9,34,112,34,214,133,9,35,34,208,34,202,133,9,34,36,158,34,68,105,6,76,17,9,33,34,190,71,97,7,34,43,28,34,76,237,6,80,233,9,35,34,178,34,172,133,59,33,34,166,136,9,34,160,34,
    154,134,19,34,148,34,142,134,9,34,136,34,130,134,9,33,124,34,68,115,6,32,36,66,227,8,33,34,112,76,177,7,34,34,106,34,76,137,6,35,34,94,34,88,134,49,33,82,34,68,125,6,34,36,128,34,72,
    171,6,35,34,64,34,58,134,29,34,52,34,46,134,9,32,40,66,67,7,75,163,9,35,34,34,34,28,133,29,34,36,110,34,72,181,6,35,51,20,34,16,133,19,35,49,52,34,10,133,9,33,34,4,66,187,7,33,33,254,
    66,187,7,32,33,82,101,8,32,33,82,101,8,148,19,82,121,8,32,33,82,121,8,33,33,224,136,79,32,218,67,11,8,32,72,135,119,33,46,166,135,119,33,33,212,136,39,32,206,136,119,147,19,32,200,
    136,39,32,194,136,39,32,188,136,19,32,182,82,241,8,32,236,135,99,33,45,230,135,99,35,50,198,33,176,133,229,34,53,180,33,68,205,6,32,33,82,141,8,33,33,158,67,111,7,32,33,82,141,8,32,
    33,82,141,8,32,33,82,141,8,33,33,134,136,39,147,19,32,128,65,123,7,33,33,122,136,39,32,116,136,19,32,110,135,19,33,47,24,135,139,33,46,118,136,139,32,104,82,81,9,32,33,68,245,6,35,
    50,12,33,92,133,179,34,48,68,33,68,255,6,33,33,80,78,125,7,33,33,74,76,157,7,33,33,68,136,19,32,62,156,19,32,56,136,39,32,50,136,39,32,44,136,19,32,38,81,167,8,32,244,135,119,33,46,
    58,136,119,33,32,33,69,49,6,34,33,20,33,69,49,6,32,33,147,19,32,8,136,39,32,2,135,39,33,32,252,135,19,33,32,246,136,19,33,240,32,69,79,6,35,32,228,32,222,133,229,35,49,160,32,216,133,
    9,34,48,86,32,69,89,6,33,32,204,81,7,7,33,32,198,71,197,7,35,32,192,32,186,133,39,35,32,180,32,174,134,9,147,19,32,168,136,39,32,162,136,39,32,156,136,19,32,150,136,19,34,144,32,138,
    134,69,33,132,32,73,55,6,35,49,88,32,120,75,113,15,33,32,114,79,139,5,35,0,0,32,108,68,175,7,33,32,102,136,19,32,96,135,19,35,49,184,32,90,134,59,34,228,32,84,133,9,35,50,12,32,78,
    133,9,35,48,68,32,72,133,9,33,44,198,135,19,33,44,192,135,19,33,32,66,69,209,7,34,32,60,32,73,135,6,33,32,48,78,155,7,138,9,32,42,136,19,137,9,32,36,69,9,7,32,32,137,9,32,30,136,19,
    136,9,35,35,156,32,24,133,129,83,205,9,35,32,18,32,12,133,19,34,32,6,32,133,8,32,0,78,105,9,78,65,9,33,49,160,65,143,7,37,31,250,31,244,31,238,131,42,37,31,232,31,226,31,220,132,9,
    33,214,31,73,215,6,32,31,137,9,32,208,136,19,32,202,136,9,32,196,136,9,32,190,136,9,32,184,136,9,34,250,31,178,131,77,34,0,0,31,137,9,32,172,136,29,34,166,31,160,134,29,32,154,136,
    19,34,250,31,148,134,19,32,142,70,113,7,130,59,135,139,137,119,130,179,134,39,147,19,32,136,67,131,7,33,31,136,85,183,7,35,31,130,31,124,87,61,15,86,127,9,85,253,9,35,48,86,44,6,133,
    39,35,31,118,31,124,90,83,5,137,59,33,31,112,135,69,33,31,106,136,9,36,100,46,100,31,94,131,51,138,169,44,88,47,204,0,0,31,82,31,76,31,70,47,150,130,9,38,64,31,58,31,52,31,46,130,9,
    32,40,130,9,34,34,47,186,130,9,38,28,31,22,31,16,40,130,130,9,32,10,130,9,44,4,48,122,0,0,30,254,31,22,30,248,30,242,130,9,34,236,30,230,133,29,33,30,224,130,19,34,218,30,212,130,19,
    32,206,130,9,34,200,30,194,130,9,38,188,30,182,30,176,30,170,130,9,38,164,30,158,30,152,30,146,130,9,48,140,30,134,30,128,30,122,0,0,30,116,30,110,30,104,30,98,130,9,38,92,30,86,30,
    80,30,74,130,9,32,68,130,59,34,62,30,56,130,9,32,50,130,49,34,44,47,222,130,9,38,38,30,32,30,26,48,62,130,9,38,20,30,14,30,8,40,82,130,9,45,2,30,158,29,252,29,246,0,0,29,240,29,234,
    31,132,199,39,29,228,30,14,29,222,29,216,130,19,38,210,29,204,29,198,29,192,130,9,32,186,130,9,34,180,43,10,130,9,32,174,130,89,132,139,49,31,28,29,168,29,162,29,156,29,150,29,144,
    29,138,29,132,37,244,130,29,35,126,29,120,31,65,143,19,33,136,29,76,207,6,36,31,136,29,108,0,133,0,37,1,1,24,254,141,0,131,5,44,52,0,1,0,34,4,79,0,1,2,190,2,152,130,5,32,154,131,11,
    46,0,228,5,91,0,1,3,58,1,193,0,1,5,86,4,130,5,33,2,226,130,56,34,1,2,226,132,29,32,17,132,47,38,89,2,155,0,1,2,118,131,11,35,1,11,1,251,130,11,32,70,132,35,32,70,132,35,32,24,132,23,
    34,253,2,40,130,101,32,253,131,23,33,1,253,132,53,34,102,3,77,130,113,32,25,132,11,33,73,3,130,101,33,2,120,132,53,32,120,132,17,32,105,131,47,33,2,105,132,65,32,18,132,17,38,108,2,
    156,0,1,2,107,132,17,32,22,132,83,38,70,2,157,0,1,1,192,132,11,32,12,132,95,32,12,131,65,33,1,12,132,47,32,135,132,47,32,134,131,17,33,2,134,132,53,32,27,132,23,32,136,132,89,32,136,
    132,23,32,136,132,23,32,31,132,65,32,69,132,47,130,22,35,0,1,2,0,132,23,32,19,132,23,32,99,132,101,32,249,132,89,32,249,132,23,32,29,132,23,32,87,131,23,33,2,5,131,23,33,2,5,132,23,
    32,23,132,5,38,8,3,140,0,1,1,3,131,23,33,1,3,132,113,32,156,132,89,32,140,132,41,32,140,132,17,32,138,131,131,33,0,38,132,47,32,20,132,95,32,42,132,47,32,42,132,17,32,6,131,47,39,1,
    16,5,23,0,1,2,138,132,59,32,138,132,23,32,26,134,119,32,156,130,23,32,15,132,11,32,9,132,41,32,178,132,59,32,178,132,35,32,32,132,23,34,132,2,155,130,35,32,39,132,17,32,20,132,17,34,
    250,1,43,130,17,32,101,131,11,35,3,199,6,31,130,11,32,186,132,5,34,104,5,239,130,11,34,104,5,248,131,5,33,4,78,130,5,34,97,254,20,130,5,38,104,6,32,0,1,1,16,132,17,34,110,5,182,130,
    11,34,100,254,141,131,5,33,7,232,130,29,34,134,254,116,131,5,133,23,33,7,87,132,29,32,52,132,29,32,97,131,5,33,6,221,132,11,32,82,132,5,32,72,132,5,32,137,132,5,36,138,0,1,3,56,132,
    53,32,104,131,179,33,2,63,131,11,33,2,158,132,17,32,100,131,17,33,1,100,132,17,32,224,132,29,32,224,132,11,32,225,132,11,32,225,132,11,40,234,254,113,0,1,2,120,7,153,132,5,36,156,0,
    1,1,20,131,11,130,5,130,11,33,4,129,131,41,33,4,126,132,41,32,59,132,185,33,107,254,130,149,33,3,36,131,5,33,1,192,132,215,32,59,132,5,34,19,5,233,130,65,33,78,7,130,161,35,2,19,6,
    150,132,11,131,215,32,78,131,227,33,2,192,133,5,131,65,33,3,52,131,11,33,3,52,132,83,32,192,131,53,33,3,52,132,53,32,192,131,53,130,11,131,53,32,192,131,119,35,2,192,6,33,130,221,32,
    52,131,11,130,23,130,233,33,2,128,132,89,33,243,7,131,35,32,101,132,83,32,238,132,5,32,132,133,5,132,95,32,43,132,95,32,43,132,95,32,132,132,95,32,43,132,95,32,132,132,95,32,43,132,
    59,32,132,132,95,32,132,132,95,32,43,132,95,33,43,7,131,95,46,110,7,19,0,1,3,34,8,101,0,1,2,110,6,247,132,11,32,73,132,11,32,127,131,11,33,7,209,132,11,130,221,33,3,34,132,71,32,107,
    132,107,32,36,131,5,33,1,20,132,5,32,16,132,161,32,75,131,71,33,2,111,132,71,32,75,131,71,130,11,131,71,33,75,6,130,71,33,2,111,132,71,32,75,131,155,130,11,32,82,130,83,32,75,132,53,
    32,111,132,83,32,96,132,203,32,103,132,5,34,69,7,15,130,29,34,149,8,97,130,5,34,69,7,6,132,11,32,88,130,5,34,69,6,194,132,11,130,173,33,2,69,132,107,32,149,132,107,32,69,132,107,33,
    149,8,131,107,32,69,132,107,32,149,132,107,32,69,132,95,32,149,132,95,32,69,132,89,32,157,132,5,34,118,254,95,130,71,34,241,254,90,131,5,33,5,182,130,5,32,103,132,5,40,175,0,0,0,1,
    2,175,4,78,130,5,32,227,132,11,32,227,132,23,32,147,131,11,33,3,17,133,5,131,17,33,3,176,132,11,32,176,131,41,33,4,20,131,11,33,4,20,132,23,32,126,132,59,38,62,254,129,0,1,2,62,132,
    11,34,147,254,119,130,11,32,147,132,29,32,69,132,53,32,69,131,23,33,3,157,132,11,32,157,132,23,38,156,6,20,0,1,3,156,132,17,32,156,132,113,32,121,131,11,33,2,39,132,65,32,70,132,173,
    32,150,132,173,32,150,131,29,33,1,202,132,17,32,35,131,17,45,3,40,5,223,0,1,3,116,7,72,0,1,1,198,131,119,33,1,198,132,53,32,36,132,119,32,36,132,77,32,123,131,35,33,2,207,131,35,37,
    2,133,7,138,0,1,130,5,130,47,35,2,19,5,116,131,11,33,6,221,130,59,32,255,132,35,34,143,7,49,130,77,32,34,131,5,33,2,161,132,89,32,27,132,53,32,161,131,41,33,3,27,131,41,34,2,14,254,
    130,173,33,2,14,132,101,32,87,132,167,32,87,132,101,32,2,132,65,32,102,131,47,33,3,21,132,59,32,120,132,59,32,71,132,11,32,6,133,11,132,251,32,6,132,53,32,234,132,53,40,75,6,24,0,1,
    2,111,7,129,130,119,32,129,132,29,32,143,132,23,32,69,132,23,32,149,132,23,32,39,131,215,33,3,39,132,29,32,196,131,215,33,3,196,132,35,38,123,254,128,0,1,2,123,132,137,32,207,132,239,
    32,182,131,41,33,2,182,132,41,32,28,132,17,34,134,254,12,130,35,32,134,132,35,34,245,254,0,130,11,32,245,132,59,32,121,132,41,32,250,132,35,34,82,254,11,130,23,32,82,132,35,32,212,
    132,35,32,212,131,35,33,3,21,131,131,33,3,120,132,131,32,17,132,131,34,247,254,118,130,167,32,16,131,191,33,3,16,132,101,32,247,132,11,32,247,132,83,32,118,132,65,32,207,132,11,32,
    147,132,155,32,147,132,17,32,230,132,101,32,242,132,65,32,129,132,11,32,70,132,29,32,70,132,29,32,153,132,17,38,18,254,20,0,1,2,18,131,17,33,1,246,132,155,32,59,132,23,34,71,254,52,
    130,23,32,248,131,5,33,3,28,132,149,38,124,254,10,0,1,3,124,131,41,33,4,86,131,173,33,4,86,132,113,32,144,132,95,32,4,131,53,33,3,4,132,17,32,203,131,155,33,2,203,131,11,34,1,35,6,
    131,95,32,129,132,23,32,44,132,119,32,130,132,11,32,77,132,137,32,77,132,17,32,190,132,113,32,2,132,113,32,102,132,113,32,70,132,83,132,167,33,3,176,132,29,32,49,131,125,33,2,49,132,
    47,32,174,131,125,33,2,174,132,95,32,202,131,107,33,1,202,132,23,32,35,132,119,32,35,132,95,32,117,132,11,32,101,132,125,32,114,132,11,32,114,132,23,32,173,132,227,34,173,6,24,130,
    203,32,29,132,179,39,29,7,108,0,1,1,254,254,130,167,33,1,254,132,71,32,153,131,11,39,4,108,254,19,0,1,4,108,132,239,32,240,132,11,32,240,132,65,40,34,6,33,0,1,2,154,7,138,130,5,32,
    34,132,89,32,30,132,53,32,154,132,11,32,133,132,35,40,2,254,104,0,1,2,2,5,84,130,5,34,98,254,71,131,5,33,6,218,130,77,32,64,131,35,33,4,64,131,47,33,5,1,131,11,33,5,1,132,47,32,135,
    131,11,33,2,250,133,5,131,17,33,4,2,132,41,32,2,132,125,32,175,132,11,32,175,132,41,32,82,132,35,32,79,132,113,32,193,132,11,32,190,132,47,32,46,132,35,32,46,132,89,32,32,132,89,32,
    32,132,47,38,145,5,39,0,1,2,180,131,17,33,2,180,131,17,33,3,60,131,11,33,3,60,131,41,32,3,132,23,32,3,132,23,33,2,161,131,215,33,3,27,132,215,32,246,132,47,38,69,253,200,0,1,2,157,
    131,5,33,3,215,132,29,32,165,132,29,32,77,132,71,40,47,5,223,0,1,3,189,7,72,130,5,32,47,132,29,32,189,131,29,33,1,207,131,59,35,1,207,5,139,130,59,32,42,131,11,41,2,42,6,229,0,1,2,
    138,254,129,131,5,132,185,34,19,6,24,130,11,32,44,132,53,32,142,131,35,33,3,142,132,143,32,103,132,11,32,103,132,35,132,251,33,1,198,131,35,35,2,128,6,20,130,47,32,66,131,29,33,2,66,
    132,41,32,94,132,41,32,94,131,11,33,1,185,132,47,32,98,132,29,32,98,132,29,32,40,132,29,32,33,132,23,32,198,132,23,32,198,132,23,32,164,132,23,32,164,132,23,32,107,132,5,38,135,254,
    128,0,1,2,135,132,11,33,246,254,131,101,32,246,131,107,33,1,247,132,197,32,247,132,23,32,131,131,11,33,2,131,132,11,32,144,132,71,32,11,132,71,32,11,132,17,32,93,132,29,32,93,132,11,
    32,44,132,11,32,73,132,11,32,168,132,227,32,161,132,17,32,172,132,17,32,2,132,83,32,241,132,65,32,21,132,65,32,17,132,23,32,101,132,131,32,101,132,203,133,173,32,4,132,23,32,90,132,
    59,32,90,132,11,32,105,132,11,38,105,6,30,0,1,2,145,132,11,38,145,5,182,0,1,4,55,131,11,33,4,55,131,11,33,2,143,132,23,32,2,132,11,32,138,132,11,32,138,131,11,33,3,116,132,107,32,120,
    132,23,32,202,132,23,32,202,132,59,32,58,132,59,32,58,132,11,32,45,132,11,32,45,132,35,32,207,132,35,32,182,132,11,38,255,254,119,0,1,2,255,132,11,32,219,132,23,32,219,132,11,32,181,
    132,89,38,42,7,129,0,1,3,27,131,23,33,3,48,132,23,32,92,132,23,32,120,132,17,32,122,132,17,32,210,132,65,32,210,132,11,32,122,131,23,32,2,133,23,32,239,132,23,32,133,132,17,38,133,
    7,129,0,1,2,129,132,11,38,129,7,138,0,1,3,224,131,11,33,3,224,132,77,32,206,132,11,32,206,131,11,39,0,0,254,116,0,1,2,153,131,17,33,2,153,132,83,32,121,132,47,32,2,132,35,132,251,39,
    3,47,6,108,0,1,2,120,132,5,32,110,132,5,38,120,5,223,0,1,1,20,131,5,33,3,47,132,41,46,26,254,20,0,1,3,26,6,18,0,1,4,74,4,78,130,47,32,58,131,17,33,2,58,132,11,32,242,132,11,32,242,
    132,11,32,120,131,5,33,1,246,131,53,33,1,246,132,17,32,127,131,11,33,2,127,132,23,37,242,254,118,0,1,1,133,41,32,109,132,53,32,109,132,35,32,178,132,35,32,178,132,35,32,236,132,35,
    33,236,6,130,113,33,2,57,132,29,32,133,132,41,32,133,132,11,32,57,132,41,34,57,6,33,130,71,32,20,131,17,33,4,40,131,11,33,2,101,132,5,32,156,132,65,32,241,132,65,32,241,132,65,32,52,
    132,53,34,107,6,24,130,185,68,121,5,132,203,33,2,26,132,59,34,126,6,31,130,215,32,10,132,83,32,157,131,11,33,2,126,132,47,32,123,132,17,34,120,6,180,131,101,37,6,108,0,1,2,127,132,
    131,32,156,131,11,33,1,255,132,29,32,52,132,23,32,126,131,5,33,3,33,131,17,39,3,33,5,182,0,1,3,68,132,11,32,68,132,11,32,64,132,11,32,64,131,11,33,2,76,132,53,32,76,132,11,32,239,132,
    11,32,239,132,11,32,58,132,11,32,58,132,11,32,118,132,11,32,118,132,11,32,32,132,11,32,121,132,11,32,157,131,5,33,0,41,131,5,33,1,172,131,23,33,1,20,131,155,33,3,126,132,95,32,126,
    132,107,32,150,132,11,32,150,132,11,32,131,132,11,32,129,132,11,32,171,132,11,32,176,132,11,32,28,132,11,72,111,5,32,167,132,95,32,159,132,83,38,238,254,59,0,1,2,22,131,5,41,3,129,
    6,33,0,1,3,143,7,138,130,17,34,69,7,169,130,5,32,151,132,41,40,151,7,172,0,1,2,74,254,20,130,5,38,239,5,203,0,1,1,90,131,23,41,1,189,6,31,0,1,1,228,6,32,130,23,34,85,7,137,131,11,33,
    5,239,132,11,32,87,132,23,130,77,33,2,85,132,77,34,78,7,72,130,23,32,19,132,41,32,78,131,41,33,3,47,131,11,33,3,189,131,11,35,2,101,254,52,130,29,32,238,132,5,34,128,6,147,130,11,34,
    243,7,251,130,5,34,128,5,248,132,11,32,97,132,11,32,116,130,5,34,243,6,221,132,17,32,82,130,95,33,74,6,131,143,32,59,132,119,32,192,132,197,32,59,131,5,33,1,238,132,167,32,22,131,5,
    33,1,238,132,113,32,69,132,35,32,238,131,221,33,2,69,131,143,33,1,234,132,23,32,135,132,23,32,14,132,59,32,170,132,59,32,234,132,35,32,135,131,35,33,3,211,131,227,39,3,211,4,78,0,1,
    3,192,132,11,34,192,5,182,130,125,32,110,131,149,34,3,34,7,131,149,33,110,5,130,149,33,3,34,132,149,32,128,131,113,34,3,19,254,131,17,32,26,132,47,32,26,132,59,32,19,132,227,32,128,
    131,95,130,29,36,59,0,1,3,19,132,131,32,16,132,119,32,103,132,119,34,16,7,232,130,209,32,40,131,23,33,2,50,131,53,33,2,50,132,29,32,165,132,29,32,37,132,65,32,81,132,95,34,49,5,239,
    130,41,34,20,254,52,130,5,32,16,131,137,33,1,16,131,131,130,5,32,233,130,17,34,49,6,20,130,161,32,251,131,125,33,1,18,131,83,33,2,251,131,59,33,3,49,132,125,34,59,7,87,130,125,32,59,
    131,185,33,3,59,132,83,32,96,131,71,33,2,103,132,5,32,111,131,29,33,2,75,131,83,33,2,111,132,221,32,75,131,89,33,2,111,132,221,32,126,131,83,33,2,126,132,95,32,230,132,59,32,75,131,
    137,33,3,10,132,83,32,10,132,101,32,10,132,191,32,69,132,77,32,157,132,5,38,149,5,188,0,1,2,69,132,83,32,149,132,83,32,69,132,83,32,149,132,83,34,19,5,223,130,29,34,121,254,22,131,
    5,132,89,34,19,6,33,130,11,32,128,132,23,34,128,6,32,131,11,132,17,34,111,4,78,130,11,32,110,132,23,33,110,5,130,233,33,2,110,132,29,32,110,132,29,32,128,132,17,32,108,132,161,38,108,
    6,30,0,1,1,16,131,41,33,1,16,131,35,33,1,24,131,23,130,11,131,89,32,75,132,65,32,75,132,59,32,75,132,59,38,71,254,20,0,1,3,129,131,35,33,3,143,132,101,34,69,6,147,132,179,131,155,32,
    69,132,89,32,69,132,47,32,69,132,47,32,149,131,41,41,2,149,6,31,0,1,2,112,5,182,130,5,32,78,132,245,34,243,7,72,130,11,34,243,7,137,132,5,36,138,0,1,3,34,131,41,33,3,38,131,35,33,3,
    34,131,29,130,5,33,82,0,130,23,131,35,130,11,131,35,33,19,7,130,17,33,2,236,131,41,33,2,236,132,77,32,111,132,71,32,111,132,71,32,111,132,89,32,248,132,161,32,143,131,35,33,3,230,132,
    35,38,149,7,10,0,1,2,149,132,41,32,149,132,65,32,149,132,47,32,149,131,47,41,1,125,3,2,0,1,1,137,5,205,130,5,34,98,3,13,130,5,32,100,131,11,33,3,128,131,221,33,1,241,131,71,33,1,228,
    131,11,33,3,253,131,5,35,0,199,254,19,130,77,32,19,131,11,33,4,35,131,5,33,2,39,131,35,33,2,36,131,11,33,6,52,132,41,32,45,132,125,32,47,132,11,32,250,132,35,32,15,132,35,32,13,132,
    53,75,69,5,32,101,132,89,32,130,130,61,40,1,2,199,5,72,0,1,1,192,132,17,32,74,131,11,33,3,140,132,125,32,238,132,17,32,238,132,71,32,51,132,17,32,14,132,17,32,234,132,71,32,127,132,
    23,32,223,66,15,5,132,17,32,185,132,35,32,16,132,17,32,165,132,119,32,110,131,95,33,4,163,132,11,32,107,132,131,32,110,132,41,32,149,132,17,32,128,132,17,132,233,32,7,132,203,33,3,
    202,132,185,32,215,133,71,131,59,39,1,179,6,20,0,1,1,16,131,23,33,1,16,131,11,33,2,50,132,11,32,19,131,11,34,0,50,254,131,29,32,247,67,101,5,132,23,32,16,131,11,33,2,128,132,119,32,
    89,132,47,32,130,132,23,32,18,131,11,33,4,95,132,119,32,37,131,53,33,2,21,132,191,38,20,6,31,0,1,1,64,132,137,32,7,131,11,33,4,100,132,35,32,96,132,17,32,121,132,71,67,71,5,32,91,132,
    17,32,40,131,71,32,3,132,233,33,2,71,131,17,33,4,48,132,101,32,148,132,29,32,166,132,101,32,42,132,65,32,69,132,17,32,69,132,239,38,111,5,182,0,1,2,99,132,17,32,85,131,11,33,4,113,
    132,17,32,78,132,17,32,78,132,17,32,164,132,17,32,102,132,17,32,102,131,11,33,7,80,131,5,33,3,179,131,17,33,3,189,132,35,32,184,132,35,32,110,132,35,32,117,131,11,33,5,186,132,17,32,
    238,132,17,32,243,132,11,37,59,2,219,0,1,4,132,83,33,2,59,132,23,32,59,132,59,132,5,33,2,22,132,17,32,69,132,17,32,221,132,47,32,170,132,17,32,135,132,107,34,34,254,164,130,59,32,181,
    132,23,32,112,132,23,32,136,133,23,131,83,33,6,26,132,11,32,36,132,143,32,34,132,125,32,253,132,17,32,19,132,17,32,19,132,179,32,31,132,17,32,153,132,17,32,165,132,71,32,34,131,59,
    33,3,1,131,11,33,1,40,132,119,32,216,132,23,32,165,132,95,32,179,132,11,32,29,131,5,39,0,0,254,155,0,1,1,37,132,41,32,238,132,5,32,44,131,35,33,1,44,132,35,32,251,131,71,33,5,203,132,
    11,32,245,132,59,32,251,132,131,32,167,132,113,32,49,132,113,132,227,33,4,9,132,35,32,26,132,35,32,98,132,113,32,75,132,17,32,103,132,17,65,139,5,32,236,132,77,32,173,132,23,32,190,
    132,23,32,230,132,41,32,229,132,17,32,248,132,77,32,10,132,95,32,15,132,17,32,144,132,35,32,155,132,35,32,82,132,17,32,157,132,17,32,149,130,11,38,2,0,49,0,36,0,61,130,15,38,68,0,93,
    0,26,0,108,130,1,34,52,0,124,130,1,58,53,0,130,0,141,0,54,0,146,0,152,0,66,0,154,0,184,0,73,0,186,0,222,0,104,0,224,130,1,34,141,0,226,130,1,34,142,0,228,130,1,40,143,0,230,0,233,0,
    144,0,235,130,1,34,148,0,237,130,1,34,149,0,239,130,1,34,150,0,241,130,1,8,204,151,0,244,1,73,0,152,1,85,1,85,0,238,1,87,1,88,0,239,1,90,1,101,0,241,1,103,1,117,0,253,1,119,1,159,1,
    12,1,162,2,0,1,53,2,74,2,74,1,148,2,77,2,77,1,149,2,79,2,82,1,150,2,84,2,87,1,154,2,89,2,118,1,158,2,125,2,126,1,188,2,130,2,176,1,190,2,178,2,181,1,237,2,183,2,196,1,241,2,198,3,49,
    1,255,3,51,3,51,2,107,3,53,3,97,2,108,3,109,3,115,2,153,3,117,3,117,2,160,3,122,3,132,2,161,3,143,3,143,2,172,3,148,3,149,2,173,3,151,3,164,2,175,3,166,3,172,2,189,3,174,3,176,2,196,
    3,179,3,179,2,199,3,182,3,190,2,200,3,192,3,192,2,209,3,201,3,227,2,210,4,111,4,112,2,237,4,114,4,115,2,239,0,42,0,0,1,136,130,3,36,130,0,1,1,124,130,7,32,118,130,3,32,112,130,3,32,
    106,130,3,32,100,130,3,32,94,130,3,32,88,134,7,32,82,130,7,32,76,130,3,32,70,130,3,32,64,130,3,32,58,134,23,32,52,130,7,32,46,130,63,32,40,130,3,32,34,130,3,32,28,130,15,32,22,130,
    3,32,16,130,3,32,10,130,3,36,4,0,1,0,254,130,3,32,248,130,3,32,242,134,3,32,236,130,7,32,230,134,11,32,224,130,7,40,218,0,0,0,212,0,4,0,206,130,11,36,200,0,3,0,194,130,7,36,188,0,2,
    0,182,130,19,32,176,130,11,38,170,0,1,255,246,255,201,130,9,33,94,4,131,59,34,5,4,79,130,11,34,0,255,132,130,5,34,5,2,85,131,5,33,255,180,130,5,32,98,131,29,35,255,255,5,106,130,47,
    34,255,0,18,130,5,34,254,0,24,130,5,34,251,255,192,130,29,34,1,255,185,132,11,32,184,130,5,33,250,255,130,5,41,0,6,255,189,0,1,253,211,4,78,130,5,32,200,132,5,32,191,132,5,32,189,131,
    5,130,127,130,190,33,255,255,130,155,34,1,253,152,131,5,39,255,247,3,51,0,1,253,155,132,29,32,118,132,5,32,5,131,5,33,255,255,132,11,32,0,132,5,32,3,132,65,32,135,132,11,32,1,132,11,
    32,236,132,5,32,32,131,5,33,2,75,132,11,32,169,132,29,130,82,35,0,1,2,80,132,23,32,87,130,5,34,2,0,5,118,33,26,34,37,0,5,130,145,8,78,142,255,228,1,152,5,182,0,3,0,15,0,22,64,10,1,
    1,7,7,13,11,114,2,2,114,0,43,43,50,17,51,124,47,48,49,65,35,3,51,3,52,54,51,50,22,21,20,6,35,34,38,1,92,143,47,237,253,77,56,55,78,78,55,56,77,1,170,4,12,250,188,76,63,63,76,74,68,
    68,0,130,81,36,135,3,166,2,207,132,81,40,7,0,16,182,5,1,128,4,3,131,77,35,50,26,205,50,130,75,35,3,35,3,33,130,3,38,1,83,38,128,38,2,72,130,4,38,5,182,253,240,2,16,253,130,3,130,59,
    36,50,0,0,4,248,130,59,8,43,27,0,31,0,57,64,27,1,28,28,14,0,31,31,25,21,21,18,18,15,4,8,8,11,11,14,14,10,23,19,2,6,10,8,0,63,51,63,51,18,57,47,51,17,130,1,32,206,130,169,33,17,51,132,
    4,33,17,51,131,100,33,33,21,130,100,32,19,132,3,35,53,33,19,33,130,3,35,51,3,33,19,130,3,33,21,1,130,14,8,49,3,217,61,1,21,254,207,83,156,83,254,235,81,153,77,255,0,1,29,62,254,240,
    1,42,82,157,81,1,24,81,153,82,1,3,252,242,1,22,61,254,235,3,117,254,199,146,254,86,1,170,131,3,39,146,1,57,146,1,175,254,81,131,3,8,48,146,254,199,1,57,0,3,0,117,255,137,4,34,6,18,
    0,36,0,44,0,53,0,40,64,20,27,46,46,40,44,9,28,45,13,6,1,20,18,17,37,8,8,35,0,1,0,47,205,51,130,164,130,5,34,18,23,57,130,8,53,48,49,69,53,38,38,39,53,22,22,23,17,46,2,53,52,54,54,55,
    53,51,21,130,13,32,7,130,21,44,17,30,2,21,20,6,7,21,17,54,54,53,52,130,15,34,39,17,14,130,16,8,153,22,22,2,6,119,209,71,73,221,105,138,177,86,100,181,120,126,109,182,81,63,71,160,78,
    128,186,100,214,200,110,107,43,95,79,126,66,91,46,40,90,119,208,2,38,32,187,34,53,3,1,151,44,100,139,99,102,146,84,7,168,165,4,42,34,163,29,38,6,254,111,40,90,134,107,147,185,19,215,
    1,129,12,86,71,49,65,49,23,242,1,96,5,41,66,46,51,70,51,0,5,0,92,255,237,6,103,5,203,0,11,0,23,0,27,0,39,0,51,0,34,64,17,40,28,46,34,13,114,26,12,18,6,0,27,27,12,0,5,114,0,43,50,50,
    47,16,204,50,63,43,50,204,50,48,49,65,66,20,7,130,198,39,23,34,6,21,20,22,51,50,131,184,36,37,1,35,1,19,150,27,8,78,1,145,156,161,155,162,150,159,151,159,73,69,69,73,75,73,72,3,109,
    252,213,169,3,43,137,154,162,155,161,151,158,150,159,73,68,68,73,75,74,73,5,203,239,218,218,243,243,218,218,239,141,158,158,159,160,160,159,157,159,120,250,74,5,182,253,204,240,217,
    217,243,243,217,217,240,143,157,158,158,160,130,1,8,45,157,0,0,3,0,104,255,236,5,215,5,205,0,37,0,48,0,60,0,43,64,25,7,30,55,3,0,18,48,15,8,38,5,11,11,0,45,22,11,114,16,10,114,49,0,
    3,131,195,42,43,43,50,17,57,47,23,57,18,23,57,132,196,42,22,21,20,6,7,1,54,54,55,51,6,130,7,39,33,39,14,2,35,34,38,38,130,212,33,54,55,65,162,5,34,19,14,2,133,220,33,55,1,132,230,33,
    23,54,131,230,8,122,2,114,111,163,91,169,128,1,119,52,67,24,205,32,105,80,1,39,254,249,165,62,144,173,107,147,214,116,74,138,96,48,79,46,96,173,24,68,98,51,151,124,118,172,62,254,191,
    81,108,75,67,116,107,99,5,205,75,141,99,133,183,74,254,148,65,165,94,128,232,91,254,223,161,54,82,45,95,180,128,106,151,117,54,54,107,119,71,101,144,78,252,225,41,81,100,69,110,131,
    76,55,4,14,84,82,69,120,71,64,120,81,74,87,0,0,1,0,135,3,166,1,83,5,182,130,241,36,10,179,1,3,2,130,205,32,205,67,87,5,33,1,83,67,78,8,131,39,36,82,254,188,2,53,130,39,37,16,0,10,179,
    13,4,131,39,38,47,48,49,83,52,18,18,130,227,130,203,8,43,18,18,23,35,38,2,2,82,65,134,102,182,144,146,65,129,94,180,102,134,65,2,49,169,1,63,1,35,122,190,254,48,245,159,254,200,254,
    224,128,120,1,28,1,59,130,117,32,62,130,77,32,33,130,77,32,17,131,77,37,2,114,5,0,47,43,130,117,8,57,20,2,2,7,35,54,18,18,53,52,2,2,39,51,22,18,18,2,33,65,133,103,180,95,128,65,66,
    128,96,182,103,133,65,2,51,167,254,196,254,227,119,127,1,34,1,55,160,163,1,60,1,36,127,123,254,222,254,193,131,161,8,35,83,2,122,4,15,6,20,0,14,0,27,64,16,5,9,7,4,10,11,3,13,1,2,12,
    11,6,8,128,0,0,47,26,205,50,23,57,130,100,8,55,3,37,23,5,19,7,3,3,39,19,37,55,5,3,2,148,38,1,133,28,254,143,239,178,175,159,185,237,254,146,30,1,127,39,6,20,254,122,113,195,35,254,
    194,97,1,89,254,168,96,1,62,36,194,113,1,134,130,181,54,99,0,229,4,44,4,193,0,11,0,14,180,10,9,9,5,6,0,47,51,51,17,51,130,84,8,32,33,21,33,17,35,17,33,53,33,17,51,2,151,1,149,254,107,
    158,254,106,1,150,158,3,32,156,254,97,1,159,156,1,161,130,61,46,78,254,248,1,142,0,238,0,10,0,12,179,5,128,1,130,146,39,50,26,205,48,49,101,23,14,130,245,56,62,2,55,1,129,13,18,55,
    66,33,148,21,39,33,10,238,23,73,166,166,74,80,178,172,72,131,217,43,77,1,207,2,71,2,125,0,3,0,8,177,132,55,44,48,49,83,53,33,21,77,1,250,1,207,174,174,131,33,38,142,255,228,1,152,0,
    253,130,153,43,10,179,3,9,11,114,0,43,50,48,49,119,69,101,10,41,142,77,55,56,78,78,56,55,77,114,69,91,8,40,1,0,19,0,0,2,244,5,182,130,87,41,11,180,3,2,114,1,8,0,63,43,130,204,45,1,
    35,1,2,244,253,224,193,2,33,5,182,250,74,130,31,54,2,0,96,255,236,4,50,5,205,0,16,0,32,0,16,183,29,13,5,114,21,5,13,131,101,131,103,8,99,65,20,2,6,6,35,34,38,2,53,52,18,54,51,50,22,
    18,5,20,18,22,51,50,54,18,53,52,2,38,35,34,6,2,4,50,54,117,187,132,166,216,106,96,215,177,168,217,105,252,248,56,126,104,104,127,56,56,125,106,106,125,55,2,221,178,254,232,194,101,
    178,1,81,238,234,1,81,181,179,254,175,236,195,254,252,129,128,1,4,196,192,1,3,131,131,254,253,0,1,0,169,130,169,32,239,130,137,45,13,0,21,64,10,11,10,10,6,12,4,114,0,12,130,128,8,47,
    43,50,50,47,51,48,49,97,35,17,52,54,54,55,6,6,7,7,39,1,51,2,239,199,1,4,2,26,57,37,167,103,1,159,167,3,221,53,89,81,38,27,49,31,135,132,1,66,130,75,53,96,0,0,4,46,5,203,0,29,0,23,64,
    11,10,18,5,114,27,2,28,28,1,131,76,35,50,17,51,51,131,208,36,97,33,53,1,62,130,205,131,188,35,7,39,62,2,130,211,34,22,21,20,130,92,8,57,1,21,33,4,46,252,50,1,129,110,145,74,134,111,
    100,161,86,108,59,138,164,100,137,199,108,93,170,116,254,227,2,207,158,1,135,111,167,157,93,114,120,73,68,134,50,81,48,96,176,118,117,199,196,111,254,230,9,0,130,119,36,89,255,236,
    4,36,130,119,49,46,0,31,64,15,5,4,28,28,27,27,12,36,44,5,114,19,12,65,73,6,41,17,57,47,51,18,57,57,48,49,65,131,107,33,21,22,132,115,37,35,34,38,39,53,22,69,7,6,41,38,35,35,53,51,50,
    54,54,53,52,131,154,36,6,7,39,54,54,130,155,8,87,3,248,78,138,91,174,177,120,249,195,117,200,90,91,212,98,191,167,90,177,132,137,139,123,159,78,135,123,76,125,107,50,99,81,231,149,
    225,234,4,101,99,145,92,19,7,22,178,146,127,198,112,38,42,183,46,51,148,130,86,109,52,166,65,118,79,103,113,33,54,34,139,62,88,198,0,2,0,42,0,0,4,109,5,188,0,10,0,22,131,167,44,6,22,
    9,9,5,1,1,2,18,7,4,114,2,65,112,5,131,166,35,51,17,51,51,130,167,38,35,17,35,17,33,53,1,130,13,8,86,33,17,52,62,2,55,35,6,6,7,1,4,109,208,197,253,82,2,170,201,208,254,107,2,4,4,1,8,
    19,47,26,254,107,1,74,254,182,1,74,159,3,211,252,59,1,164,50,91,82,71,29,40,86,37,253,188,0,1,0,124,255,236,4,35,5,182,0,33,0,35,64,17,26,25,25,22,22,31,0,0,8,30,27,4,114,15,8,65,25,
    10,130,100,32,17,131,115,32,50,65,23,15,65,13,6,8,67,7,39,19,33,21,33,3,54,54,2,64,145,217,121,130,248,176,114,198,69,74,209,97,106,157,86,170,182,61,142,46,94,56,2,232,253,198,34,
    37,110,3,136,101,194,139,152,220,118,40,40,185,43,53,66,134,103,136,148,21,11,57,2,189,179,254,109,8,16,130,255,32,105,130,141,38,56,5,202,0,34,0,49,131,255,45,18,17,41,41,22,22,6,
    35,30,13,114,13,6,5,66,241,5,72,39,5,130,139,48,83,52,62,3,51,50,22,23,21,38,38,35,34,6,2,7,51,66,42,9,37,35,34,46,2,1,50,134,154,8,108,6,21,20,30,2,105,37,89,156,234,165,44,107,35,
    38,94,47,185,210,90,7,11,30,94,133,89,128,190,104,114,211,146,108,184,136,76,1,245,125,151,134,135,92,137,75,36,74,112,2,112,131,250,218,165,94,8,10,169,12,12,151,254,253,163,49,80,
    47,105,200,143,152,222,120,81,161,241,254,196,161,165,135,157,78,117,61,63,127,107,65,0,1,0,84,0,0,4,53,5,182,0,6,0,19,64,9,5,2,2,3,67,24,8,32,17,130,161,58,97,1,33,53,33,21,1,1,18,
    2,76,252,246,3,225,253,183,5,3,179,144,250,218,0,3,0,95,130,231,55,49,5,203,0,31,0,46,0,60,0,26,64,14,43,24,8,54,4,0,35,16,13,114,47,71,152,5,34,43,50,17,70,206,10,39,6,7,30,2,21,20,
    6,6,70,200,14,32,3,71,162,7,36,38,39,39,14,2,70,204,5,8,121,22,23,62,2,53,52,38,2,73,126,201,115,75,127,78,90,148,88,121,218,146,158,220,115,84,138,82,71,117,69,117,202,172,147,148,
    142,153,73,129,84,34,85,120,65,1,38,108,137,67,114,71,68,109,63,137,5,203,80,155,115,88,133,99,39,42,108,146,100,122,179,97,93,175,123,101,149,107,38,40,102,137,90,112,155,81,251,172,
    104,134,133,109,69,104,83,34,13,36,87,111,3,108,104,97,70,97,71,30,30,71,99,68,98,103,0,0,2,0,94,255,235,4,46,65,183,9,33,17,18,65,183,6,35,5,114,13,6,66,69,10,66,183,5,34,20,14,3,
    67,87,9,34,18,55,35,71,159,9,34,51,50,30,134,197,67,103,5,8,86,46,2,4,46,37,89,156,236,164,43,112,36,38,98,47,186,210,91,6,12,30,92,134,93,126,188,104,116,212,144,109,184,136,75,254,
    10,123,152,131,136,94,137,75,36,74,112,3,71,131,251,219,165,94,10,10,170,13,14,150,1,3,164,48,80,48,105,200,142,153,223,120,81,161,241,1,59,161,164,136,157,77,118,62,62,127,107,65,
    131,177,49,142,255,228,1,152,4,102,0,11,0,23,0,16,183,21,15,7,114,69,185,6,69,187,15,32,17,69,199,19,69,207,15,48,3,178,77,63,63,77,74,66,66,0,2,0,65,254,248,1,149,130,89,52,10,0,22,
    0,18,183,20,14,7,114,1,5,128,10,0,47,26,205,57,43,50,70,113,10,75,134,12,63,116,14,18,56,65,33,149,20,40,34,10,30,77,55,57,77,77,57,55,77,238,23,72,167,166,74,80,177,173,72,2,236,135,
    94,43,0,1,0,99,0,233,4,45,4,226,0,6,130,93,46,2,5,1,3,4,3,6,0,0,47,50,206,50,23,57,130,93,58,1,53,1,21,1,1,4,45,252,54,3,202,253,11,2,245,233,1,173,107,1,225,171,254,156,254,192,130,
    155,53,108,1,185,4,36,3,233,0,3,0,7,0,12,179,1,0,4,5,0,47,51,206,130,149,36,83,53,33,21,1,130,3,47,108,3,184,252,72,3,184,3,77,156,156,254,108,156,156,0,142,111,37,5,1,4,3,2,3,130,
    122,131,53,131,111,33,83,1,132,112,52,99,2,247,253,9,3,202,252,54,1,147,1,63,1,101,171,254,31,107,254,83,130,111,56,25,255,228,3,85,5,203,0,31,0,43,0,23,64,11,31,31,35,35,41,11,114,
    12,19,3,67,203,5,43,17,51,47,48,49,65,53,52,54,54,55,62,69,241,8,32,54,76,157,6,37,6,7,14,2,21,21,65,37,12,8,63,29,32,77,68,74,87,39,125,113,96,157,75,71,86,203,123,198,218,58,109,
    77,64,69,25,207,75,58,54,78,78,54,58,75,1,170,57,77,113,99,54,60,86,86,60,94,100,51,37,155,46,58,194,165,91,129,108,60,53,79,81,57,40,254,200,71,125,8,8,50,2,0,114,255,74,6,187,5,181,
    0,65,0,79,0,41,64,19,73,76,76,22,19,37,62,3,114,9,69,69,29,5,5,12,128,46,53,0,47,51,26,204,50,47,51,50,17,51,43,50,204,50,130,6,130,171,41,20,14,2,35,34,38,39,35,6,6,130,6,131,184,
    38,51,50,22,23,3,6,6,74,79,5,50,54,53,52,2,36,35,34,4,6,2,21,20,18,4,51,50,54,55,21,131,40,32,36,130,219,41,18,54,36,51,50,4,18,1,20,22,131,22,8,153,55,38,38,35,34,6,6,6,187,43,89,
    133,91,88,112,14,10,39,145,104,163,176,110,200,137,91,177,52,20,1,2,70,48,62,84,44,150,254,248,170,172,254,248,182,94,142,1,19,198,121,234,94,91,224,132,244,254,165,184,120,226,1,66,
    201,216,1,78,190,251,243,102,91,112,103,8,12,27,77,42,102,124,56,2,219,95,182,146,87,99,70,72,97,210,174,135,208,119,31,19,254,105,33,40,10,102,70,102,171,104,186,1,7,138,108,197,254,
    243,162,199,254,233,145,54,36,146,38,48,181,1,84,238,191,1,67,238,132,177,254,185,254,156,128,118,168,138,251,8,11,95,150,0,0,2,0,130,0,8,109,5,43,5,188,0,7,0,18,0,27,64,13,13,3,18,
    2,2,3,5,2,114,7,3,8,114,0,43,50,43,17,57,47,51,17,57,48,49,97,3,33,3,35,1,51,1,1,3,46,2,39,14,2,7,3,4,84,162,253,195,160,213,2,46,210,2,43,254,78,154,7,29,29,9,10,26,25,8,156,1,179,
    254,77,5,188,250,68,2,101,1,175,22,88,94,31,41,91,79,24,254,81,0,3,0,196,0,0,4,202,5,182,131,103,51,0,37,0,31,64,15,9,8,19,19,28,28,0,29,18,8,114,27,0,2,69,237,9,33,18,57,130,111,41,
    83,33,32,4,21,20,6,6,7,21,69,9,6,34,33,19,51,69,225,5,35,35,17,17,33,132,9,8,66,38,35,196,1,168,1,23,1,26,62,118,86,90,140,81,126,231,159,253,254,205,255,177,136,162,176,230,1,23,181,
    149,67,150,127,5,182,164,199,85,134,88,16,10,15,78,143,112,134,187,97,3,79,116,111,115,101,253,156,254,7,142,120,76,109,58,0,0,71,15,5,46,205,5,203,0,31,0,16,183,0,25,3,114,9,16,9,
    133,137,38,48,49,65,34,14,2,21,73,92,5,65,215,14,8,77,22,23,7,38,38,3,52,113,179,124,66,105,212,161,96,175,87,84,179,120,224,254,215,146,92,178,1,3,169,110,209,88,76,73,167,5,26,79,
    151,212,133,177,254,254,140,35,28,176,32,31,186,1,81,229,166,1,19,201,109,47,42,171,33,50,0,2,0,196,0,0,5,90,5,182,0,10,0,20,130,121,37,16,6,2,114,17,5,65,113,5,32,50,130,121,36,20,
    2,4,35,33,130,230,42,4,18,7,52,38,38,35,35,17,51,32,130,46,8,40,182,254,167,246,254,111,1,190,224,1,70,178,215,122,236,171,225,189,1,27,1,26,2,233,247,254,181,167,5,182,163,254,193,
    241,188,244,119,251,164,1,28,130,215,130,95,33,3,249,130,95,46,11,0,25,64,12,6,9,9,1,5,2,2,114,10,1,134,98,38,17,57,47,51,48,49,97,130,98,32,21,134,3,54,3,249,252,203,3,53,253,152,
    2,67,253,189,2,104,5,182,176,254,77,174,254,12,0,134,73,32,248,130,73,36,9,0,23,64,11,135,73,132,72,33,43,50,134,71,34,35,17,33,133,71,36,1,144,204,3,52,130,68,34,66,253,190,131,66,
    33,16,175,65,97,5,37,5,52,5,203,0,33,131,137,32,33,130,246,37,20,13,3,114,28,5,65,102,6,133,65,34,65,33,17,67,56,8,37,36,51,50,22,23,7,130,245,33,34,6,65,126,8,8,65,17,33,3,36,2,16,
    115,244,152,226,254,202,161,178,1,84,240,120,221,93,74,78,189,101,168,241,129,108,230,181,91,129,54,254,187,3,12,253,47,39,40,180,1,80,236,227,1,82,186,46,41,173,35,49,142,254,254,
    177,173,254,254,144,20,14,1,156,134,201,33,5,47,65,19,7,40,8,3,3,5,11,6,2,114,1,65,118,7,137,203,59,17,35,17,51,17,33,17,51,5,47,205,253,47,205,205,2,209,205,2,163,253,93,5,182,253,
    158,2,98,134,71,33,1,145,130,71,47,3,0,12,181,1,2,114,0,8,114,0,43,43,48,49,115,130,53,45,196,205,5,182,250,74,0,1,255,95,254,116,1,139,130,35,43,17,0,12,180,13,2,114,7,0,0,47,50,130,
    35,32,81,74,96,8,33,54,53,130,46,63,20,6,6,53,80,28,32,74,42,56,92,54,206,97,178,254,116,14,11,173,9,12,44,108,96,5,153,250,113,149,193,93,134,109,33,5,1,130,73,43,14,0,26,64,14,3,
    2,8,14,4,5,13,139,183,40,18,23,57,48,49,97,35,1,7,132,182,58,54,54,55,1,51,1,5,1,239,254,21,150,205,205,51,105,52,1,163,235,253,204,2,175,128,253,209,130,192,39,54,60,118,60,1,220,
    253,131,134,87,33,4,12,130,87,35,5,0,14,182,130,197,32,3,132,198,131,163,131,199,37,33,21,196,205,2,123,130,203,33,252,178,134,43,33,6,132,130,43,49,23,0,28,64,15,11,12,21,1,4,8,14,
    10,2,114,23,16,8,131,251,35,50,50,43,50,133,133,36,1,35,30,2,21,130,135,32,33,130,131,32,33,130,7,32,52,130,142,8,50,35,1,3,65,254,56,8,3,7,5,188,1,39,1,177,7,1,188,1,37,198,4,7,3,
    9,254,45,4,233,41,129,151,73,252,161,5,182,251,89,4,167,250,74,3,107,66,143,129,42,251,25,66,117,5,33,5,98,130,113,45,19,0,23,64,11,2,12,9,19,11,2,114,1,9,132,109,130,108,32,57,133,
    242,130,109,32,23,130,101,8,34,51,1,51,46,2,53,17,51,5,98,246,253,11,8,3,8,5,1,188,244,2,242,7,2,6,5,190,4,174,54,129,141,72,252,222,130,97,60,88,44,130,145,65,3,40,0,2,0,124,255,236,
    5,199,5,205,0,17,0,32,0,16,183,29,14,3,114,22,66,142,7,33,48,49,77,95,7,40,38,2,53,52,18,36,51,50,4,77,96,9,8,72,16,2,35,34,6,2,5,199,86,169,253,169,172,255,168,83,148,1,47,230,222,
    1,44,152,251,140,99,205,159,161,204,96,220,238,160,206,100,2,221,169,254,235,199,108,108,200,1,22,169,225,1,82,187,186,254,175,229,178,254,253,141,141,1,3,178,1,14,1,49,139,254,255,
    0,67,255,5,33,4,119,130,223,34,12,0,22,131,225,40,15,9,9,11,14,12,2,114,11,131,225,67,87,7,40,65,32,4,21,20,14,2,35,35,130,227,8,49,5,35,17,51,50,54,54,53,52,38,2,87,1,27,1,5,60,135,
    220,160,167,205,1,131,182,145,127,172,88,170,5,182,226,208,94,169,129,75,253,207,5,182,172,253,212,57,126,103,136,134,131,101,34,124,254,164,132,233,58,22,0,37,0,25,64,12,34,19,3,114,
    4,7,27,27,5,10,9,114,0,43,204,51,18,57,57,43,68,110,5,41,6,7,1,33,1,34,6,35,34,38,154,247,43,93,185,138,1,90,254,230,254,233,12,22,12,154,255,41,176,254,226,201,49,254,143,1,74,2,65,
    4,28,60,2,0,196,0,0,4,237,5,182,0,15,0,24,0,29,64,14,8,18,18,12,12,14,17,15,2,114,10,14,68,167,10,32,18,81,229,9,130,165,34,35,1,33,65,13,8,8,39,53,52,38,2,87,185,241,118,82,133,76,
    1,153,235,254,156,254,243,205,1,134,185,199,173,160,170,5,182,91,185,142,112,154,98,28,253,116,2,86,253,130,14,8,37,174,253,249,135,132,137,115,0,0,1,0,102,255,236,4,7,5,203,0,47,0,
    28,64,16,16,0,20,44,40,25,6,4,36,29,3,114,12,4,68,147,6,33,18,23,78,22,6,33,35,34,83,235,5,65,121,6,35,38,39,46,3,72,3,7,68,160,5,8,89,6,21,20,22,22,23,30,2,4,7,129,234,161,81,147,
    126,51,83,218,116,101,137,70,68,147,119,83,136,99,54,119,214,141,116,201,91,65,85,172,92,85,117,61,64,138,111,125,175,92,1,137,130,185,98,17,32,24,194,34,59,55,100,70,70,95,80,45,30,
    77,100,135,88,121,172,91,46,40,170,35,44,50,92,63,71,94,76,43,48,108,152,131,169,50,24,0,0,4,93,5,182,0,7,0,19,64,9,7,3,3,4,2,114,69,119,7,79,133,5,49,33,53,33,21,33,2,162,206,254,
    68,4,69,254,69,5,5,177,177,130,223,36,182,255,236,5,43,130,53,40,19,0,16,183,19,9,2,114,14,66,219,10,44,17,20,6,4,35,32,0,53,17,51,17,20,22,132,213,8,37,17,5,43,127,255,0,194,254,237,
    254,223,206,185,183,126,160,77,5,182,252,78,154,242,140,1,39,245,3,174,252,90,185,188,91,167,115,3,166,131,141,32,0,130,141,32,225,130,87,32,14,131,141,46,9,2,14,3,2,114,2,8,114,0,
    43,43,50,18,57,80,199,5,8,32,51,1,30,2,23,62,2,55,1,4,225,253,250,213,253,250,212,1,69,18,32,28,9,9,27,34,17,1,68,5,182,250,74,130,91,42,84,49,109,108,45,45,108,111,50,3,169,130,171,
    36,23,0,0,7,99,130,83,48,41,0,27,64,14,8,23,36,3,15,41,30,16,2,114,2,15,65,255,6,77,20,6,40,1,35,1,46,3,39,14,3,7,130,8,40,51,19,30,3,23,62,3,55,19,130,109,130,8,44,2,55,19,7,99,254,
    126,217,254,245,11,22,21,130,147,8,34,13,18,22,11,254,251,216,254,128,209,223,11,21,18,14,6,5,16,19,22,12,250,205,1,3,12,23,19,15,6,7,21,28,14,223,131,137,45,3,162,37,86,85,68,18,18,
    67,84,87,39,252,95,130,151,57,137,44,91,91,85,38,39,89,92,90,41,3,117,252,134,42,92,92,84,36,50,116,124,60,3,118,131,251,32,5,130,251,32,200,130,167,48,11,0,26,64,14,2,5,11,8,4,1,10,
    6,2,114,1,4,134,167,32,18,69,131,6,130,159,34,1,51,1,130,2,8,33,4,200,234,254,131,254,126,218,1,230,254,59,227,1,97,1,96,219,254,58,2,109,253,147,2,248,2,190,253,199,2,57,253,64,130,
    253,32,0,130,85,32,154,130,85,45,8,0,23,64,12,6,3,0,3,4,2,7,2,114,132,83,32,43,69,80,5,32,65,130,74,59,17,35,17,1,51,2,78,1,112,220,254,26,205,254,25,224,2,250,2,188,252,130,253,200,
    2,47,3,135,131,155,32,71,130,69,32,80,130,69,46,9,0,25,64,12,7,4,4,5,2,114,2,8,8,1,132,153,42,17,51,43,50,17,51,48,49,97,33,53,78,155,5,53,33,4,80,251,247,2,253,253,28,3,220,253,2,
    3,18,146,4,114,178,146,251,142,130,139,36,160,254,188,2,109,130,69,43,7,0,14,181,5,2,2,114,6,1,0,47,130,60,34,48,49,65,72,32,6,48,2,109,254,51,1,205,254,238,1,18,254,188,6,250,155,
    250,61,131,123,36,19,0,0,2,245,71,9,6,32,3,66,157,7,43,48,49,83,1,35,1,211,2,34,193,253,223,65,245,5,130,95,32,51,130,95,32,1,134,95,33,0,7,130,233,36,114,0,43,50,47,130,154,32,87,
    130,95,32,53,130,3,32,51,130,88,43,238,1,206,254,50,168,5,195,155,249,6,0,130,51,40,72,2,30,4,76,5,192,0,6,131,147,35,4,0,1,17,130,51,34,205,50,57,131,95,32,51,130,97,8,38,1,72,1,183,
    108,1,225,171,254,151,254,187,2,30,3,162,252,94,2,208,253,48,0,1,255,252,254,201,3,119,255,72,0,3,0,8,177,1,2,130,199,131,197,42,53,33,3,119,252,133,3,123,254,201,127,131,91,58,82,
    4,217,2,18,6,33,0,12,0,18,183,11,4,0,128,15,6,1,6,0,47,93,26,205,57,57,130,45,8,69,30,2,23,21,35,46,3,39,53,1,60,26,74,81,33,135,37,90,90,74,22,6,33,46,113,106,38,25,29,80,89,81,29,
    20,0,0,2,0,92,255,236,3,231,4,97,0,29,0,40,0,35,64,18,7,37,37,11,30,19,19,0,11,11,114,4,10,114,23,0,7,130,174,34,50,43,43,80,192,5,81,77,6,35,21,17,35,39,79,0,8,36,36,37,55,53,52,82,
    246,5,35,54,54,1,7,76,85,7,8,71,53,2,81,204,202,144,39,8,46,100,131,94,100,156,89,1,2,1,6,190,115,103,85,156,72,64,78,199,1,60,154,191,155,106,87,134,173,4,97,181,195,253,23,160,60,
    80,40,72,145,112,168,174,8,8,62,129,106,49,34,146,40,54,253,182,7,7,113,101,88,82,151,148,130,161,8,32,171,255,236,4,131,6,20,0,22,0,36,0,37,64,20,22,0,114,21,10,114,18,19,31,31,15,
    11,114,5,4,23,23,8,132,163,41,17,51,51,43,50,17,51,51,43,43,130,250,58,17,20,6,7,51,54,54,51,50,18,17,20,2,6,35,34,38,39,35,7,35,17,1,34,6,6,7,88,33,8,8,95,1,116,7,3,10,45,165,130,
    201,242,111,201,136,129,160,45,15,38,149,1,240,109,129,56,1,128,168,138,143,142,6,20,254,130,66,126,34,72,101,254,225,254,230,186,255,0,130,95,66,141,6,20,253,168,86,174,132,15,196,
    208,212,197,200,202,0,1,0,109,255,236,3,165,4,98,0,29,0,16,183,15,8,7,114,23,0,11,114,0,43,50,43,50,48,49,69,34,87,189,5,74,7,9,32,6,130,121,32,22,77,94,6,8,51,2,102,152,228,125,134,
    237,153,90,157,53,60,56,129,58,106,139,70,69,135,101,89,142,60,58,140,20,122,251,192,200,255,122,36,27,163,22,34,93,180,132,128,177,92,40,31,177,32,32,0,0,2,131,109,37,4,68,6,20,0,
    23,65,7,5,48,17,10,114,16,0,114,11,10,31,31,6,7,114,19,20,24,24,133,124,65,7,11,37,69,34,2,17,16,18,130,130,34,22,23,51,130,142,33,17,51,65,184,5,34,39,50,54,65,179,6,130,146,8,61,
    2,40,202,241,245,203,85,128,93,31,12,5,10,201,159,33,9,30,93,129,40,163,135,1,129,172,140,142,142,20,1,30,1,25,1,28,1,35,46,76,48,31,112,42,1,163,249,236,153,48,79,46,164,188,185,31,
    197,211,219,194,192,207,135,151,34,39,4,98,130,151,45,31,0,25,64,12,27,6,6,0,9,16,11,114,24,66,59,6,35,50,18,57,47,83,133,7,34,21,33,22,65,2,7,82,21,5,8,112,18,54,23,34,6,7,33,46,2,
    2,91,144,206,110,253,20,3,176,159,105,164,88,83,167,116,160,241,134,122,222,149,122,148,13,2,30,1,56,113,4,98,123,224,152,114,177,189,40,39,169,38,35,128,251,184,182,1,3,138,156,159,
    148,91,138,78,0,1,0,33,0,0,3,40,6,31,0,24,0,27,64,14,6,5,1,1,23,6,114,19,12,1,114,3,10,114,0,43,43,50,43,50,17,51,57,48,49,65,33,17,35,17,35,53,130,248,33,54,54,65,148,9,8,43,21,21,
    33,2,180,254,241,201,187,187,86,161,115,69,115,42,52,34,84,47,87,83,1,15,3,179,252,77,3,179,97,61,76,139,169,78,24,15,154,11,19,112,113,77,130,102,8,38,0,25,254,20,4,63,4,99,0,47,0,
    63,0,75,0,45,64,22,34,12,64,64,32,6,57,57,41,41,0,26,23,23,70,19,7,114,48,0,15,66,4,5,130,119,85,159,5,35,198,50,17,57,131,129,38,34,38,53,52,54,55,38,137,5,131,136,45,22,23,33,21,
    7,22,22,21,20,6,35,38,39,6,66,36,5,130,159,45,21,20,4,37,50,54,54,53,52,38,38,35,35,34,132,23,33,19,50,84,28,6,8,153,21,20,22,1,227,221,237,130,117,44,63,69,69,87,106,101,190,136,30,
    64,58,18,1,124,196,29,36,229,202,47,48,43,44,39,74,53,192,180,192,254,208,254,233,129,173,88,57,112,82,182,70,104,58,143,176,111,110,112,110,107,113,114,254,20,162,145,103,143,25,20,
    83,52,61,89,42,36,167,112,119,168,88,5,10,5,119,30,38,104,62,168,195,1,7,25,59,38,29,36,17,152,146,185,200,147,52,96,66,60,63,22,44,83,61,83,88,3,101,117,109,117,118,121,116,107,117,
    0,0,1,0,171,0,0,4,91,6,20,0,26,0,27,64,14,26,0,114,15,25,10,114,4,5,19,19,9,67,137,9,67,133,8,84,224,6,36,17,35,17,52,38,132,200,130,9,8,42,1,116,6,4,12,35,103,131,73,129,178,92,199,
    116,120,114,136,58,201,6,20,254,84,51,98,33,58,77,39,84,177,141,253,50,2,179,132,133,91,176,128,253,207,130,101,34,2,0,157,130,116,57,134,5,239,0,3,0,15,0,16,183,4,10,3,6,114,2,10,
    114,0,43,43,206,50,48,49,65,130,80,91,233,11,8,34,1,116,201,102,48,69,69,48,49,67,67,4,78,251,178,4,78,1,161,56,63,63,57,57,63,63,56,0,0,2,255,139,254,20,1,131,75,44,16,0,28,0,19,64,
    9,20,26,11,6,114,7,65,178,6,131,78,32,83,76,220,8,32,53,76,219,5,81,204,11,46,50,51,85,31,33,64,40,67,84,201,67,143,5,67,49,133,102,51,254,20,15,10,161,10,10,74,100,4,230,251,18,100,
    150,82,7,100,63,56,132,113,65,41,6,32,89,130,195,54,18,0,32,64,19,18,0,114,15,14,4,5,11,8,6,10,13,13,17,10,114,10,6,131,195,36,50,17,18,23,57,65,46,8,76,250,5,32,1,77,9,5,8,40,1,115,
    7,3,7,23,77,30,1,96,232,254,70,1,217,238,254,141,133,200,6,20,252,244,46,117,49,30,98,34,1,120,254,39,253,139,1,247,118,254,127,130,97,38,1,0,171,0,0,1,117,130,9,40,3,0,12,181,2,0,
    114,1,10,77,209,5,39,97,35,17,51,1,117,202,202,135,35,62,6,226,4,98,0,39,0,40,64,23,28,29,36,37,4,19,19,33,9,0,7,114,33,7,114,26,6,114,14,5,25,131,53,37,50,50,43,43,43,50,130,254,32,
    23,67,184,5,32,21,65,189,7,133,8,65,199,8,33,51,23,86,191,5,8,67,23,51,54,54,5,112,183,187,199,107,106,150,133,200,48,94,72,103,123,55,201,159,29,11,33,101,124,69,124,170,40,13,53,
    187,4,98,191,210,253,47,2,184,130,130,181,175,253,168,2,184,87,115,58,89,174,126,253,201,4,78,152,57,76,39,90,92,93,89,66,87,8,130,157,46,21,0,27,64,14,15,6,114,5,14,10,114,18,17,9,
    131,159,32,0,131,143,40,51,43,50,43,48,49,65,50,22,140,144,132,133,40,2,207,191,205,199,116,120,171,137,131,119,38,35,108,133,4,98,192,212,66,70,5,41,202,192,253,206,4,78,155,58,78,
    39,69,67,6,32,110,130,97,43,17,0,32,0,16,183,30,14,7,114,22,5,69,179,8,37,65,20,14,2,35,34,93,110,5,38,51,50,22,22,5,20,22,74,95,5,33,52,38,131,242,8,54,4,110,71,135,190,120,112,187,
    136,74,124,231,160,152,230,128,252,205,65,135,107,105,136,64,64,135,108,158,147,2,41,136,213,147,77,77,147,213,136,181,255,133,134,254,181,127,183,98,98,183,127,126,180,96,211,130,
    117,36,171,254,22,4,131,130,117,56,24,0,40,0,37,64,20,18,6,114,17,14,114,11,12,34,34,7,11,114,21,20,25,25,0,70,193,17,44,50,18,17,20,6,6,35,34,38,38,39,35,22,132,239,132,230,32,23,
    70,194,6,135,151,8,68,2,201,201,241,111,201,135,85,128,91,31,13,4,9,202,165,27,10,31,91,130,42,106,127,59,2,56,130,110,94,125,61,138,4,98,254,227,254,229,188,255,131,45,75,43,39,103,
    41,254,62,6,56,158,49,81,48,166,83,164,124,32,132,181,95,102,186,123,187,213,130,161,32,108,130,161,32,67,130,161,71,99,7,50,14,114,21,6,114,19,18,31,31,15,7,114,4,5,23,23,8,11,114,
    65,129,7,71,99,8,35,52,54,55,35,131,162,47,2,17,52,18,54,51,50,22,23,51,55,51,17,1,50,54,70,92,10,8,63,3,122,5,5,11,45,166,131,198,241,111,202,134,129,163,47,8,26,163,254,19,108,129,
    60,2,133,168,142,139,139,254,22,1,213,40,98,38,74,101,1,30,1,27,186,1,0,131,101,73,154,249,200,2,121,82,165,124,35,200,207,219,192,193,209,67,65,5,51,3,58,4,98,0,21,0,25,64,13,15,6,
    114,14,10,114,18,17,7,7,65,50,9,66,17,5,81,108,5,8,49,14,2,21,17,35,17,51,23,51,62,2,2,189,30,70,25,21,24,63,27,64,116,88,50,202,162,24,9,34,97,127,4,98,7,5,187,6,8,48,91,131,84,253,
    185,4,78,199,60,100,59,130,97,36,101,255,236,3,130,130,97,48,42,0,26,64,14,14,18,39,22,4,4,32,25,7,114,11,4,132,243,32,43,74,196,6,65,141,5,91,88,10,36,39,46,2,53,52,132,247,77,70,
    5,77,69,6,8,72,3,130,105,200,140,116,168,68,73,192,92,129,115,46,115,107,105,149,78,229,188,100,177,81,65,72,152,79,103,109,51,120,102,101,147,80,1,52,107,147,74,35,33,178,35,54,82,
    70,41,63,63,41,41,82,116,92,143,151,40,36,155,31,41,66,58,45,60,57,39,38,82,118,0,130,153,42,36,255,236,2,206,5,72,0,24,0,29,131,153,41,13,21,21,16,15,18,6,114,0,7,134,153,40,205,51,
    17,51,18,57,48,49,101,85,180,7,8,55,38,38,53,17,35,53,55,55,51,21,33,21,33,17,20,22,2,43,43,88,32,34,113,59,90,146,87,153,160,71,124,1,63,254,193,89,143,15,12,154,15,21,62,150,133,
    2,110,93,74,238,250,155,253,149,93,92,130,107,54,161,255,236,4,83,4,78,0,23,0,27,64,14,23,13,6,114,3,4,18,18,8,11,68,70,6,34,50,17,51,75,72,5,72,65,5,33,35,34,72,78,5,32,20,66,135,
    5,8,40,17,4,83,161,28,11,34,108,134,75,127,177,91,202,115,118,115,135,59,4,78,251,178,151,57,76,38,84,177,140,2,209,253,75,133,132,91,175,127,2,53,130,103,36,0,0,0,4,35,130,103,41,
    13,0,21,64,10,7,6,0,12,1,130,208,33,10,114,77,130,5,130,203,8,44,97,1,51,19,22,22,23,51,54,54,55,19,51,1,1,163,254,93,213,235,23,45,8,7,10,48,22,234,214,254,92,4,78,253,111,65,154,
    50,51,154,64,2,145,251,178,130,79,40,23,0,2,6,70,4,79,0,42,131,183,43,21,34,6,3,14,41,29,15,6,114,42,14,131,83,34,50,43,50,65,190,5,42,101,3,46,3,39,35,14,3,7,3,35,130,95,37,30,2,23,
    51,62,3,130,95,32,19,132,9,32,2,130,9,55,1,4,46,168,11,25,24,19,5,8,5,18,23,25,12,176,222,254,206,205,151,15,28,20,130,15,8,64,15,18,21,9,186,218,179,13,28,22,4,8,4,22,28,16,153,202,
    254,204,2,2,64,39,94,94,81,25,25,81,95,96,39,253,195,4,77,253,191,58,124,110,39,26,77,85,80,30,2,98,253,159,44,109,106,38,34,108,127,60,2,65,251,179,0,130,165,32,32,130,245,32,44,130,
    245,48,11,0,28,64,15,9,6,0,3,4,1,8,8,11,10,114,5,131,253,32,43,130,166,34,17,18,23,130,252,32,65,77,120,5,34,1,35,1,130,2,8,34,168,254,138,229,1,15,1,15,227,254,137,1,139,229,254,223,
    254,221,227,2,52,2,26,254,106,1,150,253,230,253,204,1,175,254,81,130,89,36,2,254,19,4,37,130,89,49,29,0,26,64,14,6,29,28,13,4,0,24,17,15,114,12,0,6,72,133,5,132,87,33,83,51,132,233,
    65,83,5,88,13,5,94,33,5,8,59,54,55,55,2,217,234,18,31,23,7,7,12,44,26,224,216,254,36,39,111,152,104,48,73,26,22,63,34,63,91,65,22,50,4,78,253,133,49,93,87,42,51,146,75,2,122,251,22,
    105,151,81,11,7,160,5,8,49,90,60,130,131,215,36,74,0,0,3,126,130,125,77,183,8,38,6,114,2,8,8,1,10,68,45,5,77,183,15,52,3,126,252,204,2,68,253,224,3,2,253,197,2,73,128,3,50,156,142,
    252,219,77,87,6,59,196,5,182,0,37,0,29,64,13,28,29,10,10,9,9,0,20,19,2,114,37,0,0,47,50,43,50,18,94,229,8,41,46,2,53,17,52,38,38,35,53,62,131,8,45,54,54,51,21,14,2,21,17,20,6,7,21,
    22,22,130,7,8,67,22,22,23,2,196,137,182,89,57,111,81,81,111,57,93,182,133,69,98,52,110,111,113,108,51,98,70,254,188,1,67,138,105,1,52,69,85,39,163,1,38,84,70,1,54,105,136,67,158,2,
    37,80,65,254,212,102,122,19,12,19,122,102,254,207,65,79,36,1,130,213,63,1,227,254,24,2,130,6,19,0,3,0,8,177,0,2,0,47,47,48,49,65,51,17,35,1,227,159,159,6,19,248,5,130,177,36,71,254,
    188,2,216,135,177,39,10,9,28,28,29,29,0,18,130,177,130,62,140,177,32,83,134,169,35,55,53,38,38,132,186,34,39,53,30,131,177,38,22,22,51,21,34,6,6,130,179,8,53,6,6,71,69,97,52,110,110,
    111,109,51,98,69,137,181,89,57,112,81,81,112,57,93,181,254,188,160,2,36,79,65,1,46,103,122,18,12,19,123,101,1,47,66,79,37,1,158,1,66,137,105,254,204,70,130,197,39,39,84,69,254,201,
    104,137,68,130,141,8,33,99,2,71,4,45,3,93,0,25,0,29,64,12,20,19,19,3,10,23,128,7,6,6,16,23,0,47,51,51,47,51,26,16,205,96,243,5,39,65,38,38,35,34,6,7,53,75,1,5,75,138,11,8,85,2,41,72,
    100,47,57,127,51,51,124,74,61,118,90,74,99,45,59,125,50,48,124,75,60,118,2,136,32,24,68,52,170,53,54,26,39,31,25,68,52,168,52,57,25,0,0,2,0,142,254,139,1,152,4,93,0,3,0,15,0,19,183,
    0,0,7,7,13,7,114,2,0,47,43,50,17,51,125,47,48,49,83,51,19,35,19,20,90,111,6,39,51,50,22,201,144,48,239,254,103,177,7,45,2,150,251,245,5,69,76,63,63,76,74,67,67,0,130,191,49,175,255,
    236,3,234,5,203,0,35,0,20,183,26,24,16,23,34,8,102,180,6,38,47,51,205,51,48,49,65,102,155,6,130,187,77,68,11,36,7,21,35,53,46,102,190,6,8,59,2,213,82,143,52,59,59,131,57,106,141,69,
    70,136,102,89,137,67,58,121,78,141,126,184,98,101,184,123,5,203,163,3,35,24,163,23,32,91,180,136,134,175,86,35,29,172,28,33,3,196,202,18,127,235,179,185,240,128,18,171,131,129,38,72,
    0,0,4,78,5,202,130,129,49,37,64,18,23,19,19,22,30,11,11,29,14,14,0,22,12,114,7,94,219,6,37,18,57,47,51,51,17,96,83,9,70,113,5,42,6,21,17,33,21,33,21,20,6,6,7,130,7,8,81,53,62,2,53,
    53,35,53,51,17,52,54,54,2,178,111,181,73,68,64,146,78,107,119,1,150,254,106,41,66,38,3,6,251,250,60,90,51,194,194,105,188,5,202,48,34,158,29,44,115,130,254,248,150,202,81,107,66,21,
    179,168,17,65,114,88,204,150,1,22,131,176,91,0,0,2,0,119,1,4,4,26,4,163,130,145,8,46,51,0,74,64,35,33,30,30,40,21,24,24,40,40,27,22,23,23,32,31,27,15,12,12,48,3,6,6,48,48,9,14,13,13,
    4,5,9,9,27,12,0,63,51,47,206,50,50,133,162,133,5,32,16,142,15,40,48,49,83,52,54,55,39,55,23,66,1,5,33,55,23,76,121,5,47,7,23,7,39,6,6,35,34,38,39,7,39,55,38,38,55,72,247,13,8,72,6,
    184,38,32,135,106,135,48,115,63,61,111,49,136,107,134,31,39,36,34,131,104,136,47,113,61,64,116,46,135,105,134,32,38,148,68,115,70,72,116,68,68,116,71,71,115,68,2,211,61,114,48,137,
    104,132,33,36,36,33,132,103,137,47,115,62,62,115,48,135,103,132,31,37,131,15,38,136,48,114,61,71,115,67,132,48,60,69,69,116,0,0,1,0,25,0,0,4,118,5,182,0,22,0,44,64,21,0,20,20,17,10,
    7,7,13,16,130,220,39,17,17,12,1,21,4,114,12,130,217,68,4,5,33,17,51,136,207,40,48,49,65,1,51,1,51,21,33,131,1,36,17,35,17,33,53,131,1,130,17,8,44,2,72,1,93,209,254,102,246,254,215,
    1,41,254,215,194,254,214,1,42,254,214,242,254,106,212,3,3,2,179,253,3,137,165,136,254,253,1,3,136,165,137,2,253,0,2,67,233,10,44,7,0,12,179,4,6,3,0,0,47,50,47,51,67,239,5,32,17,67,
    243,6,8,44,159,159,6,19,252,238,254,42,252,237,0,2,0,118,255,244,3,139,6,30,0,54,0,69,0,26,64,15,47,33,51,30,60,67,3,23,8,44,36,16,9,1,114,0,43,130,61,33,23,57,65,103,5,33,38,38,72,
    32,19,34,21,20,6,65,117,5,38,35,34,38,39,53,30,2,92,17,5,41,38,39,46,2,55,20,22,22,23,23,77,244,5,8,143,39,6,6,135,97,63,72,82,211,189,110,163,77,58,69,141,92,114,99,50,114,95,101,
    149,80,88,61,70,77,233,207,112,169,66,47,118,126,61,144,110,40,110,104,104,150,81,163,56,123,99,44,46,76,54,131,118,55,86,3,39,98,125,33,39,112,83,123,146,41,31,144,29,42,62,58,39,
    60,56,35,38,89,118,83,103,131,35,38,108,78,142,160,38,32,160,22,41,24,86,58,40,59,59,39,40,85,120,109,50,77,68,38,16,27,89,66,50,81,71,37,16,90,0,0,2,1,46,5,10,3,130,5,223,0,11,0,23,
    0,14,180,15,21,21,3,9,102,183,8,33,52,54,107,135,8,32,37,138,11,61,1,46,60,43,43,61,61,43,43,60,1,133,59,43,43,62,62,43,43,59,5,117,55,51,51,55,54,53,53,54,134,7,8,39,0,0,3,0,100,255,
    236,6,68,5,203,0,19,0,46,0,66,0,27,64,13,34,27,57,10,3,114,40,20,128,47,0,9,114,0,43,50,26,204,50,105,218,5,38,69,34,36,38,2,53,52,94,132,5,41,22,18,21,20,2,6,4,3,34,38,86,188,13,33,
    21,20,80,80,7,34,7,50,62,130,45,8,79,46,2,35,34,14,2,21,20,30,2,3,84,163,254,237,203,111,112,204,1,19,161,157,1,17,206,116,112,203,254,238,131,204,205,97,187,133,65,131,57,57,50,98,
    46,126,138,125,136,49,116,52,49,104,103,131,230,174,99,95,170,231,138,138,232,171,94,93,171,232,20,112,202,1,19,162,161,1,19,203,113,131,52,8,74,163,162,254,237,202,112,1,35,251,209,
    134,207,118,32,29,122,26,27,176,153,159,171,26,21,127,22,28,182,95,173,234,140,133,232,178,100,95,173,235,140,134,233,176,99,0,2,0,63,3,10,2,129,5,199,0,28,0,39,0,31,64,14,6,36,36,
    18,29,0,5,5,9,192,22,0,3,134,224,33,47,17,130,128,83,77,10,36,6,6,35,34,38,132,217,32,55,81,153,6,36,7,39,54,54,19,83,77,9,8,66,1,117,134,134,102,25,40,117,76,66,99,53,73,147,110,108,
    79,63,52,106,49,49,56,137,205,98,116,81,58,48,97,92,5,199,119,121,254,64,93,47,59,45,93,69,71,93,49,4,4,40,63,55,30,25,105,28,37,254,143,4,5,63,53,49,46,96,78,130,153,8,39,80,0,115,
    3,231,3,213,0,6,0,13,0,36,64,18,11,12,12,5,9,8,8,6,13,3,10,0,7,6,2,1,4,5,0,47,51,204,50,23,57,68,67,6,41,48,49,83,1,23,1,1,7,1,37,133,6,8,35,80,1,92,144,254,227,1,29,144,254,164,1,
    169,1,95,143,254,228,1,28,143,254,161,2,48,1,165,81,254,160,254,161,82,1,163,26,137,10,49,0,1,0,99,1,3,4,41,3,32,0,5,0,14,180,1,1,4,132,95,97,62,5,51,17,35,17,33,53,4,41,155,252,213,
    3,32,253,227,1,129,156,255,255,0,104,179,6,39,6,6,0,16,0,0,0,4,66,41,8,58,13,0,22,0,42,0,62,0,35,64,18,12,8,14,3,0,22,1,53,33,3,114,10,0,128,43,23,66,48,11,100,122,5,45,17,33,50,22,
    21,20,6,7,19,35,3,35,17,17,67,91,5,34,35,35,19,66,74,18,32,39,66,47,14,50,2,60,1,8,164,156,99,64,238,170,204,135,109,82,91,84,91,107,130,66,64,18,32,163,66,42,14,54,1,28,3,128,134,
    132,97,113,25,254,117,1,97,254,159,1,220,81,69,76,67,251,207,66,64,19,32,109,66,43,16,47,1,255,250,6,20,4,6,6,166,0,3,0,8,177,2,1,85,229,8,63,4,6,251,244,4,12,6,20,146,0,0,2,0,108,
    3,75,3,1,5,203,0,15,0,27,0,16,182,16,0,192,22,8,66,71,7,34,48,49,65,84,103,9,131,247,33,6,39,82,36,11,8,42,182,98,149,83,82,148,100,98,149,84,84,149,96,89,97,99,87,92,96,95,3,75,80,
    145,94,95,144,82,81,145,95,94,145,80,133,102,84,87,103,103,87,84,102,130,103,38,99,0,0,4,46,4,212,130,139,130,105,52,64,11,14,4,13,13,9,7,10,10,1,1,0,0,47,50,17,51,47,51,51,103,195,
    5,36,115,53,33,21,1,106,190,10,36,99,3,203,254,105,106,193,9,35,156,156,3,51,106,195,10,55,51,3,84,2,134,6,212,0,26,0,18,183,2,25,25,0,120,10,17,119,0,63,51,228,130,81,130,189,33,33,
    53,99,68,20,8,76,7,33,2,134,253,173,234,77,83,32,70,60,56,99,53,81,62,145,92,130,152,52,105,78,154,1,140,3,84,120,229,75,96,78,44,59,63,45,43,105,52,63,129,115,66,112,115,73,146,0,
    0,1,0,43,3,69,2,151,6,211,0,41,0,27,64,12,6,7,29,29,26,26,20,13,120,35,0,130,110,34,50,228,50,74,67,8,66,31,5,105,43,5,32,35,81,236,9,36,52,38,35,35,53,66,47,6,8,69,34,6,7,39,54,54,
    1,93,138,151,87,69,86,95,172,180,75,131,62,67,137,64,100,94,106,105,118,113,104,87,76,61,59,105,55,76,62,142,6,211,126,100,80,105,21,8,17,108,80,119,146,28,31,139,34,40,79,69,67,69,
    119,78,61,58,61,42,36,102,45,56,87,161,15,34,1,8,12,87,161,14,8,33,21,14,3,7,35,53,62,2,55,2,18,22,75,90,90,37,134,32,79,76,25,6,33,20,29,81,89,80,29,25,38,106,113,46,131,67,51,171,
    254,20,4,92,4,78,0,29,0,33,64,17,17,12,11,4,3,24,24,77,152,5,32,20,75,228,6,35,43,50,17,51,130,0,35,47,48,49,65,77,159,8,33,39,35,94,197,5,91,36,9,8,49,4,92,159,30,10,33,88,117,75,
    80,116,38,7,3,4,2,201,201,118,120,116,133,57,4,78,251,178,152,55,77,40,52,46,24,75,91,49,254,181,6,58,253,73,129,134,91,176,126,2,53,131,125,58,120,254,252,4,106,6,20,0,18,0,18,182,
    6,9,9,3,17,5,0,0,47,50,47,51,57,47,51,105,206,6,33,35,17,68,190,9,8,79,51,33,4,106,124,203,125,31,76,39,125,185,102,111,199,133,2,55,254,252,6,150,249,106,3,65,9,9,96,218,181,191,221,
    94,0,255,255,0,142,2,62,1,152,3,87,6,7,0,17,0,0,2,90,0,1,0,12,254,20,1,169,0,0,0,22,0,16,181,19,16,10,3,192,18,0,47,26,204,50,57,92,195,5,65,166,12,8,44,39,55,51,7,30,2,1,169,146,153,
    34,60,20,21,66,30,64,71,99,85,86,131,47,48,79,48,254,235,101,114,8,6,120,4,8,39,48,51,52,10,170,99,11,44,71,131,193,57,80,3,84,1,251,6,193,0,13,0,18,64,9,11,10,12,7,4,2,120,13,119,
    0,63,228,23,131,93,95,224,6,8,34,6,6,7,7,39,37,1,251,161,2,3,2,18,51,26,101,77,1,20,6,193,252,147,2,24,34,69,65,26,19,43,18,73,102,198,130,170,52,0,65,3,10,2,198,5,200,0,12,0,24,0,
    16,182,16,3,3,22,9,3,112,146,5,35,51,48,49,65,74,240,10,35,22,5,20,22,66,77,8,8,46,2,198,176,149,141,179,173,151,97,143,81,254,8,85,95,94,86,85,94,94,87,4,106,166,186,182,170,168,182,
    82,156,112,115,119,119,115,115,117,115,0,2,0,78,0,115,3,229,69,161,6,49,26,64,15,2,9,10,13,7,0,6,4,3,11,10,5,12,1,8,69,158,6,130,105,38,1,39,1,1,55,1,5,133,6,48,3,229,254,161,142,1,
    28,254,228,142,1,95,254,85,254,163,143,131,11,47,143,1,93,2,22,254,93,82,1,96,1,95,81,254,92,27,137,10,8,49,0,0,4,0,61,0,0,6,7,5,182,0,3,0,17,0,28,0,37,0,54,64,28,21,30,30,24,24,19,
    34,27,27,22,18,3,3,18,12,114,13,12,14,9,4,4,15,1,1,15,4,131,225,38,47,16,204,23,57,43,50,130,6,34,57,47,57,75,8,5,38,48,49,97,1,51,1,3,109,71,9,8,64,37,51,17,1,53,33,53,1,51,17,51,
    21,35,21,1,51,53,52,54,55,6,6,7,1,21,3,93,168,252,162,117,2,3,2,19,50,26,101,77,1,19,151,2,251,254,115,1,144,163,127,127,254,96,250,2,3,10,58,22,5,182,250,74,2,74,65,110,7,57,19,72,
    102,197,252,148,253,182,192,112,2,67,253,204,127,192,1,63,196,44,106,49,26,96,32,0,130,177,32,44,130,187,32,9,132,187,56,18,0,45,0,38,64,20,20,43,43,28,35,19,12,114,14,13,15,10,4,4,
    16,1,1,16,138,177,33,204,50,130,141,34,48,49,115,133,169,33,62,2,109,241,5,132,169,68,122,22,44,21,236,3,94,167,252,162,93,1,3,3,1,20,132,175,56,20,151,1,224,233,77,83,33,72,59,56,
    99,52,81,61,146,91,131,151,53,104,78,153,1,139,135,181,38,25,52,51,46,20,19,43,135,182,68,161,19,36,140,0,4,0,47,130,185,34,74,5,201,130,195,8,46,45,0,56,0,65,0,63,64,31,49,58,58,52,
    52,47,62,55,55,50,46,3,3,46,12,114,39,40,20,20,17,17,11,4,33,26,1,1,33,5,114,0,43,50,47,50,16,204,68,189,6,65,126,20,68,194,27,73,170,5,32,7,68,236,5,65,154,20,32,109,131,235,8,33,
    163,217,75,132,62,68,137,65,99,95,107,106,117,112,104,88,76,61,60,103,56,77,62,144,96,138,150,86,69,86,96,173,3,53,65,173,7,37,97,249,3,3,12,57,65,173,5,42,58,29,31,138,34,39,78,70,
    67,68,120,68,243,5,47,101,46,56,126,101,79,105,21,9,17,107,80,120,146,253,198,65,189,18,60,2,0,54,254,119,3,114,4,94,0,31,0,43,0,22,64,9,0,0,35,35,41,7,114,12,19,0,47,51,78,60,7,37,
    65,21,20,6,6,7,114,239,8,33,23,6,78,71,6,37,54,55,62,2,53,53,78,86,11,57,2,110,32,77,68,74,88,38,126,112,95,158,75,71,87,201,124,199,217,58,110,75,66,67,26,207,105,92,7,51,2,152,57,
    76,114,98,55,60,85,87,59,94,101,52,36,155,46,58,195,164,105,92,5,46,82,57,39,1,56,76,63,63,76,75,67,67,255,255,0,130,0,61,5,43,7,138,6,38,0,36,0,0,1,7,0,67,1,25,1,105,0,10,179,25,5,
    2,114,0,43,206,48,49,146,35,34,118,1,190,159,35,35,1,74,0,222,132,35,32,31,144,71,32,83,135,107,35,1,81,0,193,132,35,32,36,144,35,32,72,136,143,34,106,0,60,130,35,35,12,180,40,28,133,
    36,139,145,32,10,133,37,41,0,7,1,79,1,93,0,120,0,2,131,171,51,6,170,5,182,0,15,0,19,0,43,64,21,19,9,9,16,3,10,13,3,130,1,38,5,6,2,114,14,1,1,102,48,5,44,17,51,43,17,57,57,47,47,17,
    51,17,51,50,95,99,5,37,17,33,3,35,1,33,103,9,5,8,35,17,33,1,33,17,35,6,170,252,233,254,10,204,210,2,167,4,4,253,182,2,36,253,220,2,74,251,65,1,168,118,1,179,254,77,5,103,100,6,35,1,
    180,2,157,130,117,38,124,254,20,4,205,5,203,130,143,52,38,0,0,0,7,0,122,2,33,0,0,255,255,0,196,0,0,3,249,7,138,130,23,32,40,65,93,5,33,0,244,132,241,33,18,2,65,93,9,143,35,34,118,1,
    153,159,35,35,1,74,0,184,150,35,32,72,136,107,34,106,0,23,130,35,35,12,180,33,21,133,108,38,206,48,49,255,255,255,224,130,135,32,160,132,145,32,44,133,145,33,255,142,132,73,33,10,1,
    137,145,36,184,0,0,2,120,138,35,34,118,0,102,143,35,33,255,195,130,35,32,149,137,35,35,1,74,255,113,143,35,33,0,4,130,35,33,87,7,131,145,133,107,34,106,254,213,132,145,33,13,25,133,
    108,130,145,62,0,2,0,55,0,0,5,90,5,182,0,14,0,28,0,31,64,15,12,17,17,11,20,20,9,16,14,2,114,21,9,100,74,10,32,51,76,55,5,34,4,18,21,105,96,5,8,73,35,53,51,17,5,35,17,33,21,33,17,51,
    32,0,17,52,38,38,2,130,224,1,70,178,182,254,166,246,254,122,151,151,1,164,216,1,82,254,174,179,1,27,1,27,123,236,5,182,163,254,193,235,247,254,181,167,2,124,175,2,139,172,254,33,175,
    254,50,1,28,1,25,188,244,119,65,167,5,47,5,98,7,83,6,38,0,49,0,0,1,7,1,81,1,65,132,205,33,20,10,65,21,9,36,124,255,236,5,199,132,241,32,50,65,57,5,33,1,167,132,35,34,39,14,3,67,41,
    8,143,35,34,118,2,75,132,35,32,40,153,35,35,1,74,1,107,132,35,32,45,144,35,131,143,132,107,130,143,32,77,132,35,32,50,144,35,32,72,130,179,132,35,35,0,106,0,201,130,35,35,12,180,54,
    42,133,36,8,43,206,48,49,0,1,0,132,1,15,4,12,4,151,0,11,0,36,64,19,7,9,1,3,8,11,2,5,8,10,6,0,0,10,10,6,4,4,6,0,47,51,47,17,51,131,4,39,18,23,57,48,49,65,23,1,130,254,8,33,1,39,1,1,
    55,1,3,159,109,254,170,1,84,109,254,169,254,175,112,1,83,254,172,112,1,84,4,151,110,254,170,254,171,111,130,14,8,61,174,110,1,85,1,83,113,254,170,0,3,0,124,255,187,5,199,5,250,0,26,
    0,37,0,48,0,57,64,28,42,30,30,24,24,21,21,45,45,23,22,18,3,114,31,41,41,11,11,8,8,34,34,10,9,5,9,114,0,43,206,50,51,81,168,5,33,17,51,139,11,130,126,117,32,6,8,36,39,7,39,55,38,2,53,
    52,18,36,51,50,22,23,55,23,7,22,18,7,52,38,39,1,22,22,51,50,54,18,37,20,22,23,1,38,38,103,208,9,8,39,113,186,73,99,127,105,94,91,148,1,47,230,106,182,74,95,126,102,95,98,216,48,48,
    253,139,51,132,81,161,204,96,252,100,46,46,2,117,50,127,78,103,226,9,8,49,47,46,142,84,151,100,1,33,180,225,1,82,187,47,43,135,85,144,98,254,224,182,123,200,72,252,125,35,39,141,1,
    3,178,120,197,73,3,127,34,36,139,254,255,0,255,255,0,182,255,236,69,37,6,32,56,65,251,6,39,119,1,105,0,10,179,26,9,66,31,9,143,35,34,118,2,28,132,35,32,27,153,35,35,1,74,1,59,132,35,
    32,33,144,35,35,72,6,38,0,133,107,34,106,0,154,130,35,35,12,180,41,29,133,36,69,1,8,35,4,154,7,138,130,37,32,60,132,145,33,118,1,133,145,33,16,7,134,145,104,167,6,47,120,5,182,0,14,
    0,24,0,31,64,15,15,6,24,11,6,130,1,32,8,130,71,36,8,8,114,0,43,69,4,9,111,45,6,8,60,35,17,35,17,51,21,51,32,4,1,51,50,54,54,53,52,38,35,35,4,120,59,133,220,161,170,205,205,201,1,29,
    1,1,253,25,144,132,172,85,169,187,177,3,11,94,168,129,74,254,198,5,182,249,229,254,13,58,126,103,137,132,130,134,55,0,171,255,236,4,208,6,31,0,60,0,21,64,11,46,57,1,114,52,10,114,26,
    18,11,130,107,35,50,43,43,50,132,101,32,3,90,48,6,32,21,90,89,7,82,10,11,35,53,52,62,3,99,86,5,33,6,21,74,161,5,8,118,51,50,22,22,4,76,58,85,85,58,32,82,74,71,103,56,97,178,122,97,
    145,55,36,94,106,52,107,99,35,83,74,83,98,42,56,84,83,56,141,111,75,124,73,201,124,213,136,136,204,116,4,237,72,104,79,65,61,37,30,46,61,49,46,97,118,83,112,148,73,34,32,174,21,39,
    24,90,78,49,71,72,46,52,87,88,56,65,89,67,64,76,55,81,81,43,99,86,251,105,4,154,139,171,79,69,136,255,255,0,92,255,236,3,231,6,33,6,38,0,68,130,204,44,7,0,67,0,201,0,0,0,10,179,47,
    0,7,67,225,8,143,35,34,118,1,111,150,35,32,32,135,71,35,1,74,0,142,132,35,32,53,143,71,33,5,233,134,35,35,6,1,81,112,131,105,32,58,144,33,32,223,135,33,40,0,106,237,0,0,12,180,62,50,
    133,34,66,5,5,133,141,32,147,136,105,34,79,1,16,130,105,48,13,183,3,2,41,0,1,1,128,86,0,43,52,52,0,0,3,131,217,8,36,6,157,4,98,0,49,0,61,0,69,0,51,64,26,9,16,16,24,65,6,37,44,7,114,
    33,50,6,50,6,0,57,24,11,114,47,62,62,132,91,40,50,17,51,43,50,18,57,57,47,72,154,5,32,17,69,121,6,34,22,22,21,98,199,14,123,173,11,81,191,10,35,51,50,22,23,101,19,11,8,154,54,53,1,
    34,6,7,33,52,38,38,4,231,135,197,106,253,55,4,161,149,101,162,84,83,163,109,94,160,124,42,49,113,153,110,99,158,92,110,221,166,187,117,100,81,155,71,64,77,204,104,125,167,43,57,173,
    254,147,146,180,147,101,84,83,130,75,1,223,117,140,10,1,248,51,106,4,98,123,223,150,116,184,183,40,39,169,38,35,53,103,76,73,104,55,72,145,112,112,151,82,5,8,74,122,101,47,35,145,41,
    53,84,92,84,93,253,181,7,7,113,101,88,82,67,133,99,2,18,156,151,91,139,77,0,255,255,0,109,254,20,3,165,4,98,6,38,0,70,0,0,0,7,0,122,1,111,0,132,23,37,255,236,4,39,6,33,130,23,32,72,
    65,245,6,32,209,130,31,34,10,179,38,65,139,10,32,109,142,35,34,118,1,118,150,35,32,32,135,71,35,1,74,0,149,132,35,32,44,143,71,33,5,223,134,35,41,6,0,106,245,0,0,12,180,53,41,65,211,
    10,33,255,231,130,133,32,167,131,143,33,3,175,130,9,34,6,0,67,130,70,36,10,179,10,2,6,66,131,8,36,140,0,0,2,76,138,33,35,118,58,0,0,140,33,33,255,170,130,33,33,124,6,130,139,132,67,
    36,7,1,74,255,88,132,139,32,16,137,69,33,255,230,130,35,32,57,131,139,133,35,35,0,106,254,184,130,35,35,12,180,25,13,133,36,71,157,5,32,108,130,249,8,43,106,6,30,0,36,0,52,0,33,64,
    19,37,4,5,36,3,33,6,30,32,31,9,22,22,0,45,14,11,114,0,0,47,43,50,18,57,47,23,57,51,48,49,65,22,69,173,5,84,182,5,32,35,81,164,10,38,23,55,38,38,39,5,39,131,5,34,19,34,6,88,207,7,8,
    50,53,52,46,2,1,188,69,130,58,232,79,195,96,141,76,123,231,161,148,230,129,119,216,144,73,118,91,29,9,32,126,83,254,254,78,217,41,91,47,253,107,137,65,66,135,106,159,148,36,74,115,
    130,123,8,68,73,42,138,115,115,90,229,254,233,164,190,254,251,134,120,224,155,155,221,119,26,51,39,3,112,188,76,151,117,126,28,55,24,253,149,79,153,111,99,152,85,197,190,58,104,82,
    48,255,255,0,171,0,0,4,91,5,233,6,38,0,81,0,0,1,7,1,81,0,173,130,235,34,10,179,39,65,155,14,34,110,6,33,130,35,32,82,66,7,6,32,242,133,35,32,14,67,253,9,35,109,255,236,4,139,35,34,
    118,1,151,150,35,32,32,135,71,35,1,74,0,182,132,35,32,45,143,71,132,143,132,107,130,143,32,153,132,35,32,50,144,35,32,223,134,71,41,6,0,106,21,0,0,12,180,42,54,133,34,44,206,48,49,
    0,3,0,99,0,245,4,46,4,174,82,187,5,39,0,20,183,4,10,0,22,16,82,185,5,34,16,206,50,130,2,41,48,49,83,53,33,21,1,34,38,53,86,180,7,86,74,5,77,33,5,44,99,3,203,254,26,49,67,67,49,47,67,
    67,47,134,7,47,2,132,156,156,254,113,61,66,69,56,56,69,66,61,2,189,135,9,130,99,8,41,109,255,187,4,110,4,136,0,24,0,34,0,45,0,53,64,26,29,38,38,22,22,32,32,21,20,16,7,114,39,28,28,
    10,10,7,7,42,42,9,8,4,11,71,227,23,71,225,5,89,113,8,34,53,16,0,71,223,7,33,22,5,71,212,8,32,5,71,233,8,8,105,54,4,110,125,232,159,76,130,56,83,120,90,64,70,1,20,239,76,135,55,78,122,
    87,62,69,252,205,22,23,1,174,33,85,52,158,147,2,100,21,21,254,84,31,84,50,105,136,64,2,41,182,255,0,135,34,33,116,82,124,75,208,133,1,16,1,41,37,35,110,82,120,73,203,129,74,123,48,
    2,87,23,25,211,191,69,118,46,253,172,22,23,98,183,255,255,0,161,255,236,4,83,6,33,6,38,0,88,65,241,5,40,1,4,0,0,0,10,179,30,13,67,107,9,143,35,34,118,1,169,132,35,32,31,144,35,32,32,
    135,71,35,1,74,0,200,149,71,33,5,223,134,35,41,6,0,106,39,0,0,12,180,45,33,133,70,69,205,5,36,2,254,19,4,37,132,143,32,92,132,143,34,118,1,60,132,71,33,37,0,134,143,99,83,7,8,34,6,
    20,0,28,0,42,0,33,64,18,22,32,32,26,7,114,16,0,114,15,14,114,9,39,39,4,11,114,0,43,50,17,51,43,43,131,5,34,48,49,65,71,100,5,32,38,83,21,10,44,6,7,51,62,2,51,50,18,3,52,38,35,34,106,
    23,7,8,75,54,4,131,110,199,134,86,130,92,31,13,3,6,4,202,202,5,3,9,31,91,130,87,201,241,206,137,144,159,134,2,130,166,95,124,61,2,42,188,255,131,44,73,45,19,66,68,23,254,56,7,254,254,
    69,36,101,30,48,80,48,254,224,254,234,200,200,185,183,35,197,211,96,183,255,255,0,132,197,132,233,131,197,130,233,32,186,131,233,33,51,39,133,196,72,189,8,35,5,43,6,221,78,9,8,40,76,
    1,13,1,105,0,10,179,21,78,9,10,38,92,255,236,3,231,5,116,70,219,8,40,76,0,189,0,0,0,10,179,43,67,179,10,78,189,5,32,97,136,71,34,77,0,255,132,71,32,23,144,71,32,248,136,71,34,77,0,
    175,130,54,34,10,179,45,139,71,37,254,52,5,43,5,188,78,79,8,45,80,3,113,0,0,255,255,0,92,254,52,4,19,4,131,95,32,68,130,51,36,7,1,80,2,99,132,23,38,124,255,236,4,205,7,138,77,239,5,
    37,1,7,0,118,2,49,132,119,33,38,25,76,11,9,32,109,130,191,34,165,6,33,70,111,5,131,35,32,1,130,83,36,0,10,179,36,8,68,79,9,142,71,35,1,74,1,79,132,71,32,44,142,71,34,180,6,32,135,71,
    35,1,74,0,144,130,139,34,10,179,42,144,71,32,88,135,143,35,1,78,2,69,149,143,33,5,239,136,71,34,78,1,132,132,71,155,143,34,75,1,78,132,71,32,32,142,143,32,177,138,143,34,75,0,141,132,
    71,32,30,138,143,38,196,0,0,5,90,7,138,130,107,40,39,0,0,1,7,1,75,1,44,132,71,33,21,6,75,47,9,38,109,255,236,5,152,6,20,130,35,32,71,131,35,35,2,52,3,18,130,71,36,11,182,2,49,15,130,
    7,38,86,0,43,52,0,255,255,78,29,7,35,6,6,0,146,130,21,33,2,0,130,53,8,51,4,223,6,20,0,31,0,44,0,42,64,21,21,18,18,24,15,15,6,26,10,19,0,114,39,10,6,7,27,32,32,0,11,0,63,50,17,51,63,
    51,51,43,63,17,57,47,51,51,17,51,48,49,108,18,14,93,53,5,35,21,51,21,35,108,26,23,8,65,202,85,128,94,31,12,6,9,254,101,1,155,201,155,155,160,33,9,31,92,128,40,164,133,1,128,172,140,
    141,141,20,1,26,1,19,1,27,1,27,46,77,47,34,115,40,111,143,180,180,143,251,47,153,48,79,46,164,182,181,31,195,204,212,190,189,202,79,255,8,33,6,221,130,227,32,40,131,227,41,1,76,0,232,
    1,105,0,10,179,14,79,255,10,131,209,34,39,5,116,72,55,8,34,76,0,197,130,233,34,10,179,34,66,159,10,80,35,5,32,97,136,71,34,77,0,217,132,71,32,16,144,71,32,248,136,71,34,77,0,183,132,
    71,32,36,144,71,32,88,136,71,34,78,1,172,132,71,80,143,11,133,143,32,239,136,71,34,78,1,137,72,235,16,38,196,254,52,3,249,5,182,133,71,37,0,7,1,80,2,66,130,35,8,47,3,0,109,254,52,4,
    39,4,98,0,21,0,45,0,53,0,41,64,20,35,18,17,17,31,31,38,49,28,28,22,3,10,38,11,114,46,22,7,114,0,43,50,43,204,50,18,57,47,78,4,6,36,51,48,49,69,20,92,206,7,33,35,34,92,15,6,35,14,2,
    3,50,74,114,17,33,53,52,109,58,8,51,3,35,48,43,33,52,17,29,61,42,107,110,59,91,49,163,81,90,35,200,109,76,27,51,237,46,47,10,4,125,8,11,107,93,58,109,93,34,18,66,100,82,5,38,109,94,
    24,81,187,17,41,1,75,0,182,1,105,0,10,179,12,65,115,15,32,6,73,243,9,40,75,0,147,0,0,0,10,179,32,65,115,10,50,124,255,236,5,52,7,138,6,38,0,42,0,0,1,7,1,74,1,130,132,71,33,46,13,68,
    43,9,36,25,254,20,4,63,132,71,32,74,130,35,45,6,1,74,91,0,0,11,182,3,82,19,1,1,123,67,9,7,133,71,32,97,136,71,34,77,1,163,132,71,32,38,143,71,33,5,248,130,35,131,71,36,7,1,77,0,120,
    130,143,130,73,32,80,145,73,32,88,136,73,33,78,2,78,59,5,32,40,144,73,32,239,136,73,34,78,1,80,133,73,32,82,130,73,32,150,136,147,37,254,59,5,52,5,203,133,73,42,0,7,4,113,1,55,0,0,
    255,255,0,134,207,41,4,38,2,54,34,0,3,6,0,74,130,59,35,10,179,5,31,68,249,9,38,196,0,0,5,47,7,138,130,57,32,43,130,241,36,7,1,74,1,68,132,205,32,24,68,33,9,39,255,173,0,0,4,91,7,232,
    130,35,32,75,133,35,44,255,91,1,199,0,11,182,1,33,26,1,1,146,132,131,34,0,2,0,130,0,60,5,243,5,182,0,19,0,23,0,39,64,19,11,4,7,7,14,1,23,18,20,20,0,9,5,2,114,16,0,82,67,10,49,206,50,
    50,50,17,51,51,48,49,115,17,35,53,51,53,51,21,33,130,3,50,51,21,35,17,35,17,33,17,17,33,53,33,196,196,196,205,2,209,205,130,5,41,253,47,2,209,253,47,4,51,153,234,130,0,46,153,251,205,
    2,163,253,93,3,84,223,0,1,0,16,0,130,149,60,6,20,0,34,0,35,64,18,33,30,30,1,4,4,13,34,0,114,19,29,10,114,23,13,6,114,0,43,50,130,1,32,18,68,120,7,33,65,21,99,39,5,71,117,5,106,210,
    5,110,17,9,32,35,130,133,38,1,116,1,154,254,102,6,130,68,36,103,132,74,129,176,110,25,7,8,65,155,155,6,20,180,145,146,51,97,33,58,77,40,85,178,140,253,92,2,136,133,133,92,175,127,253,
    248,4,207,145,180,0,255,255,255,175,0,0,2,172,7,83,6,38,0,44,0,0,1,7,1,81,255,93,1,105,0,11,182,1,4,1,1,1,149,86,69,65,5,33,255,141,130,37,36,138,5,233,6,38,75,231,5,130,37,38,59,0,
    0,0,10,179,21,76,11,10,32,246,130,35,34,97,6,221,136,73,34,76,255,164,133,73,32,6,130,73,32,148,135,73,32,219,130,37,34,69,5,116,134,73,35,6,1,76,137,131,71,32,6,138,71,32,224,130,
    33,34,119,7,97,136,71,34,77,255,142,133,71,32,8,130,71,65,177,5,130,183,32,204,130,37,34,99,5,248,136,145,34,77,255,122,132,145,32,8,137,73,39,0,100,254,52,1,193,5,182,133,73,36,0,
    6,1,80,18,130,241,33,0,60,130,21,34,153,5,239,130,21,32,76,130,49,130,21,32,234,131,21,32,185,130,253,34,162,7,88,136,117,34,78,0,103,133,117,32,10,130,117,32,193,134,189,43,0,196,
    254,116,3,226,5,182,4,38,0,44,130,59,37,7,0,45,2,87,0,131,61,38,157,254,20,3,166,5,239,130,23,131,83,36,7,0,77,2,32,131,23,43,255,95,254,116,2,145,7,138,6,38,0,45,130,95,36,7,1,74,
    255,109,130,85,35,10,179,30,12,70,195,8,36,255,139,254,20,2,77,103,5,32,176,134,35,32,88,132,201,33,29,11,73,177,6,33,255,255,130,119,34,59,5,1,132,201,32,46,130,27,35,7,4,113,0,130,
    43,130,23,38,171,254,59,4,89,6,20,130,95,32,78,130,23,35,6,4,113,61,130,66,59,0,171,0,0,4,89,4,78,0,18,0,27,64,15,5,4,13,1,17,5,7,18,8,6,114,3,7,10,66,90,5,38,18,23,57,48,49,73,2,111,
    41,5,8,41,51,17,6,6,7,51,54,54,55,1,4,56,254,91,1,198,236,254,158,143,209,209,1,5,5,4,24,51,23,1,121,4,78,254,21,253,157,1,229,115,254,142,130,10,40,224,77,150,50,31,62,30,1,186,131,
    245,32,163,130,103,32,12,132,221,32,47,130,118,36,7,0,118,0,81,132,221,32,12,86,7,10,38,140,0,0,2,76,7,232,130,161,32,79,134,35,40,58,1,199,0,10,179,11,2,0,78,139,8,32,196,130,197,
    32,12,132,221,130,71,32,0,130,197,32,111,131,93,36,134,254,59,1,151,132,219,130,57,32,0,130,243,34,255,23,0,71,15,6,33,4,24,135,45,45,1,7,2,52,1,147,255,162,0,11,182,1,18,1,71,243,
    10,32,171,130,119,32,203,135,61,37,1,6,2,52,70,0,131,35,33,16,2,138,35,32,196,131,191,136,119,38,7,1,78,2,98,253,124,77,233,5,38,2,170,6,20,4,38,0,132,121,41,1,78,1,111,253,146,0,1,
    0,20,133,47,51,0,13,0,28,64,17,2,1,3,10,4,7,9,8,8,0,5,2,114,11,68,33,6,8,40,18,23,57,48,49,115,17,7,39,55,17,51,17,55,23,5,17,33,21,196,97,79,176,205,253,80,254,179,2,123,1,253,57,
    134,107,3,1,253,121,151,139,130,242,36,178,0,1,255,239,130,165,40,52,6,20,0,11,0,26,64,16,132,81,33,9,7,131,81,39,0,114,0,10,114,0,43,43,142,79,55,7,17,163,100,80,180,201,119,81,200,
    2,54,62,133,114,3,37,253,90,81,133,133,253,75,86,125,8,34,138,6,38,86,125,5,41,0,118,2,60,1,105,0,10,179,26,86,125,10,32,171,130,191,34,91,6,33,130,35,32,81,65,139,5,39,1,172,0,0,0,
    10,179,29,70,105,10,130,152,35,5,98,5,182,133,71,39,0,7,4,113,1,26,0,0,66,105,6,34,91,4,98,133,59,131,23,33,0,135,132,23,37,196,0,0,5,98,7,136,119,35,1,75,1,89,86,245,16,133,119,32,
    32,79,57,8,32,75,76,219,6,32,22,138,119,32,3,130,155,40,245,5,182,4,39,0,81,0,155,130,149,35,6,2,6,233,130,164,35,0,196,254,116,131,143,48,0,33,0,27,64,14,11,22,12,3,19,29,21,2,114,
    19,8,127,81,6,32,43,108,163,5,32,65,91,201,8,33,54,55,126,131,7,8,73,51,1,51,46,2,53,17,51,17,20,6,6,3,211,54,84,29,33,80,46,61,98,59,1,252,209,8,4,8,5,188,244,2,242,7,3,6,4,190,99,
    179,254,116,14,12,167,8,12,42,99,85,4,171,45,142,157,68,252,238,5,182,251,163,48,136,149,67,2,205,250,82,137,180,87,96,73,9,51,98,0,36,0,33,64,18,23,24,24,14,14,28,7,114,21,6,114,20,
    10,115,2,8,34,43,43,50,130,109,34,51,48,49,137,141,33,53,17,69,167,9,32,51,113,241,6,33,22,21,132,145,58,43,48,77,28,28,58,35,59,78,116,118,114,136,60,201,159,31,11,35,106,132,74,129,
    177,91,65,134,115,21,8,57,3,82,130,128,90,175,127,253,204,4,78,155,58,78,39,84,178,141,252,145,100,150,82,0,255,255,126,195,5,33,6,221,87,185,7,41,1,76,1,154,1,105,0,10,179,35,87,221,
    10,38,109,255,236,4,110,5,116,80,69,8,36,76,0,229,0,0,132,35,76,61,12,35,5,199,7,97,136,71,34,77,1,140,132,71,32,37,144,71,32,248,136,71,34,77,0,215,132,71,32,37,80,105,10,88,181,14,
    35,1,82,1,204,130,71,35,12,180,39,52,88,73,8,130,181,80,251,14,130,37,32,23,130,73,132,37,132,146,82,47,5,8,35,124,255,238,7,4,5,204,0,24,0,40,0,45,64,24,37,34,34,17,9,114,7,10,10,
    3,11,14,8,114,6,3,2,114,38,25,25,102,15,5,34,17,51,43,108,228,6,33,43,50,89,219,5,33,22,23,91,128,8,33,21,33,24,66,83,9,8,107,23,34,14,2,21,20,18,22,51,50,54,55,17,38,38,3,18,52,105,
    45,3,40,253,175,2,43,253,213,2,81,252,222,44,106,52,226,254,215,145,145,1,38,234,118,174,113,56,98,204,156,57,109,39,40,105,5,204,11,11,176,254,77,174,254,12,177,8,10,188,1,83,226,
    226,1,81,186,177,79,149,212,134,178,254,254,140,18,16,4,59,17,16,0,0,3,0,107,255,236,7,62,4,96,0,36,0,51,0,59,130,179,52,22,33,37,37,55,6,6,22,52,0,0,30,7,114,19,44,44,9,16,16,22,79,
    87,6,130,168,131,172,34,17,57,47,92,55,5,32,65,74,162,17,32,39,131,184,35,38,2,53,52,106,107,5,35,62,2,5,34,82,237,7,8,114,54,53,52,38,38,37,34,6,7,33,46,2,5,115,144,205,110,253,28,
    5,169,156,108,164,86,83,165,115,141,214,66,63,208,135,149,227,129,122,229,158,130,205,61,40,115,144,253,68,155,141,63,131,104,104,130,62,63,131,2,169,119,145,12,2,19,1,54,112,4,96,
    122,223,152,114,185,181,40,39,169,38,35,111,109,108,112,134,1,1,182,181,253,133,112,107,70,98,51,165,204,198,133,182,93,92,180,131,134,181,92,9,154,151,91,138,76,68,93,5,54,4,237,7,
    138,6,38,0,53,0,0,1,7,0,118,1,180,1,105,0,10,179,32,15,70,199,8,39,0,171,0,0,3,58,6,33,130,35,32,85,134,35,39,24,0,0,0,10,179,29,15,70,199,12,35,4,237,5,182,133,71,49,0,7,4,113,0,178,
    0,0,255,255,0,131,254,59,3,58,4,98,133,59,131,23,33,255,20,69,235,8,138,119,35,1,75,0,209,132,119,32,25,138,119,32,134,130,119,34,88,6,32,133,59,36,1,6,1,75,52,131,117,32,22,138,117,
    36,102,255,236,4,7,132,189,32,54,134,153,32,112,132,69,33,55,29,75,61,9,36,101,255,236,3,130,132,189,32,86,140,189,33,50,25,67,43,9,142,71,35,1,74,0,144,132,71,32,61,144,71,131,141,
    131,71,35,6,1,74,55,131,141,32,55,139,69,37,254,20,4,7,5,203,130,175,130,141,37,0,7,0,122,1,64,132,235,38,101,254,20,3,130,4,98,130,23,130,57,132,23,32,24,132,23,143,117,34,75,0,141,
    132,117,32,48,154,117,65,3,5,32,43,138,117,38,24,254,59,4,93,5,182,130,93,130,144,47,0,6,4,113,66,0,255,255,0,36,254,59,2,206,5,72,130,21,39,87,0,0,0,6,4,113,199,131,21,38,24,0,0,4,
    93,7,138,133,43,37,1,7,1,75,0,129,132,113,33,8,4,65,237,9,38,36,255,236,3,229,6,20,134,57,36,7,2,52,1,96,130,65,33,1,0,132,59,50,5,182,0,15,0,33,64,16,10,6,6,14,2,2,11,3,3,0,7,130,
    57,90,46,6,34,47,51,51,95,49,7,35,17,33,53,33,131,3,95,50,6,63,1,211,254,214,1,42,254,69,4,69,254,67,1,40,254,216,2,142,168,1,206,178,178,254,50,168,253,114,0,0,2,0,130,115,131,173,
    8,33,0,3,0,28,0,37,64,18,16,25,25,22,18,19,19,0,1,128,21,22,6,114,4,11,11,114,0,43,50,43,205,26,204,50,110,107,8,36,83,53,33,21,3,115,225,23,36,53,2,120,130,43,115,227,16,37,2,43,145,
    145,254,100,115,232,19,91,195,8,32,83,91,87,7,42,1,81,1,34,1,105,0,10,179,20,9,65,25,9,38,161,255,236,4,83,5,233,83,201,8,40,81,0,174,0,0,0,10,179,24,83,237,10,34,182,255,236,82,187,
    6,40,56,0,0,1,7,1,76,1,106,132,71,32,22,144,71,32,116,136,71,34,76,0,248,132,71,32,26,143,71,33,7,97,136,143,34,77,1,92,132,71,32,24,144,71,32,248,136,71,34,77,0,234,132,71,32,28,144,
    71,32,252,136,71,34,79,1,189,130,71,40,13,183,2,1,20,19,1,1,147,90,3,5,84,237,8,32,147,136,75,34,79,1,75,130,75,131,39,33,24,23,77,78,6,32,52,92,235,18,35,1,82,1,156,130,79,35,12,180,
    27,40,133,152,83,193,5,36,161,255,236,4,120,85,59,9,130,37,32,41,130,77,35,12,180,31,44,84,209,8,8,44,0,2,0,182,254,52,5,43,5,182,0,21,0,41,0,31,64,15,41,31,2,114,18,17,17,36,36,3,
    10,10,27,9,114,0,43,50,47,51,50,17,51,17,51,43,50,80,3,22,32,1,24,66,245,18,53,3,190,50,43,33,50,18,29,61,42,108,109,64,96,48,137,63,85,42,1,109,127,24,67,8,12,51,218,59,53,10,4,125,
    8,11,111,106,66,126,104,32,21,67,110,97,6,94,24,67,26,18,41,255,255,0,161,254,52,4,91,4,78,65,21,5,49,0,7,1,80,2,172,0,0,255,255,0,23,0,0,7,99,7,138,130,23,47,58,0,0,1,7,1,74,2,5,1,
    105,0,10,179,55,16,66,45,9,38,23,0,2,6,70,6,32,130,35,32,90,133,35,39,1,120,0,0,0,10,179,55,68,191,10,130,16,93,167,11,35,1,74,0,150,84,233,5,32,7,137,71,36,2,254,19,4,37,132,71,32,
    92,79,253,8,35,10,179,36,0,69,123,9,133,69,32,72,93,237,8,34,106,255,245,130,69,35,12,180,30,18,133,70,65,147,5,36,71,0,0,4,80,132,179,32,61,69,7,6,32,126,132,107,32,17,85,13,10,38,
    74,0,0,3,126,6,33,130,73,32,93,134,35,32,14,130,92,35,10,179,17,5,137,109,133,71,32,88,130,35,132,71,35,1,78,1,145,132,71,32,16,143,71,33,5,239,135,71,130,35,32,34,132,71,32,16,144,
    71,32,138,136,71,33,75,0,72,231,5,32,10,143,71,132,251,131,143,40,6,1,75,43,0,0,10,179,10,135,69,61,0,1,0,171,0,0,2,243,6,31,0,16,0,14,182,0,10,1,114,5,10,114,0,43,43,50,48,49,65,34,
    94,36,10,8,33,23,7,38,38,2,34,85,88,202,91,164,111,71,106,41,50,32,83,5,122,104,114,251,96,4,164,139,166,74,24,15,155,11,18,130,238,58,0,192,254,20,4,22,5,203,0,37,0,35,64,16,32,33,
    33,11,30,14,14,11,11,0,26,19,15,74,147,6,82,150,10,130,96,32,50,24,74,204,7,37,21,21,33,21,33,17,87,76,5,32,39,118,66,5,8,119,53,17,35,53,55,53,52,54,54,3,59,70,109,40,48,34,78,45,
    84,78,1,9,254,250,76,151,112,42,80,29,31,62,34,79,80,202,202,82,157,5,203,27,15,154,11,20,95,114,127,154,252,62,121,159,78,13,8,164,9,10,88,116,3,182,96,63,125,140,162,69,0,4,255,255,
    0,0,5,47,7,172,0,18,0,30,0,42,0,55,0,38,64,19,19,25,1,13,31,4,17,17,7,18,8,15,8,48,48,55,55,37,7,0,47,51,51,47,51,47,63,63,91,106,7,34,99,1,38,89,212,9,46,7,1,35,3,33,3,19,33,3,46,
    2,39,14,3,7,127,50,11,38,3,53,62,2,55,51,21,130,21,8,105,1,2,13,43,49,128,100,99,135,49,43,2,13,216,157,253,179,153,215,1,217,159,10,30,29,10,7,22,21,20,6,74,52,63,64,51,50,64,61,43,
    30,66,61,22,236,17,65,81,83,35,4,233,26,90,62,104,118,117,103,62,90,26,251,21,1,128,254,128,2,51,1,140,26,78,84,35,26,64,64,55,17,1,108,60,55,54,60,60,54,53,62,1,134,18,32,82,85,37,
    15,23,62,68,63,23,0,5,95,73,5,8,49,7,169,0,29,0,40,0,52,0,65,0,77,0,52,64,26,6,37,37,19,30,30,0,11,11,114,5,10,67,77,77,73,71,71,41,53,47,60,23,0,7,114,0,43,50,222,50,204,50,50,130,
    227,35,17,51,63,43,73,94,11,24,67,116,36,34,3,34,6,113,16,5,37,53,52,38,39,50,22,131,205,41,35,34,38,53,52,54,1,21,14,2,108,106,6,32,81,24,67,151,33,59,208,50,64,59,55,49,63,64,48,
    66,104,61,132,99,101,128,128,1,173,23,112,128,47,140,29,67,62,22,24,67,179,34,8,32,4,100,61,52,53,62,62,53,52,61,106,53,97,68,105,118,117,105,102,117,1,39,12,26,76,77,25,15,28,70,72,
    31,255,130,0,41,0,0,6,170,7,138,6,38,0,136,67,125,5,39,3,19,1,105,0,10,179,27,82,245,9,95,171,5,33,6,33,130,35,32,168,133,35,40,2,184,0,0,0,10,179,77,44,72,205,9,34,124,255,187,100,
    139,6,32,154,134,35,88,55,5,33,56,18,73,21,9,34,109,255,187,92,171,6,32,186,133,35,33,1,152,132,71,33,53,16,137,71,34,102,254,59,72,207,10,42,6,4,113,29,0,255,255,0,101,254,59,72,205,
    10,130,21,32,245,130,64,63,0,82,4,217,3,36,6,32,0,18,0,23,64,11,9,4,14,3,18,128,6,15,12,1,12,0,47,93,51,26,205,23,24,69,122,8,36,38,38,39,6,6,65,116,6,8,36,37,26,91,100,38,139,54,117,
    54,54,114,54,136,38,97,90,27,6,32,45,113,107,39,23,34,99,55,55,97,36,23,40,107,112,45,0,1,138,89,43,27,64,12,9,0,14,4,4,12,6,128,15,130,24,130,90,38,26,205,50,50,17,51,17,131,93,57,
    46,2,39,53,51,22,22,23,54,54,55,51,21,14,2,7,1,78,27,90,98,37,136,54,117,51,130,96,54,139,38,100,91,26,4,217,46,110,107,39,25,36,99,56,56,100,35,25,39,106,111,46,132,93,46,219,2,189,
    5,116,0,3,0,12,180,3,15,2,1,2,130,85,46,51,48,49,65,21,33,53,2,189,253,149,5,116,153,153,132,37,49,217,2,233,5,248,0,16,0,18,183,16,8,128,12,15,4,1,4,132,217,33,204,50,130,43,122,59,
    5,36,51,30,2,51,50,130,125,62,2,233,7,83,146,100,154,165,8,121,5,53,90,61,54,90,59,7,5,248,85,129,73,156,131,56,57,19,21,58,53,131,77,39,5,0,1,59,5,239,0,11,130,115,36,0,15,6,1,6,133,
    115,32,83,24,93,153,10,41,199,47,69,69,47,50,67,67,5,239,24,65,166,8,40,2,0,82,4,217,2,30,6,147,116,129,6,40,18,6,192,12,0,0,47,50,26,132,131,93,195,10,112,237,12,8,34,54,101,127,127,
    101,98,134,133,99,50,64,66,48,48,66,59,4,217,118,104,102,118,117,101,105,119,108,61,53,52,61,61,52,53,61,131,145,50,254,52,1,176,0,31,0,20,0,14,180,3,10,192,18,17,0,47,51,132,87,32,
    87,87,122,16,42,23,6,6,238,48,44,32,52,18,29,62,87,90,5,34,102,70,75,87,60,11,36,92,32,31,64,112,66,17,5,54,219,3,79,5,233,0,25,0,29,64,13,22,13,13,5,17,128,25,25,10,15,17,1,130,92,
    8,90,93,51,51,47,26,16,205,50,47,50,48,49,83,62,3,51,50,30,2,51,50,54,55,51,6,6,35,34,46,2,35,34,6,7,82,6,37,61,81,48,45,83,76,72,34,43,54,14,111,13,123,97,44,81,76,73,36,44,53,14,
    4,219,65,100,68,35,35,46,35,57,61,127,142,36,45,36,57,61,0,0,2,0,82,4,217,3,79,6,33,0,12,130,111,50,31,64,14,1,14,14,12,25,128,8,6,6,21,15,19,1,19,0,47,130,112,33,17,51,66,45,5,34,
    48,49,65,112,98,8,32,35,136,9,44,3,79,17,66,83,84,35,113,30,69,66,21,155,134,10,47,68,65,23,6,33,20,28,81,89,81,29,25,39,107,111,46,138,10,35,0,1,1,254,130,113,34,38,6,108,130,113,
    37,14,180,8,6,128,1,65,139,9,34,53,62,3,66,131,6,8,63,254,14,28,24,20,5,205,13,57,69,36,4,217,28,38,94,103,99,41,22,49,133,140,59,0,0,3,1,11,5,10,3,169,6,180,0,11,0,23,0,35,0,23,64,
    9,24,30,30,18,11,128,6,12,18,0,47,51,220,26,204,17,51,17,51,132,171,68,163,6,32,7,66,30,10,32,33,138,11,47,3,37,20,61,73,38,95,15,33,29,8,237,43,56,56,43,130,3,57,2,2,41,59,59,41,45,
    55,55,6,180,20,43,106,110,49,23,42,106,111,46,213,51,55,54,53,118,117,8,48,54,55,51,255,255,0,10,0,0,5,53,6,3,6,38,0,36,130,10,38,7,1,83,254,27,255,150,130,23,42,142,3,77,1,152,4,102,
    6,7,0,17,130,170,32,105,68,155,5,45,4,174,6,3,4,39,0,40,0,181,0,0,0,7,130,43,32,0,131,43,130,45,34,0,5,228,132,25,32,43,145,25,33,2,75,132,25,34,44,0,186,141,51,35,255,236,6,38,130,
    25,35,38,0,50,95,143,75,32,226,132,49,34,60,1,72,140,49,43,246,0,0,6,87,6,4,4,38,1,117,93,132,49,57,253,247,255,152,255,255,255,209,255,236,2,180,6,180,6,38,1,133,0,0,1,7,1,84,254,
    198,130,55,48,16,64,9,3,2,1,47,16,1,1,175,86,0,43,52,52,52,108,167,7,37,5,188,6,6,0,36,78,129,8,34,202,5,182,130,15,32,37,130,49,47,1,0,196,0,0,4,19,5,182,0,5,0,14,182,2,5,24,75,173,
    8,49,48,49,65,21,33,17,35,17,4,19,253,126,205,5,182,178,250,252,130,35,34,2,0,42,130,45,34,160,5,184,130,45,38,16,0,25,64,12,11,3,130,49,47,1,4,16,16,3,8,114,0,43,50,18,57,57,43,17,
    57,130,58,8,56,1,21,33,53,9,2,46,2,39,14,2,7,1,2,208,1,208,251,138,1,208,1,203,254,243,16,33,27,9,9,26,31,15,254,240,5,184,250,193,121,123,5,61,250,249,3,24,49,106,99,38,38,99,103,
    46,252,226,85,15,6,33,3,249,132,161,32,40,132,177,32,71,130,115,32,80,132,15,32,61,83,161,8,32,47,132,15,32,43,130,193,32,3,82,57,5,37,5,205,0,3,0,21,130,227,130,149,41,3,2,2,9,33,
    18,3,114,26,9,24,83,147,13,130,148,32,5,105,75,6,24,80,20,24,37,4,77,253,167,3,211,24,81,16,28,37,3,62,174,174,97,169,24,81,19,32,80,101,5,33,1,145,132,169,32,44,136,185,32,1,132,15,
    32,46,130,185,32,1,130,3,54,0,4,234,5,182,0,14,0,19,64,9,7,2,14,2,114,2,13,8,114,0,43,50,65,69,6,33,35,1,65,66,6,8,39,35,1,2,225,2,9,213,254,191,18,36,32,12,9,30,36,18,254,192,213,
    2,7,5,182,250,74,3,164,53,116,112,45,46,112,115,52,252,91,5,182,133,115,33,6,132,132,99,32,48,136,115,32,98,132,15,32,49,130,111,38,3,0,71,0,0,4,43,130,115,49,3,0,7,0,11,0,25,64,12,
    5,4,4,0,8,9,2,114,1,90,20,11,38,48,49,115,53,33,21,1,134,3,52,71,3,228,252,151,2,238,252,192,3,146,177,177,2,164,175,175,2,98,176,176,83,179,7,37,5,205,6,6,0,50,132,207,36,196,0,0,
    5,22,130,91,39,7,0,16,183,6,1,2,114,130,88,132,205,32,50,130,78,35,17,33,17,35,130,3,37,196,4,82,203,253,70,131,185,35,5,5,250,251,133,175,33,4,119,132,159,32,51,132,67,32,74,130,159,
    130,175,46,0,18,0,39,64,19,3,11,11,7,7,12,2,0,4,130,158,34,13,13,16,90,183,6,39,17,51,17,51,43,18,57,57,130,6,33,17,51,130,90,33,53,1,131,168,8,49,33,34,34,38,39,1,1,54,54,51,33,21,
    74,1,229,254,39,3,209,253,254,33,88,85,27,1,211,254,23,74,148,77,2,17,166,2,87,2,20,165,177,2,2,253,246,253,173,2,3,177,130,125,79,221,6,130,193,34,55,0,0,67,45,5,33,4,154,132,141,
    32,60,130,141,8,33,3,0,102,255,236,6,25,5,203,0,24,0,33,0,42,0,33,64,16,34,1,1,25,23,24,3,114,35,10,10,33,13,12,9,130,222,34,205,50,50,130,142,132,5,8,115,48,49,65,21,22,4,22,21,20,
    14,2,7,21,35,53,46,3,53,52,62,2,55,53,17,14,2,21,20,22,22,23,19,17,62,2,53,52,38,38,3,161,234,1,21,121,64,145,243,180,195,183,245,143,61,67,148,242,175,158,189,83,92,190,148,195,152,
    189,89,83,188,5,203,180,4,147,242,150,102,195,158,97,3,225,225,3,100,159,193,98,110,194,150,88,3,180,254,164,5,94,165,111,117,172,96,5,2,253,253,3,5,98,173,116,111,164,93,131,181,36,
    5,0,0,4,200,132,181,32,59,130,181,58,1,0,111,0,0,6,24,5,182,0,29,0,31,64,15,24,21,21,6,9,9,8,29,23,15,2,114,106,225,5,33,50,50,91,137,9,32,17,130,171,36,35,17,35,17,34,86,90,6,45,22,
    22,51,17,51,17,50,54,54,53,17,6,24,62,130,162,8,56,194,181,242,142,60,200,90,188,147,194,147,189,91,5,182,254,32,117,197,144,81,254,69,1,187,82,146,195,113,1,227,254,33,129,162,76,
    3,78,252,178,77,160,127,1,226,0,1,0,73,0,0,5,250,5,205,0,39,131,123,50,0,20,3,114,32,28,28,31,31,8,11,11,10,8,114,0,43,50,17,78,219,9,32,65,102,242,6,36,23,21,33,53,33,130,126,39,52,
    18,36,51,50,4,18,21,127,63,10,8,75,52,38,38,3,33,153,205,103,66,146,120,253,169,1,118,91,146,84,161,1,46,212,214,1,47,160,84,145,93,1,118,253,168,122,147,66,104,207,5,28,118,220,155,
    136,231,194,82,172,178,65,191,250,151,198,1,36,160,159,254,221,199,152,250,191,65,178,172,81,195,233,134,156,220,117,255,255,111,121,35,66,1,7,32,7,78,85,31,55,109,255,236,4,213,6,
    108,6,38,1,125,0,0,1,6,1,83,42,0,0,10,179,52,20,74,69,9,36,86,255,236,3,178,132,33,32,129,133,33,32,228,131,33,32,45,93,237,10,36,171,254,20,4,91,132,33,32,131,133,33,32,75,131,33,
    32,24,88,67,10,36,166,255,236,2,180,132,33,69,229,5,35,83,254,193,0,131,35,33,17,16,78,151,9,38,158,255,237,4,130,6,180,130,137,32,145,132,69,40,84,40,0,0,14,181,30,54,42,78,45,5,32,
    206,86,129,5,132,175,49,4,97,0,13,0,51,0,39,64,20,27,6,114,23,24,24,8,8,130,175,35,36,44,44,48,130,49,85,203,14,36,51,43,48,49,101,24,71,190,12,50,23,34,2,17,16,18,51,50,22,23,51,54,
    54,55,51,14,2,21,17,73,36,6,74,7,5,8,128,35,14,2,2,87,107,129,59,1,129,168,144,139,138,103,202,245,246,219,121,161,51,13,8,33,22,162,15,25,16,50,36,16,37,9,10,39,46,22,83,103,23,15,
    30,89,130,144,85,174,134,15,198,205,210,200,199,202,164,1,30,1,25,1,23,1,39,87,85,37,84,32,46,142,164,79,254,111,70,54,7,4,152,6,11,8,75,99,48,79,47,0,0,2,0,171,254,20,4,189,6,31,0,
    24,0,48,0,41,64,21,8,7,41,41,42,42,0,20,15,114,18,30,33,33,15,11,114,25,0,1,68,105,5,36,17,51,57,43,18,121,223,10,35,22,21,20,6,116,243,6,36,6,35,34,38,39,109,64,5,37,23,34,6,6,21,
    17,24,99,23,14,8,84,53,52,38,2,158,139,214,122,157,147,180,192,120,221,155,113,164,67,202,129,226,139,80,133,79,67,154,93,166,156,84,151,102,100,81,148,139,149,6,31,87,173,130,147,
    175,24,8,21,196,185,140,197,103,38,34,253,223,6,53,163,208,99,164,58,139,123,252,169,38,46,153,139,102,130,62,166,145,121,120,121,0,0,1,0,7,130,183,49,43,4,78,0,23,0,21,64,10,17,10,
    6,23,11,6,114,6,15,130,170,48,43,50,18,57,57,48,49,65,1,14,2,21,35,52,54,54,55,24,70,138,7,8,87,2,55,19,4,43,254,106,32,45,24,215,26,47,30,254,71,209,223,20,44,35,6,8,6,31,40,19,210,
    4,78,251,207,83,189,178,71,59,175,189,83,4,64,253,195,52,130,119,36,32,117,129,49,2,71,0,0,2,0,107,255,236,4,108,6,24,0,36,0,51,0,28,64,16,34,14,30,37,49,41,6,0,45,22,11,114,7,0,0,
    93,40,10,32,65,79,194,9,36,20,22,22,23,30,24,97,153,18,32,19,68,178,5,110,241,6,8,118,2,152,137,201,89,84,81,171,99,93,87,56,122,98,123,169,88,128,232,156,149,229,131,109,191,122,75,
    114,63,218,178,81,166,111,72,135,94,99,138,72,135,6,24,65,43,152,44,57,81,56,48,78,83,53,65,149,182,118,166,227,115,107,205,145,133,195,132,35,44,98,120,76,142,148,253,54,21,91,164,
    130,87,133,75,78,151,108,133,170,0,0,1,0,86,255,236,3,178,4,98,0,44,0,31,64,15,23,24,44,44,2,2,17,38,31,7,114,10,17,11,133,178,24,100,215,9,33,21,35,107,36,9,35,55,21,6,6,78,206,5,
    33,55,53,24,81,93,15,8,73,21,20,22,51,2,237,152,106,138,67,71,130,88,115,184,69,66,187,127,246,231,145,106,98,109,114,196,124,116,184,82,70,68,147,96,123,124,162,145,2,140,155,40,82,
    60,61,77,36,52,32,171,33,41,181,139,122,122,27,10,27,125,98,97,129,65,41,37,155,31,42,76,73,86,75,130,159,58,109,254,118,3,181,6,20,0,41,0,27,64,12,21,25,25,26,26,38,12,3,27,0,114,
    3,0,47,70,131,10,36,69,20,6,7,35,69,221,5,48,39,46,2,53,52,62,2,55,14,2,35,33,53,33,21,6,2,107,216,5,8,78,23,30,2,3,181,80,50,201,35,60,37,34,99,101,132,182,93,94,168,219,125,16,89,
    116,56,254,244,3,11,209,251,129,44,79,149,105,109,131,60,77,88,162,67,46,99,94,35,29,46,38,18,24,108,184,134,146,250,225,214,110,2,4,3,159,139,175,254,234,224,185,82,112,119,57,22,
    21,66,91,0,130,153,40,171,254,20,4,91,4,98,0,23,130,153,44,15,17,6,114,16,10,114,5,15,114,19,10,10,110,176,8,37,43,43,48,49,65,50,97,95,15,8,36,51,23,51,62,2,2,207,128,176,92,199,115,
    121,113,135,60,201,159,29,11,35,108,133,4,98,84,178,142,251,70,4,159,132,133,90,176,128,24,77,39,8,59,3,0,108,255,236,4,92,6,33,0,13,0,20,0,27,0,25,64,12,24,18,18,4,21,11,1,114,14,
    105,89,5,37,43,50,17,57,47,51,106,197,6,8,89,32,2,17,52,18,54,51,32,18,1,50,18,19,33,18,18,19,34,2,3,33,2,2,4,92,102,223,182,255,0,245,102,221,178,1,1,250,254,5,152,145,6,253,168,4,
    139,155,149,139,10,2,86,9,142,3,8,252,254,157,189,1,163,1,120,251,1,99,188,254,98,252,6,1,31,1,31,254,228,254,222,4,250,254,241,254,243,1,13,1,15,130,241,44,166,255,236,2,180,4,78,
    0,16,0,14,182,16,87,33,9,130,122,68,139,7,8,36,6,6,35,34,38,38,53,17,1,109,78,79,45,94,31,32,114,58,94,145,83,4,78,252,250,93,92,16,10,154,15,20,61,150,134,3,9,95,17,5,45,4,89,4,78,
    6,6,0,249,0,0,0,1,255,249,130,227,32,112,130,227,50,45,0,30,64,17,2,17,1,40,4,13,22,30,11,114,5,13,1,114,94,184,5,32,50,93,152,6,49,99,1,39,38,38,35,34,6,7,53,62,2,51,50,22,22,23,1,
    77,195,5,34,21,14,2,131,121,8,95,39,3,46,3,39,35,6,6,7,3,7,1,217,51,37,83,85,34,55,21,18,47,51,24,108,135,90,41,1,85,18,36,40,25,15,35,12,15,42,47,23,57,80,57,22,145,12,27,26,20,6,
    7,17,53,29,250,4,50,144,95,88,8,5,164,5,7,5,73,150,114,252,71,50,57,25,7,3,152,7,11,7,38,80,60,1,155,35,79,78,71,26,63,147,70,253,190,94,185,5,34,20,4,92,132,187,32,119,131,187,55,
    0,2,0,0,4,30,4,78,0,17,0,21,64,10,7,6,0,17,10,114,12,0,6,114,74,70,5,8,87,57,48,49,83,51,19,30,2,23,51,54,18,18,53,51,20,2,2,7,35,2,207,230,15,39,35,9,8,115,135,59,200,79,191,169,202,
    4,78,253,139,40,114,109,36,121,1,21,1,74,200,218,254,137,254,171,168,0,1,0,107,254,118,3,180,6,20,0,60,0,37,64,17,5,4,30,30,33,33,48,22,13,13,17,17,18,18,19,0,71,203,5,36,17,51,17,
    51,47,69,109,7,36,83,52,54,54,55,24,71,133,7,47,14,2,35,35,53,33,21,35,34,14,2,21,20,22,51,51,67,187,8,68,110,5,67,44,10,8,99,107,79,131,80,69,100,53,78,137,87,32,88,93,39,56,2,216,
    61,86,166,133,80,147,178,166,170,123,168,85,82,152,104,109,129,56,79,46,196,35,58,34,32,101,102,134,183,95,1,171,100,153,104,26,11,18,71,109,77,91,122,76,23,3,5,3,159,149,37,75,115,
    77,104,108,148,76,136,90,101,104,51,22,22,68,91,59,87,163,67,47,99,93,37,27,45,39,19,23,104,168,0,93,169,7,8,34,4,98,6,6,0,82,0,0,0,1,0,25,255,236,5,32,4,78,0,24,0,29,64,16,12,17,21,
    3,16,16,18,6,114,14,10,24,77,81,8,36,43,50,17,23,51,24,77,79,11,40,53,17,33,17,35,17,35,53,55,130,214,8,56,17,20,22,4,164,30,51,19,21,82,51,116,127,254,71,199,226,157,4,106,221,54,
    142,14,10,154,12,20,136,138,2,173,252,85,3,171,89,74,163,253,96,68,57,0,2,0,153,254,20,4,108,4,98,0,21,0,34,130,109,44,15,22,18,7,114,13,15,114,7,26,26,29,29,67,66,5,35,17,51,17,51,
    85,190,5,37,20,2,6,35,34,38,24,79,231,7,93,47,5,34,22,37,34,70,173,10,8,58,4,108,115,214,148,85,155,58,12,5,6,203,119,223,154,143,218,122,254,22,145,141,56,147,79,152,136,133,2,40,
    185,255,0,131,48,42,40,144,83,254,217,4,32,185,247,126,132,254,219,193,197,254,191,49,50,202,205,205,198,131,247,36,109,254,118,3,183,130,139,43,38,0,20,64,10,26,18,30,14,4,21,7,68,
    48,5,34,47,23,57,85,222,12,70,27,8,65,172,9,8,74,3,53,52,18,54,2,130,84,162,63,61,60,128,67,169,151,62,148,128,109,130,56,80,45,196,35,58,36,34,100,100,91,152,110,60,133,240,4,98,35,
    28,162,22,33,222,212,115,132,73,28,23,68,92,60,89,168,64,47,100,95,38,28,46,40,19,18,71,117,171,118,215,1,11,126,0,106,181,5,50,202,4,78,0,17,0,32,0,23,64,11,29,15,30,30,12,6,114,22,
    68,84,8,33,51,51,130,142,32,20,67,209,6,38,52,54,36,51,33,21,33,24,81,136,8,8,71,53,52,38,38,39,35,34,6,4,112,118,230,170,154,230,125,145,1,15,187,2,2,254,243,79,100,252,204,63,136,
    108,161,146,37,72,52,62,199,192,1,242,148,234,136,127,244,177,204,253,117,160,79,219,116,113,172,98,202,160,88,155,134,58,189,0,1,0,25,255,235,3,172,130,127,32,21,131,125,40,20,2,19,
    19,21,6,114,6,13,70,117,7,132,125,33,21,33,68,88,15,8,39,33,53,55,3,172,254,92,99,83,47,95,35,33,115,64,97,159,94,254,220,157,4,78,163,253,167,107,90,14,11,151,14,22,59,151,136,2,102,
    89,74,130,95,36,158,255,237,4,130,130,95,41,23,0,16,183,17,5,6,114,10,0,134,91,34,48,49,69,24,79,51,11,130,202,8,88,39,51,30,2,21,16,0,2,120,182,207,85,201,137,149,155,152,34,32,202,
    22,29,15,254,249,19,135,235,148,2,91,253,167,172,186,224,240,139,230,126,83,155,165,99,254,195,254,210,0,2,0,109,254,20,5,118,4,97,0,29,0,41,0,33,64,17,22,15,114,8,23,23,34,20,11,114,
    1,0,0,30,12,7,114,0,43,50,50,17,51,133,4,35,48,49,65,23,70,133,6,55,17,52,54,51,50,22,22,21,20,2,4,7,17,35,17,46,2,53,52,18,5,34,6,21,76,154,6,8,109,1,67,151,77,92,90,154,95,182,155,
    130,187,100,154,255,0,152,192,156,242,137,120,2,242,63,84,106,163,94,52,97,4,92,103,102,219,143,138,172,85,11,2,85,187,194,134,243,164,194,254,254,135,10,254,37,1,219,11,122,245,194,
    169,1,22,40,99,121,253,169,9,104,188,134,120,171,93,0,0,1,255,232,254,20,4,109,4,87,0,38,0,34,64,18,13,20,20,25,8,28,5,4,0,27,15,114,32,0,0,7,6,131,164,37,47,51,43,17,23,57,130,170,
    34,48,49,83,130,152,37,23,19,1,51,1,19,69,83,6,65,251,5,37,39,3,1,35,1,3,69,117,6,8,117,54,54,192,72,91,65,30,131,1,53,209,254,80,195,23,48,62,45,25,46,22,27,66,47,87,116,77,32,142,
    254,164,215,1,216,166,29,69,50,20,41,19,27,67,4,87,56,110,83,254,172,2,68,253,1,254,23,59,75,36,5,4,156,9,12,65,123,88,1,127,253,109,3,82,1,173,79,79,6,7,159,8,12,0,1,0,158,254,20,
    5,185,6,18,0,31,0,33,64,18,8,8,24,6,114,17,15,114,30,19,19,1,16,11,114,0,0,114,0,43,65,70,5,37,43,50,47,48,49,65,65,41,5,33,39,51,65,68,12,8,66,17,51,17,20,22,22,23,17,3,127,119,169,
    90,37,33,195,34,33,150,254,254,162,192,161,246,138,196,88,157,104,6,18,250,129,10,95,184,142,140,244,140,138,246,136,208,254,254,124,9,254,37,1,219,6,116,247,201,2,37,253,214,145,171,
    80,7,5,129,130,135,59,114,255,236,5,237,4,78,0,48,0,35,64,17,35,35,15,48,23,6,114,12,11,41,41,8,8,30,15,130,137,132,133,38,17,51,51,43,50,18,57,131,137,73,184,8,33,39,35,67,37,8,36,
    54,55,51,6,2,87,247,5,33,54,53,133,149,73,196,5,8,92,2,39,5,117,41,53,26,90,178,134,118,143,35,9,33,145,116,131,180,91,26,53,41,200,64,61,116,105,67,84,40,189,42,84,63,72,99,52,61,
    65,4,78,95,180,186,108,162,250,141,100,93,93,100,139,249,165,108,186,179,96,142,254,247,161,189,202,67,120,79,1,54,254,202,85,119,62,91,175,124,160,1,11,142,255,255,255,235,255,236,
    2,180,5,223,82,135,7,59,0,106,254,189,0,0,0,12,180,38,26,16,6,114,0,43,206,206,48,49,255,255,0,158,255,237,4,130,132,37,39,145,0,0,1,6,0,106,31,131,35,34,45,33,5,138,35,99,73,5,36,
    108,6,38,0,82,131,35,39,1,83,25,0,0,10,179,33,99,145,10,132,69,32,6,130,33,32,1,132,69,34,1,83,35,131,33,32,24,91,91,10,36,114,255,236,5,237,132,33,32,149,130,67,36,7,1,83,0,219,130,
    141,35,10,179,49,23,77,47,9,37,196,0,0,3,249,7,126,17,28,63,0,1,0,24,255,237,5,114,5,182,0,32,0,33,64,17,16,25,25,0,24,20,20,21,2,114,18,8,114,7,0,9,130,216,32,50,101,27,5,39,57,47,
    51,48,49,69,34,38,90,254,6,40,54,53,53,52,38,35,33,17,35,96,3,7,8,59,50,22,21,21,20,6,3,249,48,88,30,35,74,44,46,85,56,113,132,254,140,203,254,165,3,234,254,60,1,129,210,225,206,19,
    14,12,175,12,13,34,94,88,133,114,107,253,41,5,5,177,177,254,131,194,183,141,202,203,255,255,83,153,5,37,7,138,6,38,1,96,131,205,42,0,118,1,162,1,105,0,10,179,13,5,93,37,6,130,167,54,
    124,255,236,4,242,5,205,0,35,0,25,64,12,5,8,8,20,0,29,3,114,13,20,82,247,13,40,34,14,2,7,33,21,33,30,3,24,105,208,17,8,72,22,23,7,38,38,3,67,104,173,127,77,11,2,190,253,62,5,67,124,
    180,118,101,185,93,86,195,119,237,254,207,146,93,183,1,12,174,129,205,90,79,78,170,5,26,60,118,172,114,174,122,190,131,69,35,28,176,32,31,188,1,82,226,168,1,20,200,109,50,43,172,37,
    49,0,130,175,32,102,130,139,39,7,5,203,6,6,0,54,0,131,15,32,196,130,181,82,253,11,126,249,34,33,255,255,24,102,103,7,130,69,63,45,0,0,0,2,0,5,255,234,7,71,5,182,0,38,0,48,0,31,64,16,
    39,27,8,114,48,18,18,0,29,16,2,65,138,8,41,50,17,57,47,51,43,50,48,49,87,65,137,9,39,55,54,54,18,18,55,33,17,68,236,5,8,107,4,33,33,17,33,14,4,7,14,2,37,51,50,54,53,52,38,38,35,35,
    139,36,70,28,24,55,32,57,68,42,18,13,33,38,41,21,2,211,133,205,246,109,254,251,254,231,254,157,254,164,11,25,27,27,27,13,27,79,134,3,154,134,176,174,85,168,125,106,22,13,10,171,10,
    13,87,150,93,66,202,1,2,1,42,159,253,161,108,189,123,202,233,5,5,86,188,192,184,163,66,136,191,101,195,127,135,93,108,45,130,179,130,249,33,7,108,130,179,42,19,0,29,0,35,64,17,7,3,
    3,29,130,178,36,6,1,2,114,20,82,195,9,32,43,132,182,40,51,17,51,48,49,115,17,51,17,130,170,139,172,32,17,137,168,63,196,205,2,89,206,132,204,246,110,254,250,254,233,254,155,253,167,
    3,39,134,175,174,85,167,125,106,5,182,253,158,2,98,134,136,36,2,163,253,93,173,133,129,38,1,0,24,0,0,5,114,133,129,47,64,14,2,18,18,14,3,3,16,19,2,114,9,16,8,114,74,23,5,32,47,101,
    171,6,35,21,33,17,33,24,89,67,8,66,185,5,8,33,4,41,254,21,1,145,202,217,204,105,125,254,126,205,254,167,5,182,178,254,130,191,184,253,241,1,250,114,105,253,43,5,4,178,65,221,6,39,5,
    0,7,138,6,38,1,179,66,157,6,38,224,1,105,0,10,179,18,94,199,10,38,22,255,236,5,8,7,129,130,35,32,188,131,35,35,2,51,0,70,132,35,33,34,17,66,193,9,36,196,254,119,5,22,130,169,44,11,
    0,21,64,10,11,6,2,114,8,2,1,5,132,165,38,204,51,43,50,48,49,97,132,148,65,32,5,52,5,22,254,59,206,254,65,205,2,186,203,254,119,1,137,5,182,250,253,5,3,86,227,15,65,127,5,32,4,130,29,
    48,0,13,0,23,0,25,64,12,5,23,23,0,4,1,2,114,14,84,225,14,33,17,33,130,253,66,32,5,34,6,33,39,66,24,8,61,196,3,107,253,98,201,210,244,105,253,254,224,219,202,184,164,79,166,131,174,
    5,182,176,254,81,106,190,124,206,229,65,98,6,67,159,6,87,73,8,134,15,44,19,5,182,6,6,1,96,0,0,0,2,0,12,130,217,32,117,130,217,32,15,130,133,51,26,64,12,17,15,2,114,10,1,22,22,4,7,6,
    8,0,63,205,50,51,24,87,70,8,35,51,17,35,17,131,229,8,35,51,54,26,2,55,5,33,6,10,2,7,33,4,189,184,197,252,33,197,112,70,125,100,68,13,1,253,254,177,11,59,91,111,62,2,157,131,243,56,
    253,196,1,137,254,119,2,60,124,1,47,1,79,1,93,172,179,120,254,224,254,209,254,231,112,87,61,16,39,0,1,0,3,0,0,6,235,130,133,41,17,0,34,64,19,0,9,3,15,12,130,160,46,14,14,11,17,8,114,
    8,5,1,2,114,0,43,50,50,130,2,24,86,115,9,40,17,51,17,1,51,1,1,35,1,130,149,61,1,35,2,67,253,216,221,2,30,195,2,29,222,253,216,2,63,230,253,212,195,253,211,230,2,242,2,196,253,60,134,
    3,38,61,253,13,2,229,253,27,131,3,79,207,5,54,4,98,5,203,0,46,0,31,64,15,4,3,27,27,26,26,11,35,43,3,114,19,11,68,148,10,80,251,5,81,153,8,32,4,24,80,254,12,34,35,35,53,70,178,5,37,
    38,35,34,6,7,39,77,103,5,8,179,4,64,190,156,185,195,132,254,250,194,131,226,91,62,148,153,68,197,198,241,215,191,183,151,191,90,158,134,133,186,86,95,62,154,186,110,157,218,113,4,95,
    148,175,25,7,24,181,146,129,195,109,38,42,183,30,44,23,143,130,133,122,168,62,113,78,105,115,69,56,139,44,70,40,93,164,0,0,1,0,198,0,0,5,113,5,182,0,21,0,26,64,14,7,8,18,19,4,0,12,
    20,8,114,10,0,2,114,0,43,50,43,50,17,23,57,48,49,83,51,17,20,14,2,7,51,1,51,17,35,17,52,62,2,55,35,1,35,198,189,3,4,5,2,8,3,11,233,188,4,5,6,1,8,252,243,234,5,182,252,214,49,116,112,
    88,22,4,173,250,74,3,34,54,122,115,89,22,251,76,0,255,255,133,101,37,7,129,6,38,1,177,66,255,6,116,233,5,32,26,132,102,34,206,48,49,130,137,32,196,130,137,32,0,130,137,44,10,0,25,64,
    13,2,7,10,3,4,9,5,2,24,101,136,17,130,129,54,51,17,1,51,1,5,0,242,253,131,205,205,2,110,229,253,150,2,230,253,26,5,182,65,135,5,130,73,36,5,255,234,4,241,130,73,46,29,0,21,64,11,3,
    28,2,114,19,12,9,114,0,8,130,208,131,209,40,48,49,97,35,17,33,14,3,7,24,87,242,13,69,38,5,8,55,4,241,204,254,67,14,31,33,34,16,26,80,134,109,36,71,27,24,55,32,57,68,43,17,12,33,38,
    41,20,3,52,5,5,107,239,238,213,82,136,191,101,13,10,171,10,13,89,149,90,64,202,1,4,1,44,159,88,111,22,89,157,8,88,35,15,133,47,39,5,22,5,182,6,6,1,109,90,143,8,87,255,8,133,47,39,4,
    205,5,203,6,6,0,38,132,31,87,161,12,130,229,36,22,255,236,5,8,132,229,43,23,64,11,23,16,5,29,17,2,114,12,5,66,91,6,66,87,5,32,1,141,226,8,73,1,51,1,30,2,23,51,62,2,55,1,5,8,254,42,
    62,130,185,144,57,106,43,43,99,52,74,104,75,33,253,219,221,1,124,9,20,21,7,8,7,18,19,8,1,77,5,182,251,227,140,192,97,17,13,193,19,20,47,97,76,4,54,252,253,16,46,48,22,18,48,48,18,3,
    3,70,229,6,33,6,25,131,159,33,1,114,132,159,87,123,15,36,196,254,119,5,208,130,159,33,11,0,130,159,57,10,5,2,114,11,7,7,1,4,8,114,0,43,204,51,17,51,43,50,48,49,101,17,35,17,33,68,208,
    5,44,17,5,208,197,251,185,205,2,186,203,174,253,201,68,208,7,34,250,248,0,130,231,36,153,0,0,4,217,130,71,45,19,0,29,64,14,17,14,14,2,5,5,0,19,9,104,117,7,70,59,9,39,97,35,17,6,6,35,
    34,38,73,236,5,8,36,51,50,54,55,17,51,4,217,204,120,211,123,207,223,204,122,138,114,189,117,204,2,84,43,50,193,180,2,74,253,222,117,119,43,41,2,186,66,117,5,33,7,147,135,165,40,11,
    7,2,2,114,8,4,4,1,131,165,32,50,131,164,69,119,5,70,148,5,130,170,42,7,147,249,49,205,2,51,205,2,52,206,69,115,5,35,250,253,5,3,131,69,35,254,119,8,57,130,69,37,15,0,27,64,13,14,131,
    236,32,15,140,237,130,238,142,239,130,76,44,17,8,57,197,249,80,205,2,41,207,2,42,205,139,246,130,83,130,250,38,2,0,11,0,0,5,44,130,85,32,13,69,189,5,32,23,130,248,33,2,3,69,189,16,
    35,97,17,33,53,71,222,9,69,189,9,63,1,103,254,164,2,40,210,201,241,109,254,254,254,236,227,210,172,169,83,163,122,183,5,5,177,253,161,108,191,121,202,233,69,191,6,39,0,3,0,196,0,0,
    6,29,130,103,36,11,0,21,0,25,131,193,43,21,3,3,0,23,1,2,114,22,12,0,8,69,23,6,34,17,57,47,71,159,6,70,39,5,32,4,70,39,10,58,1,17,51,17,196,205,201,199,241,108,254,255,254,235,215,200,
    170,168,80,160,120,178,3,192,204,5,182,134,107,44,172,128,135,93,107,44,253,89,5,182,250,74,0,70,151,6,32,174,134,113,34,23,64,11,131,111,130,110,134,109,32,43,155,107,49,196,205,239,
    200,246,112,254,246,254,236,255,238,172,177,87,167,120,213,136,100,134,208,40,1,0,65,255,236,4,159,5,203,74,11,5,42,30,29,29,7,23,16,9,114,0,7,3,68,209,5,32,18,132,201,46,65,34,6,7,
    39,54,54,51,50,4,22,18,21,20,2,69,120,5,8,78,22,22,51,50,62,2,55,33,53,33,46,3,1,238,100,177,75,77,91,221,120,171,1,2,171,86,155,254,199,237,127,183,89,93,182,100,124,188,129,69,3,
    253,67,2,187,7,72,122,167,5,26,48,34,170,42,47,105,196,254,243,164,234,254,166,189,31,32,176,26,37,68,131,191,123,176,109,170,120,62,132,233,58,255,236,7,241,5,205,0,23,0,39,0,33,64,
    18,15,10,10,5,36,20,3,114,13,2,114,12,8,24,113,43,8,32,43,73,2,5,84,57,5,45,6,35,34,36,2,39,33,17,35,17,51,17,33,54,24,110,168,14,8,86,52,2,38,35,34,6,2,7,241,81,161,243,161,207,254,
    231,149,10,254,173,205,205,1,86,16,152,1,19,200,213,1,32,146,251,200,91,191,149,152,193,90,90,190,151,151,192,92,2,221,169,254,235,200,107,172,1,56,210,253,94,5,182,253,157,192,1,30,
    156,187,254,174,225,178,254,252,142,141,1,3,178,178,1,2,139,138,254,255,131,169,49,32,0,0,4,92,5,182,0,14,0,24,0,29,64,14,3,23,23,130,21,34,16,10,2,130,171,32,8,65,57,5,86,128,5,130,
    165,40,1,35,1,46,2,53,52,36,33,131,165,32,17,86,140,6,8,50,51,51,2,126,254,141,235,1,155,75,134,83,1,21,1,12,1,164,205,207,111,153,80,170,175,206,2,87,253,169,2,130,25,94,163,126,201,
    211,250,74,2,87,2,179,52,110,89,129,140,255,255,102,9,5,38,4,97,6,6,0,68,0,130,109,8,35,0,112,255,236,4,101,6,30,0,34,0,50,0,44,64,21,46,42,42,22,17,18,18,22,22,7,35,30,11,114,13,8,
    8,4,7,1,131,138,39,50,17,51,43,50,18,57,125,122,35,7,49,48,49,83,52,18,54,55,54,54,55,23,14,3,7,14,2,7,51,71,43,5,33,21,20,98,80,5,38,1,50,54,54,53,52,38,131,168,8,112,7,20,30,2,112,
    98,208,166,123,253,124,31,61,134,134,121,47,89,126,71,8,13,29,100,140,89,140,191,99,127,228,153,118,187,131,69,2,7,88,127,70,122,136,85,139,96,21,29,69,123,2,149,237,1,79,203,37,31,
    44,18,176,8,21,22,24,13,21,102,187,149,42,85,56,120,221,152,182,247,124,90,175,253,254,159,75,162,132,164,187,73,100,39,96,181,146,85,0,0,3,0,171,0,0,4,85,4,78,0,17,0,26,0,35,71,253,
    5,43,32,32,22,22,12,31,13,6,114,23,12,10,87,205,15,71,253,8,40,6,35,33,17,33,50,22,22,3,130,170,131,9,32,54,133,8,8,67,51,50,54,4,48,118,99,107,147,93,198,160,254,25,1,228,122,187,
    108,172,136,130,254,251,1,9,125,137,32,106,117,254,240,241,126,128,3,54,100,118,19,8,14,123,116,95,146,83,4,78,54,122,253,170,89,79,254,170,80,2,56,71,71,254,220,73,0,1,131,147,33,
    3,95,130,147,96,221,5,34,6,114,4,105,39,8,45,21,33,17,35,17,3,95,254,22,202,4,78,163,252,131,183,38,2,0,37,254,128,4,160,130,45,53,14,0,21,0,27,64,12,16,14,6,10,1,21,21,7,4,128,5,10,
    0,63,26,73,178,5,37,63,51,48,49,65,17,73,178,9,8,56,18,18,55,5,35,6,2,2,7,33,4,1,159,190,252,254,187,88,89,118,64,5,1,171,252,11,63,100,68,1,238,4,78,252,84,253,222,1,128,254,128,2,
    34,125,1,49,1,81,173,157,140,254,228,254,254,101,0,84,147,6,40,39,4,98,6,6,0,72,0,0,73,171,6,32,35,130,127,52,17,0,32,64,18,1,10,16,13,7,4,6,6,17,15,11,6,114,6,3,9,131,185,35,50,50,
    43,50,116,32,5,33,73,2,73,160,6,35,1,1,51,1,130,142,8,33,1,5,252,254,67,1,228,224,254,45,187,254,46,224,1,228,254,66,216,1,180,187,1,181,4,78,253,234,253,200,2,47,253,209,132,3,38,
    56,2,22,253,233,2,23,131,3,130,111,58,70,255,236,3,169,4,98,0,47,0,31,64,15,7,8,34,34,33,33,0,24,16,11,114,41,0,7,68,52,9,71,83,5,33,50,22,91,73,5,80,115,8,78,243,9,33,52,38,24,85,
    46,17,8,74,229,122,188,106,112,96,68,110,66,105,213,165,80,143,117,44,70,191,111,84,134,79,70,139,107,140,119,144,167,124,121,87,151,81,67,89,198,4,98,66,129,95,99,118,26,9,18,64,108,
    81,92,150,87,17,32,22,176,34,52,35,80,65,60,78,38,155,73,88,75,74,37,34,153,38,40,65,197,5,51,4,148,4,78,0,18,0,23,64,11,15,6,16,8,18,6,114,10,16,10,135,163,40,57,48,49,65,17,20,14,
    2,7,73,168,5,8,44,54,54,55,1,35,17,1,108,3,5,4,2,2,65,245,190,3,6,1,253,193,246,4,78,253,140,23,73,80,70,19,3,125,251,178,2,104,37,109,102,28,252,132,4,78,88,11,6,50,148,6,24,6,38,
    1,209,0,0,1,6,2,51,99,0,0,10,179,23,130,99,33,0,43,73,163,5,36,171,0,0,4,69,130,127,46,10,0,25,64,13,5,10,2,3,7,1,8,6,114,4,120,125,12,32,65,75,51,7,45,51,17,3,66,221,254,47,1,247,
    232,254,24,202,202,130,118,38,237,253,197,2,47,253,209,130,9,32,233,130,203,36,10,255,241,4,14,130,75,44,22,0,21,64,11,3,21,6,114,15,8,11,114,88,99,8,73,165,5,36,2,2,6,6,35,78,203,
    7,8,47,62,2,18,55,33,4,14,203,254,192,20,60,91,130,92,33,58,21,15,34,19,41,69,57,47,36,14,2,184,3,172,255,0,254,149,229,107,9,9,156,6,5,67,143,224,1,59,205,65,47,5,33,5,111,130,99,
    42,20,0,27,64,14,19,6,10,3,16,20,130,205,33,9,2,65,50,5,88,202,7,40,65,17,35,17,52,54,55,35,1,130,1,90,35,5,8,48,33,1,1,5,111,183,5,4,6,254,162,163,254,167,6,4,4,184,1,19,1,79,1,84,
    4,78,251,178,2,161,53,107,51,252,140,3,117,51,107,61,253,102,4,78,252,162,3,94,0,133,107,33,4,114,130,107,46,11,0,25,64,12,1,8,8,10,4,11,6,114,6,10,67,238,10,36,48,49,65,17,33,67,49,
    8,54,1,117,2,53,200,200,253,203,202,4,78,254,59,1,197,251,178,1,231,254,25,4,78,87,161,18,36,171,0,0,4,90,130,89,41,7,0,16,183,4,7,6,114,2,6,134,84,131,80,32,35,130,82,38,35,17,4,90,
    202,253,229,130,73,37,251,178,3,169,252,87,132,69,43,171,254,22,4,131,4,98,6,6,0,83,0,67,99,6,33,3,165,132,15,33,70,0,131,175,36,43,0,0,3,195,132,85,39,19,64,9,1,5,5,6,6,24,108,28,
    7,33,17,51,130,88,33,33,17,130,89,49,53,33,3,195,254,150,200,254,154,3,152,3,171,252,85,3,171,163,131,73,38,2,254,19,4,37,4,78,130,89,32,92,130,73,8,40,3,0,107,254,20,5,127,6,20,0,
    21,0,30,0,38,0,37,64,20,31,22,22,1,20,7,114,32,9,9,30,12,11,114,10,15,114,0,0,114,0,43,84,60,6,33,50,50,132,95,32,17,67,64,5,85,123,7,36,54,54,55,17,17,98,30,13,8,79,3,85,171,248,135,
    130,247,177,192,170,248,136,129,249,180,118,155,77,77,155,118,188,118,154,76,175,6,20,254,71,14,143,243,164,162,244,144,15,254,34,1,222,14,143,244,164,165,244,142,13,1,185,253,166,
    11,102,173,117,118,173,102,12,3,38,252,218,13,103,172,117,176,206,255,255,0,32,0,0,4,44,132,177,32,91,132,251,36,171,254,128,4,253,65,171,7,36,9,4,6,114,10,130,204,34,128,3,10,130,
    165,32,26,74,157,7,32,65,74,156,9,63,51,4,253,190,252,108,202,2,35,201,156,254,128,1,128,4,78,252,85,3,171,252,82,0,0,1,0,146,0,0,4,73,130,73,45,19,0,29,64,14,7,4,4,12,15,15,10,9,19,
    130,80,69,95,5,36,17,57,47,51,51,133,239,33,20,22,74,147,5,32,17,74,167,8,8,33,1,91,103,97,100,165,84,201,201,87,179,122,171,191,4,78,254,114,96,91,58,49,1,222,251,178,1,223,53,67,
    176,158,1,153,66,87,6,33,6,154,132,171,43,23,64,11,11,8,3,6,114,9,5,5,2,24,100,76,8,32,50,66,85,7,44,33,17,51,17,33,17,6,154,250,17,201,1,202,130,2,35,4,78,251,178,134,169,130,173,
    132,73,35,254,129,7,56,130,73,32,15,131,171,43,15,12,7,6,114,13,1,9,9,3,128,6,136,247,74,167,5,69,213,7,131,79,132,83,36,153,159,194,250,53,135,85,37,252,82,253,225,1,127,139,89,38,
    2,0,29,0,0,5,41,130,89,32,14,74,169,5,41,16,1,1,11,13,14,6,114,17,11,66,253,15,69,61,5,38,6,35,33,17,33,53,1,130,4,8,49,50,54,53,52,38,2,66,1,36,153,200,98,93,199,160,254,20,254,164,
    3,66,254,227,1,31,118,138,128,4,78,254,65,71,140,104,104,152,84,3,171,163,253,163,254,169,83,96,91,73,0,71,95,5,33,5,164,130,109,32,12,74,175,13,36,6,114,22,13,0,132,112,32,50,73,79,
    7,33,115,17,130,191,135,115,82,72,6,34,35,35,1,130,21,52,171,202,1,6,147,192,93,90,193,155,255,0,240,115,140,130,122,243,3,102,201,137,113,51,154,82,97,92,73,254,14,4,78,251,178,0,
    0,2,0,171,0,0,4,95,134,113,42,23,64,11,17,0,0,10,11,6,114,18,67,220,6,74,175,5,33,65,33,137,220,33,51,1,71,194,7,56,1,117,1,50,148,195,97,93,196,156,254,9,202,2,33,133,122,254,222,
    1,36,112,141,2,143,133,217,35,4,78,252,255,130,104,60,168,83,0,0,1,0,67,255,236,3,146,4,98,0,31,0,25,64,12,13,12,12,0,17,24,7,114,7,0,95,254,10,85,99,12,35,55,33,53,33,88,12,5,34,39,
    54,54,75,149,5,8,56,2,6,1,114,96,144,63,65,148,88,102,146,84,6,253,246,2,9,12,152,149,62,139,56,56,60,167,89,153,234,132,135,244,20,31,30,168,27,39,75,153,117,153,161,156,34,22,154,
    27,39,113,248,206,193,255,0,126,131,227,35,255,236,6,78,130,125,53,22,0,38,0,33,64,18,14,9,9,4,35,19,7,114,12,6,114,11,10,114,27,90,140,6,74,169,12,35,35,34,38,38,74,168,7,73,112,5,
    24,108,28,14,8,69,6,6,78,118,219,152,139,210,124,11,254,243,201,201,1,15,16,124,208,138,146,218,121,252,255,58,124,100,99,124,58,59,123,99,99,124,59,2,41,181,254,255,135,116,226,165,
    254,25,4,78,254,59,151,210,112,135,254,180,131,182,95,94,182,132,130,180,92,92,180,0,130,155,36,26,0,0,3,215,66,95,6,32,29,83,199,5,39,14,1,21,9,6,114,12,1,73,78,12,34,48,49,115,74,
    154,5,8,55,54,51,33,17,35,17,35,3,20,22,51,51,17,33,34,6,254,228,1,56,66,114,70,222,179,1,238,201,250,248,132,120,246,254,234,116,104,1,201,17,76,129,94,160,169,251,178,1,178,1,82,
    91,91,1,98,97,69,125,6,32,39,24,77,133,27,60,0,1,0,16,254,20,4,92,6,20,0,47,0,41,64,21,24,28,28,21,29,29,39,35,14,14,39,25,0,124,54,12,35,47,51,17,51,68,1,9,85,147,8,124,59,11,36,35,
    53,51,53,51,24,66,5,16,8,76,20,6,6,3,50,46,75,27,28,55,32,56,75,117,117,114,135,59,203,155,155,201,1,127,254,129,6,3,12,34,104,131,74,129,178,91,64,132,254,20,15,10,163,10,11,75,98,
    3,39,131,128,92,175,127,253,248,4,209,143,180,180,143,148,50,97,34,58,77,40,84,178,140,252,189,102,150,82,71,217,5,39,3,95,6,33,6,38,1,204,84,163,6,38,52,0,0,0,10,179,13,88,51,7,130,
    211,53,109,255,236,3,189,4,98,0,32,0,25,64,12,19,22,22,0,15,8,7,114,26,66,125,16,24,98,106,14,36,6,7,33,21,33,90,165,8,8,56,2,126,158,239,132,137,242,156,86,163,64,59,61,137,60,99,
    137,77,10,2,8,253,246,7,72,136,101,92,148,67,62,143,20,120,250,195,206,254,117,34,28,157,24,31,71,140,106,153,114,154,77,39,27,167,30,32,0,130,163,32,101,130,127,39,130,4,98,6,6,0,
    86,0,131,15,32,157,130,169,34,134,5,239,130,15,32,76,131,15,32,255,24,78,145,34,38,255,255,255,139,254,20,1,133,53,32,77,130,225,61,2,0,10,255,241,6,108,4,78,0,31,0,40,0,31,64,16,33,
    1,1,18,13,31,6,114,25,18,11,114,34,68,157,7,77,125,8,86,119,6,37,6,6,35,33,17,33,72,123,16,33,1,35,130,31,8,40,54,53,52,38,3,209,235,147,191,94,91,197,158,254,89,254,251,20,60,91,129,
    90,36,57,21,14,34,18,41,70,58,46,37,14,3,76,209,212,114,140,131,68,75,9,50,3,171,255,0,254,149,228,107,9,9,154,4,7,68,143,225,1,59,205,68,205,9,38,2,0,171,0,0,6,182,130,161,53,20,0,
    29,0,35,64,17,1,18,18,22,13,13,15,20,16,6,114,23,11,11,15,69,231,8,66,71,9,140,165,78,38,5,32,17,136,155,55,4,27,232,150,192,93,90,195,159,254,84,254,41,204,204,1,218,1,158,215,216,
    115,139,129,130,142,42,63,70,139,104,104,152,84,1,233,254,23,130,13,34,61,1,195,136,133,47,255,255,0,16,0,0,4,91,6,20,6,6,0,233,0,0,66,49,5,39,4,69,6,33,6,38,1,211,66,49,6,39,119,0,
    0,0,10,179,18,8,90,65,9,42,2,254,19,4,37,6,24,6,38,0,92,130,35,35,6,2,51,203,131,33,33,34,0,134,33,39,0,1,0,171,254,129,4,102,130,219,43,11,0,21,64,10,7,2,6,114,10,9,4,67,177,5,34,
    50,204,43,81,91,13,43,35,2,45,254,126,202,2,40,201,254,133,190,70,92,5,35,251,178,254,129,81,161,5,42,4,40,6,229,0,7,0,16,183,2,0,109,152,8,33,204,51,70,248,5,52,35,17,33,17,4,40,253,
    105,205,2,170,6,229,254,31,250,252,5,182,1,47,130,142,39,0,171,0,0,3,108,5,139,135,55,76,242,7,139,55,48,3,108,254,9,202,2,3,5,139,254,40,252,77,4,78,1,61,119,219,18,37,0,67,2,65,1,
    105,130,249,32,48,119,219,16,32,33,130,249,32,90,130,106,37,7,0,67,1,181,0,131,251,33,49,28,65,29,9,120,35,14,35,0,118,2,231,160,71,33,118,2,130,79,35,0,10,179,49,120,35,10,133,71,
    32,72,130,107,32,58,132,107,34,106,1,100,130,71,35,12,180,63,51,133,144,92,89,5,38,23,0,2,6,70,5,223,136,145,34,106,0,216,130,145,35,12,180,64,52,133,74,133,37,120,111,14,35,0,67,0,
    210,132,147,33,15,7,88,135,6,34,255,255,0,65,213,5,131,219,32,92,132,111,34,67,0,152,130,54,120,113,10,44,0,1,0,82,1,210,3,174,2,123,0,3,0,24,141,73,12,38,82,3,92,1,210,169,169,130,
    54,131,33,32,7,147,33,32,7,133,33,130,103,130,67,131,33,47,6,6,2,2,0,0,0,2,255,252,254,58,3,75,255,194,130,83,38,7,0,12,179,1,2,6,24,134,209,7,34,65,33,53,132,1,37,3,75,252,177,3,79,
    131,3,36,254,58,127,139,126,131,101,47,27,3,193,1,91,5,182,0,10,0,14,181,1,0,128,5,131,197,8,35,26,205,57,48,49,83,39,62,2,55,51,14,2,7,39,12,18,56,66,33,147,20,41,33,11,3,193,22,73,
    167,167,72,78,178,174,71,131,59,32,26,130,59,32,90,130,59,32,11,131,59,34,5,128,11,136,59,33,65,23,130,55,60,35,62,3,55,1,75,15,18,56,67,33,146,15,30,28,23,8,5,182,22,73,166,166,74,
    58,130,134,125,54,130,189,49,65,254,248,1,129,0,237,4,7,2,6,0,39,251,55,0,1,0,131,139,32,92,134,139,35,9,4,128,10,133,79,32,51,130,139,61,30,2,23,35,46,2,39,55,243,11,33,41,20,147,
    33,67,56,18,14,5,182,71,175,178,77,74,166,166,73,22,130,251,131,59,33,2,227,132,59,40,21,0,23,64,10,17,16,16,6,130,65,32,21,131,66,34,50,26,204,75,49,6,131,149,32,39,130,214,32,35,
    135,8,44,2,227,21,40,34,10,203,15,18,57,67,34,246,131,10,43,202,12,18,55,66,34,5,182,78,179,173,71,130,173,33,167,73,136,8,34,0,2,0,130,239,33,2,226,132,99,32,22,131,99,39,1,12,12,
    17,5,128,10,22,135,99,74,165,5,133,250,34,2,55,35,133,8,35,3,55,2,211,130,94,61,66,33,148,20,41,34,10,191,14,17,57,66,33,145,15,30,27,23,7,5,182,22,74,166,165,74,77,178,175,71,132,
    8,65,23,10,39,3,9,0,237,4,7,2,10,65,23,6,40,128,0,0,3,149,6,20,0,11,130,117,44,9,4,1,1,7,10,10,3,8,3,0,47,47,109,123,9,8,45,37,19,35,19,5,53,5,3,51,3,37,3,149,254,165,49,211,48,254,
    184,1,72,48,211,49,1,91,3,216,29,252,11,3,245,29,184,29,1,161,254,95,29,0,1,0,119,130,79,32,157,130,79,47,21,0,39,64,17,9,0,0,6,3,11,20,20,14,17,3,130,1,34,5,15,5,132,87,36,57,47,47,
    17,51,130,208,132,4,36,48,49,65,37,21,135,97,135,102,130,16,46,2,66,1,91,254,165,48,212,47,254,170,1,86,41,41,131,5,34,47,212,48,131,19,41,41,1,251,28,181,28,254,130,1,126,130,6,43,
    1,23,1,8,28,181,29,1,126,254,130,29,130,20,32,248,130,129,52,153,1,232,2,105,3,239,0,15,0,8,177,4,12,0,47,51,48,49,83,52,73,130,7,44,6,6,35,34,38,38,153,61,105,66,66,105,61,133,5,8,
    34,2,236,94,114,51,51,115,93,91,115,54,54,114,255,255,0,142,255,228,5,211,0,253,4,38,0,17,0,0,0,39,0,17,2,31,130,7,36,7,0,17,4,59,132,7,36,92,255,237,9,58,24,147,243,12,49,63,0,75,
    0,44,64,23,64,52,70,58,13,114,40,28,46,34,13,24,147,253,24,35,43,50,204,50,130,242,35,50,22,21,20,131,137,24,148,1,42,32,37,150,51,34,1,145,156,24,148,25,36,53,2,136,154,161,153,162,
    151,158,150,159,74,68,68,74,75,73,73,5,203,239,218,218,24,148,42,37,32,143,24,148,60,17,52,1,0,84,3,168,2,34,5,182,0,3,0,10,179,2,1,2,114,0,43,205,130,217,46,51,1,35,1,93,197,254,182,
    132,5,182,253,242,0,2,131,37,33,3,157,132,37,39,7,0,14,181,7,0,2,6,131,41,34,50,206,50,130,43,35,1,51,1,33,130,3,42,1,206,1,10,197,254,181,254,2,1,9,130,56,130,85,35,14,253,242,2,130,
    3,8,47,0,1,0,80,0,115,2,60,3,213,0,6,0,16,183,4,6,3,0,2,5,1,5,0,47,204,23,57,48,49,83,1,23,1,1,7,1,80,1,92,144,254,227,1,29,144,254,164,24,105,212,11,130,59,32,78,130,59,32,58,135,
    59,130,56,33,6,5,130,170,135,59,49,21,1,39,1,1,221,1,93,254,163,143,1,28,254,228,3,213,254,24,100,110,8,32,0,66,3,5,47,3,133,5,182,4,38,0,4,0,0,0,7,0,4,1,237,130,7,37,1,254,129,0,0,
    2,130,23,130,72,36,11,180,2,18,114,130,223,33,63,43,85,113,5,38,2,133,252,162,166,3,93,24,127,119,8,8,32,112,2,76,2,213,4,237,0,20,0,25,64,10,18,17,9,9,14,4,0,16,16,0,0,47,50,47,16,
    204,50,51,17,51,24,120,166,18,8,68,51,23,51,54,54,1,212,124,133,130,75,78,111,89,130,103,18,8,34,122,4,237,115,128,254,82,1,159,79,80,122,115,254,175,2,149,92,52,52,0,1,0,79,0,0,4,
    36,5,182,0,17,0,32,64,15,1,16,16,4,13,9,12,12,0,8,5,4,114,0,24,110,121,8,32,206,68,152,5,40,115,17,35,53,51,17,33,21,33,131,3,32,21,131,7,59,249,170,170,3,43,253,157,2,61,253,195,1,
    63,254,193,1,14,137,4,31,176,254,34,176,225,137,254,242,130,91,32,73,130,91,8,33,79,5,202,0,41,0,44,64,21,11,37,37,14,34,18,30,30,15,33,33,25,26,22,22,25,12,7,0,5,114,0,43,50,63,75,
    59,9,134,103,32,65,109,173,9,131,101,131,3,113,33,10,130,131,32,53,131,3,49,52,54,54,2,182,113,179,71,67,68,144,79,104,120,1,143,254,113,130,3,43,111,34,63,46,3,5,251,250,67,89,46,
    195,130,0,8,39,92,187,5,202,46,33,156,29,38,114,130,162,137,161,139,78,114,76,24,179,168,16,75,124,88,139,161,137,133,144,195,102,0,3,0,165,255,236,6,34,130,255,53,12,0,21,0,46,0,45,
    64,22,30,37,11,114,44,45,45,23,42,26,26,46,23,24,133,212,8,41,0,47,43,50,18,57,47,51,47,205,131,174,90,215,6,37,65,32,22,21,20,14,24,133,221,11,39,53,52,38,5,21,51,21,35,103,68,15,
    8,111,35,53,55,55,1,198,1,18,248,56,128,213,157,61,196,1,24,84,57,181,178,161,2,180,238,238,60,62,35,79,26,29,95,63,80,116,63,155,162,66,5,182,226,208,95,169,130,74,253,208,5,182,173,
    253,213,133,152,137,133,224,219,148,254,106,76,79,14,9,145,13,22,61,129,102,1,170,85,76,206,0,1,0,56,255,236,4,121,5,200,0,54,0,43,64,21,51,48,48,15,39,24,24,42,21,12,15,15,0,28,35,
    13,114,65,91,6,34,43,50,17,130,179,35,206,50,50,17,132,181,104,236,10,43,14,2,7,33,21,33,20,6,21,20,22,23,75,216,11,45,35,34,38,38,39,35,53,51,38,38,53,52,54,55,130,8,8,42,62,2,3,19,
    102,174,82,77,60,144,77,73,123,95,65,17,1,223,254,19,2,1,1,1,180,254,92,21,100,159,108,79,153,63,61,150,96,163,237,148,29,159,143,130,21,8,62,1,143,157,25,149,240,5,200,44,46,160,31,
    47,47,94,140,93,137,17,38,21,20,41,21,138,110,155,81,36,28,176,28,35,124,233,164,138,21,36,24,21,43,13,137,167,246,133,0,4,0,117,255,244,6,10,5,193,0,3,0,15,0,27,130,205,60,37,64,18,
    48,28,35,42,3,3,35,4,114,25,13,7,19,1,1,7,12,114,0,43,50,47,50,16,204,50,134,6,67,39,5,33,1,20,68,247,6,34,51,50,22,24,104,111,11,32,1,24,111,37,25,8,49,5,26,252,213,170,3,43,1,154,
    175,146,136,178,173,147,136,179,254,26,79,90,87,79,77,89,90,79,253,174,151,198,93,162,101,54,104,41,39,39,86,38,104,99,97,100,55,100,40,38,103,67,133,5,8,73,251,155,167,182,181,168,
    167,181,180,168,106,125,125,106,106,123,123,1,79,167,176,125,155,72,22,17,115,15,19,120,110,111,115,20,17,116,19,21,0,2,0,94,255,237,3,170,5,203,0,35,0,45,0,34,64,19,16,13,32,17,20,
    43,6,8,39,25,5,114,3,3,0,0,8,13,131,202,35,17,51,47,43,86,211,5,52,101,50,54,55,51,14,2,35,34,38,38,53,53,6,6,7,53,54,54,55,17,70,97,5,33,21,20,130,15,8,132,17,20,22,19,52,38,35,34,
    6,21,17,54,54,2,121,72,98,8,127,5,72,140,109,92,149,86,45,97,49,52,95,44,66,138,107,129,151,100,181,121,76,165,59,60,71,51,120,121,130,98,114,114,162,85,74,160,130,212,15,28,13,134,
    14,30,15,1,217,96,141,79,161,145,132,214,160,54,254,244,102,117,4,17,89,91,97,83,254,105,61,207,0,4,0,182,0,0,7,204,5,182,0,19,0,23,0,37,0,49,0,38,64,18,44,31,38,24,20,21,13,3,0,9,
    9,1,2,114,12,12,0,8,0,63,50,130,172,8,34,47,17,57,57,47,51,222,50,204,50,48,49,115,17,51,1,51,46,2,53,17,51,17,35,1,35,30,2,21,17,33,53,33,21,1,131,194,32,52,80,157,6,32,6,124,153,
    11,8,89,182,223,2,129,10,3,8,5,180,220,253,121,10,4,8,5,4,22,2,36,254,237,90,141,79,169,147,90,141,80,171,142,86,79,79,86,87,81,80,5,182,251,106,51,139,140,53,3,23,250,74,4,155,54,
    144,142,55,252,240,145,145,1,18,82,157,113,168,181,81,155,113,170,182,125,115,112,112,110,110,112,112,115,0,2,0,32,2,229,5,139,130,187,8,34,20,0,28,0,43,64,21,27,23,23,3,15,12,3,24,
    7,14,14,0,0,21,24,5,2,2,24,2,114,0,43,50,47,51,16,204,66,251,5,33,23,57,72,247,5,35,17,51,19,19,130,184,37,17,52,54,55,35,3,130,1,33,22,22,130,191,8,43,17,35,53,33,21,35,17,2,148,182,
    197,203,177,124,4,1,7,210,104,200,8,2,3,253,233,209,2,30,210,2,229,2,209,253,206,2,50,253,47,1,159,22,97,28,131,10,44,34,85,18,254,87,2,104,105,105,253,152,255,255,117,255,7,56,6,6,
    1,117,0,0,0,2,0,102,255,221,4,139,4,72,0,25,0,34,0,25,64,12,34,130,147,36,30,9,6,114,18,79,97,7,33,18,57,103,67,6,34,2,53,52,81,77,5,33,21,33,115,153,5,37,55,23,14,2,19,17,81,234,5,
    8,106,17,2,121,173,237,121,93,156,188,94,151,239,140,252,197,44,161,92,149,177,69,72,48,120,172,172,38,157,106,101,147,47,35,160,1,2,147,148,214,138,66,138,253,175,254,156,47,76,123,
    111,41,76,127,76,2,139,1,21,40,79,71,46,254,233,0,0,5,0,60,255,240,6,35,5,182,0,3,0,17,0,31,0,56,0,68,0,46,64,25,57,51,39,25,4,63,45,32,18,3,3,32,13,114,13,14,9,3,24,106,230,17,37,
    50,16,204,50,23,57,24,106,55,8,56,54,54,55,6,6,7,7,39,37,51,17,1,50,54,53,52,38,39,39,14,2,21,20,22,23,67,105,5,32,55,24,66,130,11,37,22,22,21,20,6,3,91,134,7,130,36,8,56,249,3,94,
    167,252,163,91,2,4,2,20,49,26,102,77,1,21,150,3,0,81,80,85,76,23,44,60,30,79,82,152,163,42,73,45,63,70,170,118,117,165,85,64,80,99,173,138,58,71,69,63,62,69,76,5,182,250,24,107,17,
    16,8,89,254,29,72,57,55,77,24,9,20,47,60,38,56,73,119,134,114,57,84,63,23,39,93,77,109,116,110,110,76,99,33,35,108,84,114,140,2,29,23,66,54,49,58,58,49,53,66,0,0,5,0,43,255,240,6,71,
    5,201,0,3,0,45,0,59,0,84,0,96,0,57,64,30,52,53,67,79,85,5,91,73,60,46,3,3,60,13,114,40,39,20,24,106,116,28,65,12,7,36,97,1,51,1,3,81,219,9,32,52,90,3,16,33,51,50,117,123,10,65,40,49,
    40,1,65,3,94,166,252,163,177,75,24,111,111,20,8,49,97,138,151,87,69,86,95,172,3,32,81,79,84,77,23,43,60,31,79,82,151,164,43,73,45,63,70,170,117,117,166,85,64,79,100,173,138,58,70,69,
    63,62,69,77,5,182,250,74,2,58,24,106,160,29,33,254,45,65,76,45,8,53,5,0,78,255,239,6,74,5,182,0,3,0,34,0,48,0,73,0,85,0,61,64,32,41,42,56,68,74,5,80,62,49,35,3,3,49,13,114,20,21,21,
    17,17,26,29,29,11,4,22,25,1,1,22,4,69,185,8,87,30,5,33,17,51,69,193,6,35,23,57,48,49,65,79,17,42,34,6,7,39,19,33,21,33,7,54,54,65,75,5,65,68,50,8,45,63,3,94,166,252,163,154,67,141,
    46,55,136,55,91,107,104,96,46,75,28,73,34,1,242,254,138,19,25,60,36,136,182,178,3,26,81,79,84,77,22,44,60,31,79,83,152,65,64,16,33,71,70,65,64,8,8,116,56,28,26,144,33,39,79,86,76,83,
    15,8,40,1,171,125,209,5,8,143,130,142,158,254,47,71,57,56,76,24,9,19,47,60,39,56,72,120,135,113,57,85,62,23,40,92,78,108,117,110,111,76,99,32,35,109,84,113,141,2,29,23,66,54,50,58,
    58,50,52,66,0,0,5,0,85,255,240,6,48,5,182,0,3,0,10,0,24,0,48,0,60,0,45,64,24,17,18,31,43,49,5,55,37,25,11,3,3,25,13,114,9,6,4,7,1,1,7,65,49,5,34,16,204,51,65,41,11,42,115,1,51,1,3,
    1,33,53,33,21,1,65,17,18,67,126,29,48,231,3,94,167,252,162,209,1,93,254,59,2,105,254,167,3,143,130,253,36,76,24,44,60,30,130,253,34,163,93,67,66,61,7,43,80,99,173,139,59,70,68,63,63,
    68,75,5,67,122,5,36,232,132,108,253,0,67,114,16,34,85,107,35,67,113,27,61,2,0,91,255,236,4,62,5,201,0,38,0,54,0,31,64,15,52,24,39,39,21,21,0,47,11,19,114,31,0,97,152,10,74,181,6,36,
    30,2,21,20,2,71,0,6,51,52,62,3,51,50,22,23,54,54,53,46,2,35,34,6,7,53,62,2,19,115,76,6,34,50,62,2,130,229,8,98,2,138,125,168,100,43,45,94,148,204,132,141,163,68,36,76,124,176,116,84,
    136,41,2,1,1,60,116,84,61,145,58,39,96,105,40,88,127,82,40,81,91,76,126,96,65,15,20,111,5,201,87,153,199,113,124,254,248,247,198,116,113,182,105,77,174,169,137,82,82,66,19,42,15,121,
    163,82,48,39,187,20,30,18,253,151,96,152,169,74,100,127,93,156,191,98,77,103,255,255,0,127,237,6,8,38,6,6,1,97,0,0,0,1,0,191,254,27,5,43,5,182,0,7,0,14,181,6,1,2,114,4,0,0,47,50,43,
    50,48,49,83,17,33,17,35,130,3,47,191,4,108,207,253,49,254,27,7,155,248,101,6,233,249,23,130,51,36,66,254,27,4,228,130,51,48,11,0,29,64,13,3,7,7,8,2,0,4,2,114,1,9,9,131,59,125,199,8,
    130,66,8,39,53,1,1,53,33,21,33,1,1,33,21,66,2,106,253,167,4,82,252,198,2,46,253,186,3,145,254,27,121,3,135,3,34,121,175,253,25,252,171,176,130,83,38,99,2,132,4,46,3,32,81,135,15,39,
    99,3,203,2,132,156,156,0,130,33,8,66,37,255,242,4,206,6,170,0,8,0,23,64,10,2,3,5,2,3,3,6,0,19,114,0,43,47,57,47,17,57,17,51,48,49,69,1,35,53,33,19,1,51,1,1,237,254,236,180,1,39,230,
    1,254,158,253,171,14,3,3,153,253,110,5,174,249,72,0,130,93,8,40,116,1,141,5,48,4,19,0,26,0,38,0,50,0,39,64,19,30,18,18,7,45,27,21,4,4,42,24,24,4,36,10,10,48,4,18,0,63,51,51,17,130,
    1,34,47,51,18,71,85,7,38,20,6,6,35,34,38,39,132,5,86,55,8,50,54,54,51,50,22,5,38,38,35,34,6,21,20,22,51,50,54,37,52,131,11,33,7,22,131,11,8,101,5,48,76,135,91,90,152,65,61,152,85,90,
    137,78,76,138,91,85,152,63,59,153,94,134,167,253,84,49,102,66,77,88,84,82,63,104,2,75,91,76,61,103,52,48,106,64,75,90,2,207,87,147,88,101,110,99,107,81,144,96,89,145,86,99,110,98,107,
    177,139,93,85,103,76,72,105,87,88,76,101,85,92,90,88,102,0,1,0,7,254,20,3,12,6,20,0,29,0,14,181,22,15,0,24,73,206,7,33,48,49,24,134,216,11,73,96,5,33,23,21,132,154,8,72,6,21,17,20,
    6,158,41,80,30,26,69,36,87,75,82,152,104,39,76,27,25,66,34,55,71,32,185,254,20,15,11,166,10,17,115,101,5,19,130,162,76,12,11,167,10,16,53,97,67,250,237,194,174,255,255,0,99,1,130,4,
    45,4,36,6,39,0,97,0,0,0,199,2,7,131,7,33,255,59,130,131,8,42,99,0,160,4,44,5,8,0,19,0,39,64,17,1,0,0,2,19,19,5,16,15,11,10,10,9,12,12,6,15,0,47,51,51,17,51,51,47,51,16,206,50,50,132,
    8,39,48,49,65,23,7,33,21,33,131,3,37,3,39,55,33,53,33,131,3,8,35,2,250,143,107,1,14,254,171,118,1,203,253,236,130,142,104,254,243,1,85,115,254,56,2,16,5,8,64,223,156,248,156,254,231,
    62,219,130,6,131,137,41,255,255,4,46,4,245,6,38,0,31,130,115,38,7,2,42,0,0,253,124,140,23,32,33,137,23,8,32,0,2,0,103,0,0,4,64,5,193,0,5,0,9,0,25,64,13,4,9,1,7,4,0,8,2,2,114,6,0,8,
    0,63,24,148,8,9,8,40,1,51,1,1,39,9,2,2,42,254,61,1,195,83,1,195,254,61,41,1,47,254,209,254,211,2,223,2,226,253,30,253,33,231,1,248,1,249,254,7,0,130,243,49,189,4,217,3,222,6,24,0,15,
    0,18,183,15,9,128,12,15,4,24,71,55,16,8,63,38,39,51,22,22,51,50,54,55,3,222,9,87,172,138,142,168,78,7,185,11,96,107,94,111,11,6,24,102,143,74,72,142,105,111,78,83,106,0,1,1,126,4,205,
    2,133,6,20,0,12,0,14,180,1,7,12,128,5,0,47,26,205,57,57,24,69,34,9,8,44,3,55,2,133,11,48,59,32,113,11,20,19,15,4,6,20,18,40,109,112,48,24,29,76,85,80,33,0,255,255,255,119,254,59,0,
    137,255,131,4,7,4,113,254,9,0,130,157,41,1,116,4,216,2,135,6,32,0,11,131,81,41,11,128,7,6,0,47,51,26,205,50,137,81,8,60,2,55,2,135,14,34,26,6,195,14,48,63,37,6,32,24,35,109,116,44,
    20,40,108,113,47,0,0,2,0,19,3,84,2,197,6,199,0,10,0,19,0,29,64,13,6,11,11,9,9,4,1,1,3,120,15,7,119,0,63,51,228,70,218,7,130,78,44,35,21,35,53,33,53,1,51,17,51,33,53,52,73,52,5,8,63,
    2,197,127,166,254,115,1,144,163,127,254,219,3,3,11,58,21,165,4,20,192,192,112,2,67,253,204,196,44,106,49,26,96,32,241,0,1,0,68,3,67,2,155,6,193,0,30,0,31,64,14,29,28,28,25,25,3,6,6,
    19,12,120,2,30,132,98,32,50,138,99,71,30,10,97,49,8,100,244,6,8,54,7,39,19,2,109,254,138,18,25,60,36,136,181,177,168,68,139,47,54,137,55,91,107,103,96,47,74,30,72,34,6,193,126,208,
    4,8,142,130,143,157,27,26,144,33,39,80,85,77,82,14,8,39,1,172,0,130,127,36,58,3,84,2,163,130,127,41,6,0,16,182,5,1,1,6,120,3,130,119,32,228,130,199,34,48,49,83,70,120,5,34,161,1,94,
    70,65,6,8,37,84,2,233,132,108,252,255,0,3,0,50,3,68,2,166,6,209,0,25,0,39,0,51,0,23,64,12,39,26,20,6,46,5,33,12,120,40,0,130,63,35,50,228,50,23,119,96,5,33,21,20,24,125,245,9,38,38,
    53,52,54,54,55,38,131,6,125,115,5,32,51,74,82,5,39,19,34,6,21,20,22,23,54,131,203,33,1,109,72,209,6,35,140,100,142,73,72,227,5,51,91,44,60,31,79,79,81,79,84,77,2,62,69,77,58,58,71,
    70,6,209,73,246,9,34,60,112,76,74,12,8,33,254,16,74,32,5,8,40,72,57,55,76,25,1,131,58,49,53,66,24,23,66,54,49,58,0,22,0,84,254,129,7,193,5,238,0,5,0,11,0,17,0,23,0,27,0,31,0,35,130,
    189,34,43,0,47,130,193,8,45,55,0,59,0,63,0,67,0,71,0,83,0,95,0,111,0,120,0,129,0,144,0,39,64,18,18,24,24,44,44,28,28,12,19,7,57,57,53,53,33,33,1,17,0,63,51,79,83,5,135,7,60,48,49,83,
    17,33,21,35,21,37,53,33,17,35,53,1,17,51,21,51,21,33,53,51,53,51,17,33,53,33,130,9,34,33,21,1,130,7,33,1,35,130,53,130,3,135,11,131,7,32,51,134,23,32,53,131,23,130,3,32,5,78,215,22,
    32,37,74,95,12,33,35,35,24,155,77,8,33,21,21,65,66,5,34,35,1,34,115,150,6,32,53,130,101,8,33,20,6,84,1,47,192,5,206,1,48,109,249,0,111,192,5,14,195,109,253,73,1,17,251,225,1,14,254,
    242,1,14,4,183,109,130,0,49,251,194,1,16,252,48,111,111,2,192,1,16,119,1,17,250,168,111,130,0,41,6,254,109,109,251,159,127,135,135,127,131,3,8,65,254,115,66,69,71,64,64,71,69,66,1,
    225,172,110,111,46,44,45,62,109,94,207,123,66,46,36,42,47,59,74,49,37,38,52,1,94,48,32,16,32,20,37,49,125,111,4,190,1,48,111,193,193,111,254,208,193,249,2,1,47,194,109,109,194,254,
    209,130,103,50,109,6,254,111,111,250,168,1,14,2,2,1,15,250,59,109,109,1,166,130,130,32,74,130,108,8,76,111,252,47,1,16,121,1,15,253,104,1,16,73,135,166,166,135,137,164,164,137,92,105,
    105,92,92,104,104,201,67,83,49,66,8,7,9,58,69,80,90,1,98,34,32,34,29,227,154,43,37,32,42,254,250,10,102,3,5,36,50,1,146,254,114,101,93,0,0,3,0,84,254,193,7,170,6,20,130,9,8,36,33,0,
    45,0,23,64,9,33,37,2,23,15,43,43,2,0,0,47,47,57,47,57,57,18,57,51,48,49,73,3,5,53,52,54,55,62,2,104,35,7,33,23,54,74,104,6,36,7,6,6,21,21,24,161,155,8,8,86,35,34,6,3,254,3,172,252,
    84,252,86,3,235,42,67,59,78,39,189,163,57,121,112,46,82,68,127,55,63,62,53,68,76,67,27,81,60,56,83,83,56,60,81,6,20,252,86,252,87,3,169,251,47,50,62,52,47,84,98,67,137,152,27,44,27,
    178,34,46,58,47,58,71,53,61,113,80,59,254,237,72,63,63,72,76,61,61,0,92,181,5,55,2,122,6,32,6,38,3,176,0,0,1,7,1,75,255,86,0,0,0,10,179,17,11,6,24,93,75,8,88,175,6,35,6,6,2,6,130,27,
    59,2,0,19,255,236,5,2,6,33,0,59,0,71,0,46,64,21,31,28,28,32,35,5,67,67,8,52,35,130,1,37,0,45,16,9,114,60,130,229,39,50,43,50,17,57,57,47,125,87,16,13,48,50,30,2,23,51,21,35,22,22,21,
    20,2,6,6,35,34,46,130,250,32,54,68,45,8,135,253,71,159,6,45,18,53,52,38,39,38,36,38,53,52,54,54,23,34,131,20,8,125,22,23,46,3,2,104,116,179,128,81,18,144,131,2,1,65,137,216,150,112,
    149,89,37,13,12,35,32,25,51,16,46,41,109,62,95,87,12,14,98,111,117,158,80,2,2,253,254,205,139,78,163,125,85,85,97,217,179,12,58,84,107,6,33,82,153,218,136,162,20,56,27,155,254,244,
    200,112,57,100,126,68,52,101,91,33,46,36,18,9,134,22,31,102,82,46,102,110,59,85,123,128,1,4,195,20,54,19,3,118,197,120,88,142,84,161,82,76,75,121,72,2,102,160,110,56,0,1,130,253,55,
    0,4,162,5,195,0,29,0,27,64,15,24,27,0,3,26,28,2,114,26,8,114,16,9,74,73,5,44,43,18,23,57,48,49,65,62,3,55,62,2,51,24,164,87,8,8,114,7,14,3,7,17,35,17,1,51,2,74,34,72,71,64,26,29,63,
    81,55,32,52,21,12,33,16,30,55,37,24,77,89,89,36,207,254,31,224,2,240,78,164,157,131,45,50,65,33,9,10,157,3,5,34,62,40,143,183,201,97,253,221,2,47,3,135,0,0,2,0,29,255,236,6,208,4,78,
    0,27,0,52,0,45,64,23,40,40,17,2,28,28,24,24,26,27,6,114,13,14,36,36,17,11,114,46,10,11,114,0,43,50,43,50,17,51,122,117,5,36,17,51,17,57,47,130,143,36,21,35,30,2,21,73,18,6,35,35,6,
    6,35,79,37,6,37,33,53,55,5,33,14,130,25,41,22,51,50,54,53,53,51,21,20,22,132,9,8,86,52,38,38,6,208,251,31,45,25,90,179,133,117,146,34,8,35,145,116,195,206,29,51,32,254,239,153,4,94,
    252,218,30,49,30,115,107,98,92,190,44,85,63,106,113,24,43,4,78,162,74,157,160,80,158,218,113,100,93,93,100,247,242,80,161,157,73,87,75,162,69,154,159,80,183,152,147,119,185,185,85,
    119,62,154,180,79,159,154,112,121,5,53,6,132,7,138,6,38,0,48,0,0,1,7,0,118,2,207,1,105,0,10,179,31,24,86,91,10,38,171,0,0,6,226,6,33,130,35,32,80,133,35,40,3,3,0,0,0,10,179,47,33,24,
    69,217,9,36,0,253,200,5,43,24,105,23,9,47,2,83,1,68,0,0,255,255,0,92,253,200,3,231,4,97,130,59,130,15,37,1,7,2,83,0,213,130,59,8,50,16,181,3,2,47,11,1,1,184,255,150,176,86,0,43,52,
    52,0,2,0,110,253,200,2,59,255,130,0,11,0,23,0,16,180,12,0,192,18,6,0,124,47,51,26,24,204,50,48,49,65,34,24,82,194,9,82,57,11,8,35,1,82,101,127,126,102,97,136,134,99,50,64,67,47,50,
    64,58,253,200,118,103,103,118,118,101,105,118,108,61,52,54,60,60,54,52,61,130,91,56,124,255,236,6,127,6,20,0,28,0,43,0,27,64,13,23,21,17,26,5,40,14,3,114,33,113,35,10,32,206,131,102,
    67,2,5,8,79,38,38,2,53,52,18,36,51,50,4,23,62,2,53,51,23,6,6,7,22,22,5,20,18,22,51,50,54,18,53,16,2,35,34,6,2,5,197,84,170,252,169,173,255,168,82,148,1,47,231,169,1,0,84,50,54,22,208,
    14,24,121,126,43,42,251,142,98,205,160,162,203,95,219,238,161,206,99,2,221,169,24,75,22,12,45,112,101,18,79,115,72,21,136,200,49,86,209,122,178,24,156,51,7,38,13,1,50,139,254,254,0,
    126,1,5,39,5,69,4,246,0,26,0,41,131,171,41,20,18,14,24,4,39,11,7,114,31,99,118,6,35,50,18,57,57,135,171,33,35,34,130,169,36,16,0,51,50,22,133,168,33,14,2,132,168,32,22,131,168,36,54,
    53,52,38,38,130,169,8,110,4,110,125,232,159,149,230,130,1,20,239,110,186,66,58,62,22,207,14,17,64,112,88,32,34,252,205,64,136,107,107,135,63,63,135,109,159,146,2,41,182,255,0,135,135,
    1,0,182,1,16,1,41,73,68,18,81,118,72,22,95,152,107,30,64,156,91,133,182,93,94,182,132,131,179,92,204,0,0,1,0,182,255,236,6,164,6,20,0,31,0,29,64,14,7,5,31,31,1,12,17,21,2,114,26,17,
    9,114,0,43,50,43,130,157,33,51,47,132,159,48,21,62,2,53,51,23,14,3,7,17,20,6,6,35,32,0,53,24,154,143,12,8,54,61,68,28,206,14,13,48,84,133,99,121,250,195,254,232,254,217,206,188,186,
    126,158,73,5,182,197,13,77,121,80,21,76,133,106,73,16,253,153,154,242,140,1,39,245,3,174,252,83,179,187,90,165,110,3,174,131,129,8,32,161,255,236,5,200,4,247,0,34,0,41,64,21,1,0,28,
    28,30,6,14,19,6,114,9,10,10,24,24,14,11,114,7,10,130,136,33,43,50,24,75,16,8,37,47,204,50,48,49,65,132,136,34,35,39,35,78,73,6,135,139,130,148,131,167,8,58,5,186,14,12,48,84,132,97,
    161,28,11,35,106,133,74,129,178,91,202,115,118,172,137,202,60,66,26,4,247,22,75,136,109,72,12,252,179,151,57,76,38,84,177,140,2,207,253,77,133,132,203,190,2,51,121,14,78,122,78,130,
    145,50,252,250,4,187,254,126,6,150,0,21,0,21,64,9,6,3,12,19,192,74,221,6,32,26,24,127,162,8,35,7,7,35,39,69,86,8,8,42,53,54,54,51,50,22,254,126,91,74,10,121,16,77,79,69,52,31,58,22,
    22,67,41,126,132,5,215,77,88,18,101,166,13,50,46,44,36,7,6,117,7,9,96,24,104,95,17,34,0,67,0,24,122,27,17,39,198,0,0,5,113,7,138,6,117,85,6,42,0,67,1,180,1,105,0,10,179,28,0,24,107,
    71,12,34,4,39,6,24,114,191,29,36,171,0,0,4,148,131,35,33,1,209,130,216,32,7,130,71,38,48,0,0,0,10,179,25,107,249,10,58,50,255,248,7,52,5,182,0,43,0,30,64,16,31,20,20,26,11,42,16,4,
    0,5,2,114,37,0,66,3,6,77,244,6,47,69,38,10,2,39,51,22,26,2,23,51,54,54,55,19,46,136,12,8,142,18,18,19,51,10,2,7,35,46,3,39,1,1,230,85,152,119,73,7,213,9,63,90,101,48,10,13,49,30,186,
    12,21,13,2,214,6,61,94,113,57,8,79,117,65,2,214,4,90,172,127,186,49,94,85,71,26,254,254,8,144,1,90,1,126,1,143,199,187,254,156,254,188,254,238,104,63,162,82,2,0,71,145,143,67,170,254,
    164,254,181,254,220,115,161,1,129,1,196,1,2,254,231,253,247,254,45,201,79,187,199,195,87,253,21,0,1,0,39,0,0,6,60,4,79,0,40,0,31,64,17,40,30,30,35,21,26,9,27,5,11,16,6,114,5,99,252,
    6,78,171,8,33,6,2,131,155,38,2,39,3,35,38,38,2,131,180,44,18,18,23,51,62,2,55,19,38,38,39,51,22,131,12,8,127,54,18,18,55,6,60,9,86,159,119,185,40,83,72,26,237,181,65,125,102,63,5,200,
    11,80,109,53,7,12,36,39,17,146,25,27,3,201,6,66,101,57,8,71,112,70,10,4,79,199,254,142,254,156,178,71,163,170,78,254,30,103,252,1,29,1,50,157,193,254,165,254,226,102,35,72,74,38,1,
    36,98,219,100,168,254,187,254,209,136,116,1,34,1,82,188,0,2,0,15,0,0,4,255,5,182,0,19,0,29,0,36,64,10,7,10,10,4,1,29,11,11,0,5,184,255,255,180,114,20,115,139,5,48,43,18,57,47,51,205,
    50,50,17,51,48,49,97,17,33,53,33,102,62,6,115,150,18,8,44,65,254,206,1,50,207,1,153,254,103,184,208,249,110,254,253,254,225,205,187,181,172,83,170,128,159,4,66,165,207,207,165,235,
    108,191,121,202,233,172,128,135,93,106,45,0,134,127,34,184,5,39,130,127,59,28,0,33,64,16,21,5,5,1,22,14,10,114,16,4,4,19,17,1,6,114,0,43,50,205,51,17,51,90,60,5,52,48,49,65,21,33,21,
    33,17,33,50,22,21,20,6,6,35,33,17,35,53,51,105,210,9,8,43,1,206,1,95,254,161,1,38,230,222,93,200,160,254,20,248,248,1,225,254,230,1,27,119,145,136,5,39,217,162,254,225,158,155,104,
    152,84,3,172,162,217,252,202,254,105,215,6,8,32,1,0,196,255,236,7,48,5,203,0,42,0,39,64,21,11,28,28,4,31,31,0,23,16,3,114,8,2,114,7,8,114,36,66,104,6,33,43,43,24,77,181,9,32,69,115,
    42,10,41,62,3,55,50,22,23,7,38,38,35,125,230,15,8,111,5,119,222,254,217,152,9,254,192,205,205,1,70,16,106,178,248,158,113,208,86,76,75,168,95,105,169,123,76,13,2,168,253,84,6,65,122,
    177,115,99,176,92,86,182,20,172,1,55,210,253,95,5,182,253,156,142,232,167,91,1,52,43,171,36,53,60,117,169,110,176,120,191,136,71,35,28,176,32,31,0,1,0,171,255,236,5,201,4,98,0,39,0,
    38,64,20,22,15,7,114,26,10,10,29,5,5,7,8,6,114,7,10,33,87,209,5,38,63,43,18,57,47,51,51,118,234,5,32,69,105,42,15,133,168,32,6,103,62,13,8,72,4,151,149,226,133,12,254,230,202,202,1,
    28,17,141,221,138,89,161,57,57,55,130,62,98,134,73,10,2,0,253,255,6,74,138,100,88,142,64,60,141,20,109,225,171,254,27,4,78,254,58,168,209,97,38,26,155,24,31,69,138,103,163,114,152,
    76,39,27,168,30,31,0,2,0,130,0,59,5,123,5,184,0,11,0,24,0,35,64,17,4,7,7,12,11,18,18,9,11,2,114,2,6,6,9,8,90,109,5,131,159,33,18,57,130,160,39,48,49,65,1,35,1,35,17,130,1,130,7,8,45,
    23,14,3,7,7,33,39,46,3,3,42,2,81,210,254,252,139,185,141,254,251,207,2,80,109,6,23,29,29,12,69,1,80,75,11,26,25,23,5,184,250,72,2,154,253,102,131,3,46,5,184,174,22,70,79,77,29,177,
    189,29,67,71,68,0,130,127,38,5,0,0,4,151,4,78,130,127,32,23,140,127,32,6,132,127,32,10,147,127,32,3,132,127,38,3,35,1,23,35,14,2,132,128,8,34,2,2,199,1,208,200,189,105,179,110,185,
    202,1,207,125,8,10,26,27,13,57,1,20,56,13,31,26,4,78,251,178,1,212,254,44,131,3,43,4,78,131,33,72,71,30,142,139,34,77,72,131,123,36,196,0,0,7,139,130,251,53,19,0,32,0,48,64,23,26,17,
    17,12,4,8,8,20,15,12,12,14,19,15,2,131,128,35,10,10,14,8,77,168,6,39,43,50,18,57,47,18,57,51,77,180,5,65,8,13,117,130,6,32,19,65,16,8,8,43,5,59,2,80,214,254,253,136,183,139,254,251,
    211,1,15,254,114,205,205,1,214,249,109,7,22,25,28,14,70,1,73,71,12,26,25,22,5,184,250,72,2,159,253,97,135,3,49,5,182,253,154,2,104,174,27,66,71,71,34,180,188,31,68,69,65,131,163,46,
    171,0,0,6,88,4,78,0,19,0,31,0,46,64,22,134,163,32,21,132,162,65,35,5,34,9,14,10,136,162,34,17,57,47,140,161,65,42,7,32,19,136,161,65,50,8,8,39,4,137,1,207,200,193,102,178,102,195,200,
    203,254,215,189,189,1,110,192,125,7,9,28,29,13,51,1,10,52,13,30,25,4,78,251,178,1,226,254,30,132,3,8,53,227,254,29,4,78,254,55,1,201,131,28,77,79,31,118,128,34,75,71,0,2,0,28,0,0,5,
    213,5,182,0,30,0,33,0,38,64,18,33,2,2,28,28,13,16,16,21,32,30,2,114,8,15,15,21,65,58,5,127,180,8,8,77,17,51,48,49,65,21,1,30,2,23,19,35,3,46,2,39,17,35,17,14,2,7,3,35,19,62,3,55,1,
    53,5,33,1,5,81,254,99,123,151,94,36,141,208,133,29,63,105,91,205,92,104,61,29,133,212,140,27,63,89,129,92,254,107,3,175,253,76,1,90,5,182,129,254,19,12,93,162,116,130,141,60,180,92,
    106,47,3,253,84,2,172,3,47,106,92,254,76,1,201,86,134,96,58,9,1,237,129,179,254,92,0,130,159,42,14,0,0,4,252,4,78,0,29,0,32,131,159,36,32,2,2,27,27,131,159,34,31,29,6,132,159,65,55,
    5,32,43,68,36,7,153,159,32,2,133,159,8,73,4,133,254,187,99,116,72,31,126,190,119,25,53,82,70,181,74,82,50,26,121,189,126,32,72,118,97,254,188,3,7,253,240,1,7,4,78,102,254,150,12,78,
    124,85,254,173,1,65,67,77,35,2,254,10,1,246,1,34,78,68,254,191,1,83,83,125,78,14,1,105,102,155,254,215,66,125,7,54,238,5,182,0,35,0,38,0,63,64,31,1,34,34,37,35,35,30,38,2,2,32,32,131,
    165,45,27,27,29,30,2,114,29,8,8,15,15,22,22,28,65,74,5,33,17,51,68,42,5,66,131,5,133,5,67,149,5,65,86,19,34,54,54,55,65,246,6,8,42,1,53,5,33,1,7,104,254,100,123,151,93,36,143,204,135,
    30,66,105,90,206,91,106,63,28,133,211,147,24,55,37,254,88,205,205,2,177,254,119,3,177,253,77,65,97,5,34,17,11,92,65,97,5,62,96,104,43,3,253,86,2,170,3,48,105,90,254,76,1,206,78,106,
    31,253,91,5,182,253,160,1,223,129,179,254,91,131,199,38,171,0,0,6,191,4,78,132,199,34,58,64,28,137,199,42,13,13,16,16,32,27,27,28,30,6,114,133,196,66,167,8,32,18,24,146,11,9,131,196,
    168,194,8,46,6,73,254,186,99,117,71,31,126,189,119,24,54,82,70,181,71,85,51,26,121,189,127,18,41,25,254,196,189,189,2,29,254,198,3,7,253,239,1,8,4,78,102,254,149,13,76,65,133,5,8,74,
    66,77,35,1,254,12,1,245,2,34,77,67,254,191,1,83,47,80,24,254,22,4,78,254,61,1,93,102,155,254,220,0,1,0,58,254,71,4,87,6,217,0,90,0,61,64,29,36,32,32,47,17,16,64,64,55,25,47,61,61,10,
    43,47,7,0,0,82,84,128,87,78,78,71,10,3,120,81,6,39,26,204,50,50,47,51,47,51,130,198,34,18,57,57,130,6,32,57,131,197,97,27,5,86,19,5,33,7,22,91,200,9,34,14,2,7,77,179,7,32,54,78,99,
    9,41,6,35,34,38,38,53,52,54,54,55,73,246,5,127,192,13,130,22,8,136,46,2,39,53,51,22,22,23,62,2,3,114,36,53,16,12,42,20,49,108,45,177,189,194,153,181,200,70,143,219,151,110,114,39,72,
    98,82,125,113,63,83,97,26,21,106,84,59,116,132,85,133,159,71,88,200,170,199,186,246,210,192,185,151,190,91,160,134,121,194,87,96,77,180,113,34,83,79,31,144,49,119,52,40,89,105,6,217,
    10,5,119,5,6,88,68,26,192,137,148,176,26,6,25,177,146,96,157,112,62,3,3,30,51,34,47,56,6,7,22,17,179,18,30,5,5,71,123,77,85,123,71,5,5,130,135,135,117,168,24,64,8,7,8,43,53,77,14,41,
    89,83,30,25,33,109,54,48,96,64,0,0,1,0,28,254,104,3,169,5,83,0,91,0,59,64,28,36,32,32,47,16,17,66,66,55,25,47,63,63,65,55,5,45,83,85,128,88,79,72,10,6,114,0,43,50,50,50,65,53,41,78,
    252,5,65,53,29,34,62,2,53,119,68,18,32,55,65,53,10,8,165,27,35,53,15,12,41,19,46,98,43,119,143,112,95,67,109,67,100,217,174,101,103,36,74,92,72,121,106,49,70,79,19,24,92,47,46,121,
    140,76,118,138,58,75,175,149,97,140,75,70,141,106,139,119,145,167,125,120,81,158,81,67,63,122,69,28,69,67,28,139,49,109,57,38,89,106,5,83,9,5,120,5,5,75,59,26,138,108,99,119,26,9,17,
    63,105,81,92,146,86,3,2,27,47,32,49,46,6,5,22,19,162,20,22,6,5,69,114,68,77,120,71,3,3,36,77,63,60,77,36,156,72,88,75,74,36,35,153,25,36,9,36,78,73,26,25,35,101,57,49,93,62,0,255,255,
    0,111,0,0,6,24,5,182,6,6,1,116,0,131,15,38,158,254,20,5,185,6,18,130,15,63,148,0,0,0,3,0,124,255,236,5,199,5,205,0,17,0,26,0,34,0,25,64,12,31,23,23,5,27,14,3,114,18,78,144,7,114,71,
    6,78,143,15,55,18,1,50,54,54,55,33,30,2,19,34,6,6,7,33,38,38,5,199,86,169,253,169,172,24,170,167,10,51,253,91,149,198,105,8,252,103,9,104,199,152,146,196,108,13,3,149,19,220,78,131,
    15,49,186,254,175,252,217,121,227,156,156,226,122,4,129,114,212,148,223,251,131,149,44,109,255,236,4,110,4,98,0,14,0,23,0,31,131,149,40,27,20,20,4,24,11,7,114,15,78,122,7,137,149,78,
    121,9,32,22,138,146,35,7,33,46,2,78,111,10,54,152,230,128,254,0,97,129,69,7,253,161,7,70,131,95,142,146,15,2,95,9,71,129,78,107,13,51,134,254,253,179,78,152,109,109,152,78,3,42,160,
    154,102,140,72,0,1,0,130,0,63,5,94,5,195,0,29,0,23,64,12,0,23,3,114,13,6,7,2,114,6,8,114,0,43,43,18,57,43,50,48,49,65,131,255,43,1,35,1,51,1,30,2,23,62,3,55,19,81,222,8,8,62,4,251,
    46,61,49,29,254,173,230,253,247,212,1,74,22,33,26,11,7,19,23,25,16,171,45,83,116,97,43,71,24,23,49,5,25,55,114,91,251,235,5,182,252,79,66,108,97,50,38,78,82,90,48,2,25,143,179,84,17,
    8,165,8,12,133,125,37,4,99,4,89,0,27,131,125,42,19,13,14,6,114,13,10,114,7,0,7,130,125,32,50,131,127,68,7,11,33,6,7,131,132,24,155,144,8,8,61,62,2,4,3,23,51,22,16,38,17,30,45,34,17,
    254,241,241,254,98,210,250,27,39,6,6,8,32,23,144,36,67,98,4,89,8,8,157,7,6,35,68,51,252,225,4,78,253,61,79,126,37,45,136,68,1,178,105,121,51,0,255,255,133,247,52,7,138,6,38,2,113,0,
    0,1,7,4,12,4,226,1,105,0,12,180,36,49,130,254,34,0,43,206,109,185,9,34,99,6,33,130,37,32,114,134,37,32,124,130,57,35,12,180,34,47,130,170,133,37,8,34,0,3,0,124,254,19,9,216,5,205,0,
    17,0,33,0,63,0,38,64,22,63,40,41,3,34,58,51,15,45,6,114,34,6,114,30,24,173,16,11,38,43,43,63,51,18,23,57,130,216,66,112,16,80,246,8,126,110,6,35,37,51,19,30,77,207,6,32,51,24,66,83,
    14,8,63,55,5,95,77,156,234,159,161,236,154,74,134,1,23,216,209,1,19,138,251,241,86,183,145,147,181,83,83,180,145,146,184,87,4,104,215,239,18,30,23,8,7,10,46,26,221,213,254,39,39,111,
    153,102,50,73,27,23,62,35,63,91,66,22,50,66,165,18,33,229,178,81,32,6,51,180,1,1,138,139,254,255,190,253,130,48,92,87,42,51,145,74,2,125,251,24,155,108,12,53,255,255,0,109,254,19,8,
    213,4,98,4,38,0,82,0,0,0,7,0,92,4,176,130,7,8,48,2,0,124,255,138,6,17,6,42,0,29,0,56,0,39,64,19,33,36,30,36,36,3,0,27,9,114,49,46,43,43,15,12,18,3,114,0,43,50,205,51,16,205,51,43,205,
    51,51,17,130,8,62,48,49,69,34,38,39,46,2,2,53,52,18,36,55,54,54,51,50,22,23,22,4,18,21,20,14,2,7,6,6,3,133,16,32,62,130,29,32,2,92,208,5,98,235,5,8,116,22,3,74,55,71,12,144,217,146,
    73,131,1,3,191,13,71,53,52,71,14,185,1,0,133,74,145,214,139,14,72,189,18,68,50,47,69,18,122,161,81,182,181,16,71,48,51,68,18,122,164,83,83,164,118,51,56,20,124,194,1,1,151,203,1,57,
    197,26,59,45,45,59,27,197,254,198,203,151,253,194,125,21,56,51,1,32,44,36,37,43,27,149,235,152,230,1,35,40,44,39,39,43,25,150,233,152,153,235,149,0,0,2,0,109,255,150,4,213,130,227,
    34,26,0,51,131,215,51,37,34,31,31,20,17,23,7,114,46,49,43,49,49,10,7,4,11,114,0,135,208,134,223,37,48,49,65,20,6,6,130,194,32,35,132,222,130,221,134,220,36,30,2,7,52,38,93,157,6,130,
    210,89,101,5,131,245,8,94,54,54,4,213,103,197,138,9,64,52,55,62,9,132,198,109,233,210,8,62,52,49,64,10,134,197,109,206,50,102,80,14,59,52,54,59,12,121,113,114,122,13,58,52,50,59,14,
    121,113,2,41,158,236,144,23,50,48,47,53,22,143,234,160,237,1,31,34,50,39,39,52,21,145,233,157,105,161,105,20,41,40,41,42,31,202,160,162,206,31,41,35,37,38,31,206,69,11,5,8,47,7,206,
    8,83,0,22,0,41,0,107,0,67,64,35,76,102,9,114,85,92,3,114,12,13,13,19,6,128,22,22,35,24,23,58,51,3,114,105,73,70,70,67,71,71,67,67,42,9,76,51,5,47,47,17,51,17,51,51,43,50,222,50,204,
    50,47,26,204,50,130,13,38,43,50,43,50,48,49,65,101,52,5,55,30,2,51,51,21,35,34,46,2,35,34,6,7,19,53,54,54,53,52,46,2,53,52,54,89,35,5,35,6,1,34,36,130,13,32,18,65,203,5,78,42,6,39,
    21,20,18,22,51,50,54,55,130,89,32,22,131,8,32,18,131,50,131,62,32,39,132,38,33,22,18,87,35,6,8,184,38,39,6,6,2,186,37,66,89,53,59,112,115,130,77,15,19,92,142,113,94,44,55,54,3,79,59,
    61,31,41,31,58,49,61,70,61,115,254,223,195,254,254,127,73,139,197,125,79,157,61,76,44,106,60,79,127,87,46,92,181,134,63,107,49,205,49,112,65,135,180,91,46,88,125,80,61,106,44,75,60,
    158,79,125,198,138,72,71,144,217,146,110,176,73,73,175,7,76,33,64,87,55,24,36,48,36,138,34,45,34,59,59,254,174,74,18,54,27,20,19,16,27,27,41,42,73,63,56,93,65,249,227,197,1,93,226,
    167,1,14,191,103,53,43,150,32,45,76,146,210,133,181,254,248,143,46,39,1,177,254,79,41,44,143,1,8,181,133,210,146,76,45,32,150,43,53,103,191,254,242,167,170,254,229,206,113,65,130,0,
    8,51,0,0,3,0,119,255,236,6,215,7,21,0,18,0,41,0,103,0,61,64,32,76,98,11,114,83,90,7,114,25,26,26,32,19,128,35,35,0,8,7,58,51,7,114,101,66,70,70,66,66,42,11,65,112,8,65,109,19,79,245,
    5,32,7,65,96,9,33,39,50,65,122,12,46,35,53,52,62,2,3,34,46,2,53,52,18,54,51,50,79,151,6,34,6,21,20,118,42,5,34,17,51,17,132,8,88,114,13,35,18,21,20,6,96,48,6,8,199,3,131,60,71,61,115,
    80,59,60,31,40,31,57,45,60,112,115,129,76,17,20,93,142,112,94,44,54,54,3,137,36,66,90,151,113,179,124,65,110,201,137,74,117,50,71,46,83,41,118,123,39,75,107,69,66,101,51,202,35,67,
    76,45,91,127,66,124,118,40,82,46,72,49,117,75,138,201,110,115,216,150,115,165,55,57,163,6,42,74,61,58,93,64,16,74,18,56,28,19,19,15,26,27,41,43,235,37,46,36,139,34,45,35,58,62,35,63,
    88,54,24,248,215,73,144,212,137,190,1,0,131,37,29,149,22,27,207,202,94,149,104,54,49,57,1,57,254,203,40,48,22,96,179,126,202,207,27,23,150,29,37,131,255,0,190,183,253,130,84,71,74,
    81,255,255,0,50,255,248,7,52,7,29,6,38,2,93,0,0,1,7,3,137,1,113,1,105,0,10,179,55,5,2,89,189,8,38,39,0,0,6,60,5,181,130,35,32,94,133,35,35,0,217,0,2,130,35,33,52,16,89,225,6,55,0,1,
    0,123,254,20,4,242,5,203,0,32,0,19,64,9,16,22,9,114,21,7,0,3,130,65,34,50,47,43,111,210,5,80,52,5,33,14,2,88,34,6,8,70,55,17,35,17,34,46,3,53,52,18,54,36,3,78,114,216,90,77,75,175,
    98,118,186,131,68,105,223,176,53,106,46,205,152,235,170,110,52,96,186,1,12,5,203,47,42,173,35,49,80,150,213,133,176,255,139,13,14,253,90,1,216,71,133,185,230,132,166,1,19,201,110,130,
    121,50,109,254,20,3,185,4,98,0,29,0,21,64,11,15,21,11,114,20,15,70,252,9,139,123,33,6,6,138,123,131,122,8,54,54,54,2,132,84,165,60,58,59,137,60,109,144,71,76,146,104,68,90,42,201,126,
    198,136,71,137,242,4,98,35,29,162,22,34,89,180,136,137,176,86,23,16,253,89,1,217,2,69,138,211,144,206,254,117,0,130,115,60,108,255,253,4,113,5,6,0,19,0,12,179,5,12,10,0,0,47,47,57,
    57,48,49,65,23,3,5,7,37,132,3,36,39,19,37,55,5,131,3,8,101,3,150,116,185,1,32,67,254,227,207,1,29,65,254,226,184,118,184,254,226,65,1,32,206,254,225,66,1,31,5,6,66,254,192,166,112,
    165,254,154,167,113,166,254,194,66,1,65,165,113,167,1,103,166,114,167,0,0,8,0,43,254,195,7,192,5,144,0,13,0,27,0,41,0,55,0,69,0,83,0,97,0,111,0,24,64,12,73,17,87,45,3,59,6,31,108,101,
    38,31,0,47,51,47,71,13,6,45,54,54,51,50,22,23,35,38,38,35,34,6,7,1,168,13,32,19,154,41,155,27,48,5,94,5,101,101,97,108,7,77,7,76,52,61,67,6,251,54,140,14,48,1,245,5,100,101,98,108,
    6,76,7,77,52,61,66,7,252,200,135,29,33,77,51,130,44,32,105,140,43,33,4,56,140,14,35,118,5,100,102,132,87,131,42,34,7,252,182,131,72,33,107,7,134,72,47,3,180,89,103,107,85,56,34,30,
    60,252,37,89,103,106,86,131,9,53,254,234,88,103,107,84,56,34,31,59,3,10,89,102,106,85,57,33,30,60,1,231,145,39,35,1,244,89,102,131,39,38,30,60,3,4,89,102,105,132,59,8,41,0,8,0,43,254,
    127,7,127,5,211,0,9,0,19,0,28,0,37,0,47,0,56,0,66,0,76,0,24,64,12,47,10,25,65,53,4,6,34,70,76,37,34,65,105,10,58,23,6,6,7,39,55,62,2,1,23,7,14,2,7,39,54,54,1,22,22,23,21,7,38,38,39,
    1,131,26,40,35,54,54,55,37,23,30,2,23,132,17,130,26,33,7,39,131,26,130,16,32,21,130,7,33,53,1,130,61,51,35,39,62,2,55,6,118,67,75,163,60,93,2,45,109,115,251,217,94,3,130,6,8,77,54,
    66,74,164,254,77,85,192,73,14,77,181,78,4,9,11,19,67,35,94,22,40,10,2,7,17,27,54,50,20,64,46,108,44,252,47,46,107,45,93,17,40,81,31,4,231,51,115,117,53,85,191,74,254,33,21,39,11,132,
    11,13,39,46,23,4,162,66,45,107,45,94,16,27,54,52,252,1,93,130,6,38,51,21,65,45,108,2,50,132,32,40,20,66,35,253,165,14,76,181,79,130,95,45,218,2,46,109,114,53,67,74,164,59,4,160,74,
    164,130,131,47,69,173,80,253,220,13,39,45,24,93,20,40,10,133,3,76,130,87,32,14,131,95,8,33,0,2,0,196,254,119,6,85,7,108,0,23,0,39,0,39,64,19,35,29,128,32,24,7,17,9,19,11,2,114,21,0,
    2,2,83,149,5,61,16,206,51,43,50,18,57,57,222,50,26,204,50,48,49,65,19,35,17,52,54,54,55,35,1,35,17,51,17,20,130,241,40,51,1,51,17,51,3,1,34,38,99,55,8,8,78,51,14,2,4,208,165,193,5,
    8,4,8,252,241,234,189,4,6,3,7,3,12,232,230,157,253,96,141,168,76,7,182,10,97,108,93,111,10,187,8,87,173,254,119,1,137,3,34,60,147,142,52,251,77,5,182,252,212,63,145,131,47,4,174,250,
    253,253,196,7,182,71,142,106,111,78,83,106,103,142,74,130,169,42,171,254,129,5,105,6,24,0,21,0,37,131,169,53,33,26,128,30,22,7,16,8,17,10,6,114,19,2,2,0,8,10,114,0,43,206,130,134,135,
    169,32,205,138,169,136,168,148,167,8,32,24,131,197,3,5,2,253,192,245,193,4,7,3,2,65,245,213,137,253,202,142,167,78,7,183,10,98,106,94,112,10,187,9,130,165,8,35,129,1,127,2,104,43,104,
    98,32,252,131,4,78,253,161,44,103,99,40,3,125,252,84,253,223,6,88,72,142,105,108,81,83,106,102,143,131,165,36,45,0,0,4,137,87,45,6,52,32,64,15,21,5,18,1,1,15,4,4,14,19,2,114,22,14,
    8,0,63,51,85,127,7,90,152,5,44,33,21,33,21,51,50,4,22,21,20,4,33,33,86,172,5,123,207,7,8,45,38,1,145,1,63,254,193,178,212,1,0,114,254,242,254,224,254,105,151,151,1,127,178,192,178,
    179,87,166,5,182,182,174,251,107,190,123,206,229,4,82,174,182,252,241,254,5,87,46,5,34,2,0,23,130,127,55,102,6,20,0,20,0,29,0,32,64,14,19,16,16,1,4,4,15,20,22,5,5,23,15,131,126,34,
    51,47,51,118,226,10,130,125,33,33,17,24,64,33,10,87,45,13,8,43,124,1,59,254,197,1,37,154,201,98,94,201,160,254,19,155,155,1,226,254,232,1,26,119,144,134,6,20,241,145,253,253,71,140,
    104,104,152,84,4,146,145,241,251,221,87,47,7,34,2,0,196,130,129,8,39,129,5,182,0,16,0,31,0,55,64,27,3,24,24,17,22,23,23,17,6,21,21,17,17,10,5,4,4,10,10,12,31,13,2,114,12,8,114,0,43,
    83,175,5,32,17,84,232,6,135,7,57,48,49,65,20,6,7,23,7,39,6,6,35,35,17,35,17,33,32,4,1,51,50,54,55,39,55,99,112,5,8,105,35,35,4,129,100,111,109,112,136,50,112,66,174,205,1,157,1,25,
    1,7,253,16,160,36,65,30,97,118,124,50,56,174,180,188,4,4,119,204,63,148,89,183,14,13,253,207,5,182,226,254,10,4,4,132,87,164,33,109,81,136,134,0,2,0,171,254,22,4,131,4,98,0,29,0,50,
    0,57,64,30,13,30,30,18,7,114,11,6,114,10,14,114,44,43,43,45,42,42,4,39,39,24,27,27,25,26,26,87,51,5,34,17,51,50,132,2,130,166,130,2,24,136,167,7,32,69,24,171,240,15,35,51,50,18,17,
    135,183,36,3,34,6,6,7,70,148,7,135,183,8,116,2,196,85,128,91,31,13,5,8,202,165,29,9,32,91,129,87,201,241,89,80,109,114,123,33,76,82,106,127,59,2,56,130,110,22,40,19,122,120,121,41,
    39,138,20,45,75,43,41,110,37,254,67,6,56,158,49,81,48,254,227,254,229,168,238,71,148,88,164,11,14,3,208,83,164,123,33,132,181,95,7,7,158,89,157,50,153,104,200,200,0,1,0,46,0,0,4,28,
    5,182,0,13,0,29,64,14,11,3,3,10,6,6,8,2,13,2,114,8,65,102,9,82,250,6,8,36,33,17,33,21,33,17,35,17,35,53,51,17,4,28,253,117,1,163,254,93,205,150,150,5,182,177,254,38,175,253,132,2,124,
    175,2,139,0,130,81,38,13,0,0,3,103,4,78,142,81,34,6,114,8,24,68,67,15,130,77,137,81,55,3,103,254,8,1,87,254,169,201,153,153,4,78,168,254,200,156,254,46,1,210,156,1,224,131,81,36,196,
    254,0,5,5,130,163,49,38,0,31,64,15,3,0,0,10,13,13,30,23,9,6,2,114,5,133,164,33,47,51,83,171,5,37,48,49,65,34,6,7,130,162,131,86,44,54,54,51,50,30,2,21,20,14,2,35,34,38,100,49,5,81,
    143,5,8,38,2,76,47,109,31,205,3,96,253,109,43,126,61,150,242,171,91,81,147,198,116,96,132,62,62,126,71,89,136,92,47,115,218,2,114,11,5,253,158,131,215,54,16,8,12,86,168,246,160,165,
    248,166,82,24,25,182,25,24,64,123,178,114,157,211,109,131,147,36,171,254,10,4,34,130,229,53,33,0,35,64,18,30,27,27,3,6,6,14,2,33,6,114,32,10,114,21,14,15,79,103,5,33,50,17,136,151,
    130,235,121,82,12,133,144,74,48,7,130,178,8,52,3,116,254,1,35,78,39,166,238,129,121,205,128,73,118,54,46,119,65,126,141,169,180,31,71,32,202,4,78,168,254,202,6,8,124,254,197,197,249,
    119,28,28,177,25,33,191,201,200,198,8,7,254,64,130,127,8,34,1,0,3,254,119,7,63,5,182,0,21,0,40,64,22,11,17,8,20,14,1,6,7,21,19,15,2,114,9,4,7,2,7,7,13,89,32,6,34,16,204,51,24,74,101,
    9,32,51,130,114,33,35,1,130,4,33,1,35,24,74,105,7,8,38,6,212,253,216,1,185,218,197,117,253,212,195,253,211,230,2,64,253,216,221,2,30,195,2,29,5,182,253,61,253,194,253,194,1,137,2,229,
    253,27,132,3,36,242,2,196,253,60,133,3,32,0,131,129,34,129,6,108,130,139,132,129,37,1,14,17,11,20,8,132,129,36,6,114,4,9,9,131,129,89,34,6,34,51,47,204,156,129,40,5,252,254,67,1,91,
    210,189,108,24,74,237,19,39,254,104,253,225,1,127,2,47,24,74,241,17,8,33,255,255,0,86,254,52,4,98,5,203,6,38,1,176,0,0,0,7,3,107,1,109,0,0,255,255,0,70,254,52,3,169,4,98,130,23,32,
    208,134,23,32,18,130,7,57,1,0,196,254,119,5,88,5,182,0,14,0,33,64,17,8,13,1,3,10,14,11,2,114,4,7,130,172,32,10,65,46,8,32,43,116,190,5,33,73,2,65,44,8,63,51,17,1,4,228,253,150,1,239,
    239,197,133,253,131,205,205,2,110,5,182,253,60,253,195,253,194,1,137,2,230,253,26,131,13,43,2,196,0,1,0,171,254,128,4,125,4,78,130,95,48,31,64,16,3,8,11,3,5,10,6,6,114,12,2,2,0,5,70,
    110,10,42,23,57,48,49,65,17,35,1,17,35,17,130,89,55,51,1,1,51,17,3,193,100,254,24,202,202,1,205,221,254,47,1,108,195,254,128,1,128,131,242,52,4,78,253,233,2,23,253,237,254,103,253,
    222,0,0,2,0,196,0,0,5,0,130,191,32,3,130,97,50,35,64,18,6,11,14,3,9,2,1,2,1,8,13,9,2,114,5,8,132,194,130,190,36,57,57,47,47,17,132,101,130,97,32,1,137,104,37,2,143,121,121,2,113,24,
    84,113,9,37,1,10,3,217,251,29,137,193,33,253,60,130,99,36,171,0,0,4,69,130,195,134,99,46,9,14,6,3,12,3,2,3,2,11,5,12,6,114,8,94,112,6,33,50,18,137,99,130,198,32,37,130,196,134,102,
    36,2,99,128,1,95,131,197,33,247,232,131,209,40,3,246,252,104,3,152,88,253,237,24,75,53,10,34,2,0,35,130,99,32,246,134,199,34,29,64,15,131,199,34,8,0,1,140,196,32,206,65,133,5,35,83,
    53,33,21,138,193,52,35,2,39,2,172,243,253,132,205,205,2,110,229,253,149,4,96,176,176,251,160,140,192,35,0,1,0,14,130,93,8,33,69,6,20,0,18,0,41,64,22,18,0,114,11,5,8,3,7,10,10,13,10,
    114,1,17,17,14,14,4,7,6,114,0,43,206,97,102,5,38,50,17,18,23,57,43,48,94,7,6,32,1,135,199,41,35,53,51,53,1,117,1,96,254,160,65,146,5,132,204,41,157,157,6,20,180,143,253,102,2,23,136,
    206,35,209,143,180,0,130,113,36,10,0,0,5,147,130,207,48,12,0,27,64,14,4,11,1,3,6,12,8,9,2,114,3,6,65,145,6,133,203,57,73,2,35,1,17,35,17,33,53,33,17,1,5,119,253,150,2,134,242,253,131,
    205,254,179,2,26,66,80,6,32,14,132,201,37,4,178,253,60,2,196,130,85,32,29,130,85,35,5,4,78,0,132,85,34,11,1,4,132,85,32,6,130,85,32,10,130,191,32,50,67,82,9,137,85,8,32,4,223,254,47,
    1,247,226,254,23,194,254,165,2,29,1,206,4,78,253,235,253,199,2,47,253,209,3,174,160,253,233,2,23,130,85,36,196,254,119,5,250,130,171,48,15,0,31,64,15,9,4,4,6,12,7,2,114,13,2,2,0,132,
    172,73,17,5,38,17,57,47,51,48,49,65,131,172,130,3,36,51,17,33,17,51,130,1,60,5,53,210,253,46,205,205,2,210,204,203,254,119,1,137,2,163,253,93,5,182,253,158,2,98,250,255,253,194,130,
    89,36,171,254,129,5,48,130,175,138,89,32,6,133,89,66,252,9,149,89,60,4,111,197,253,203,202,202,2,53,200,190,254,129,1,127,1,231,254,25,4,78,254,59,1,197,252,84,253,223,130,89,36,196,
    0,0,6,123,70,173,7,42,3,12,12,0,8,5,5,1,2,114,10,24,90,1,8,32,51,133,87,32,115,130,165,70,175,7,44,33,17,196,205,2,209,2,25,254,180,205,253,47,133,167,34,178,250,252,131,180,32,0,130,
    81,36,171,0,0,5,207,130,171,36,13,0,29,64,14,135,81,32,6,130,81,65,91,6,130,72,24,84,43,8,137,81,32,171,130,162,38,2,37,254,163,200,253,203,133,159,34,160,252,82,131,172,131,81,36,
    196,254,0,8,67,130,163,58,41,0,35,64,17,34,31,31,1,4,4,40,38,41,2,114,36,40,8,114,21,14,0,47,51,43,50,72,190,5,71,87,5,32,17,70,168,22,34,46,2,35,70,204,6,8,36,17,35,17,4,230,49,124,
    56,143,232,167,90,80,146,197,117,98,129,63,62,126,71,86,135,93,48,67,123,170,103,45,109,35,203,253,118,205,130,216,63,99,8,6,86,168,247,161,163,246,165,83,25,24,182,23,26,64,124,176,
    112,127,180,116,55,9,8,253,161,5,3,250,253,130,147,38,1,0,171,254,10,6,182,130,239,39,35,0,37,64,19,28,25,25,130,157,42,34,32,35,6,114,30,34,10,114,19,12,70,184,5,145,159,33,22,22,
    80,204,7,36,53,22,22,51,50,70,183,10,32,33,131,157,8,35,53,31,67,32,145,231,135,115,195,122,71,114,51,44,111,62,117,133,159,164,27,64,26,201,254,9,202,4,78,254,36,5,7,120,253,202,70,
    187,12,38,8,254,65,3,171,252,85,130,135,8,39,2,0,124,255,172,5,230,5,205,0,61,0,78,0,44,64,21,66,58,58,21,37,30,3,114,49,46,46,21,9,114,75,4,4,18,7,7,15,0,47,73,67,6,48,50,17,51,43,
    50,18,57,125,47,51,48,49,65,20,6,6,7,132,142,33,55,21,71,229,5,42,6,6,35,34,38,38,2,53,52,18,36,81,165,8,32,14,80,95,8,32,46,120,138,8,37,7,52,38,38,35,34,133,25,8,174,23,62,2,5,187,
    71,114,62,30,77,44,39,70,30,19,52,57,28,92,159,71,51,130,67,159,246,168,87,139,1,29,218,67,124,37,54,28,98,50,112,163,104,50,115,208,140,24,44,18,52,77,42,55,101,138,83,105,171,101,
    198,40,78,58,44,67,46,24,41,70,46,60,90,50,2,168,134,218,158,48,14,17,12,10,172,8,12,5,52,48,18,18,105,194,1,15,165,240,1,89,185,23,14,168,10,19,79,153,217,137,189,252,126,5,6,61,164,
    193,101,126,184,121,59,98,217,186,104,151,82,49,89,123,75,96,167,135,50,42,136,172,0,0,2,0,109,255,195,4,216,4,98,0,55,0,69,0,43,64,21,56,24,24,0,63,17,17,14,14,44,47,11,114,31,34,
    34,41,87,196,6,34,47,51,17,105,142,10,32,51,87,207,5,32,7,131,213,80,210,8,91,186,5,99,72,5,65,39,8,114,194,13,36,62,2,1,34,6,110,144,9,8,141,38,2,82,55,91,33,44,24,72,40,153,127,65,
    136,105,30,45,9,54,69,79,143,96,88,139,80,112,77,20,56,31,29,58,30,25,71,39,73,138,58,44,105,76,160,225,117,59,121,182,1,170,42,60,30,68,50,62,77,27,56,4,98,17,11,159,7,14,218,200,
    119,177,97,9,3,64,169,118,120,159,79,75,159,125,140,196,53,10,14,7,8,153,9,8,43,37,16,23,147,255,160,129,213,154,84,254,102,49,94,67,97,145,45,40,147,104,64,93,49,0,255,255,0,124,254,
    52,4,205,5,203,6,38,0,38,0,0,0,7,3,107,2,43,0,131,23,35,109,254,52,3,24,152,199,10,35,3,107,1,138,130,31,8,32,1,0,22,254,119,4,93,5,182,0,11,0,23,64,11,8,4,4,5,2,114,9,0,2,8,114,0,
    43,206,51,43,50,17,68,179,7,8,37,53,33,21,33,17,51,17,2,167,212,254,67,4,71,254,67,204,254,119,1,137,5,3,179,179,251,178,253,194,0,0,1,0,43,254,129,3,193,97,217,5,43,64,11,2,10,10,
    11,6,114,5,3,8,10,130,71,33,50,204,134,71,132,65,58,35,17,35,17,33,53,3,193,254,152,187,190,197,254,154,4,78,162,252,244,253,225,1,127,3,172,162,88,243,5,32,4,24,114,93,9,38,1,0,0,
    254,20,4,37,130,85,48,15,0,24,64,13,15,2,8,9,4,1,14,3,6,114,1,15,130,87,72,84,6,34,65,35,17,24,181,4,12,8,35,2,119,201,254,82,214,224,27,51,10,10,13,47,29,222,214,254,82,254,20,1,234,
    4,80,253,172,75,170,50,50,168,77,2,84,251,176,130,175,130,252,131,105,54,0,16,0,37,64,18,11,8,5,5,12,3,3,15,2,2,6,16,8,114,10,6,2,130,94,32,50,78,38,7,34,51,17,51,101,82,8,34,1,51,
    1,130,2,33,21,33,130,202,8,34,1,231,254,203,1,53,254,25,224,1,110,1,112,220,254,26,1,52,254,204,1,80,177,46,3,135,253,68,2,188,252,130,55,177,254,132,103,134,193,46,21,0,33,64,16,1,
    9,9,4,17,16,8,8,6,21,24,110,229,9,37,17,57,47,57,57,51,68,193,5,32,1,75,197,5,34,33,53,33,138,210,8,46,4,37,254,83,1,28,254,227,201,254,225,1,29,254,84,214,221,31,47,13,9,15,49,31,
    217,4,78,251,178,153,254,173,1,83,153,4,78,253,176,83,155,61,61,159,84,2,75,130,219,60,5,254,119,5,26,5,182,0,15,0,32,64,17,3,6,12,9,4,2,11,7,2,114,13,0,2,2,5,8,131,218,80,77,5,32,
    23,73,52,6,74,184,5,132,218,8,43,17,4,85,119,254,131,254,126,218,1,230,254,59,227,1,97,1,96,219,254,58,1,114,201,254,119,1,137,2,109,253,147,2,248,2,190,253,199,2,57,253,64,253,191,
    65,247,5,38,32,254,128,4,113,4,78,134,107,39,12,6,9,4,5,11,7,6,133,107,32,10,157,107,41,3,179,108,254,223,254,221,227,1,136,24,181,187,10,43,28,180,254,128,1,128,1,175,254,81,2,52,
    24,181,190,7,36,254,110,253,222,0,130,215,36,23,254,119,6,211,132,215,39,29,64,14,8,4,4,12,5,130,212,32,9,66,174,8,67,199,6,35,48,49,65,17,24,132,99,8,33,17,51,130,1,45,6,16,251,161,
    254,102,4,51,254,52,2,185,204,208,66,185,8,36,176,5,3,250,255,66,189,8,33,5,192,132,197,39,33,64,16,12,5,13,9,9,130,94,43,5,2,10,114,5,6,114,0,0,47,43,43,130,69,33,51,50,131,4,145,
    93,61,5,1,252,88,254,210,3,77,254,169,2,23,201,191,254,129,1,127,3,172,162,162,252,247,3,171,252,82,253,225,131,183,36,153,254,119,5,165,130,183,33,23,0,130,93,46,22,19,19,7,10,10,
    6,23,14,2,114,3,1,6,8,67,32,7,76,223,8,130,100,34,35,17,35,24,92,178,16,49,4,217,204,196,212,120,211,123,207,223,204,122,138,114,189,117,5,182,131,187,33,1,137,24,92,186,16,39,0,1,
    0,146,254,128,5,5,130,205,37,23,0,27,64,13,19,132,108,32,6,131,108,67,141,8,130,108,24,87,163,7,146,105,54,73,188,192,197,87,179,122,171,191,201,103,97,100,165,84,4,78,252,84,253,222,
    1,128,24,82,123,7,39,254,114,96,91,58,49,1,222,24,93,131,11,60,27,0,51,64,25,26,23,23,20,21,21,20,20,9,3,6,6,9,8,8,9,9,1,27,15,2,114,1,8,67,171,5,130,115,32,47,130,219,32,51,131,3,
    132,8,131,129,131,125,32,7,130,237,35,34,46,2,53,107,133,5,130,5,53,54,54,55,17,4,217,206,71,139,76,121,112,176,123,64,204,127,144,121,76,143,67,130,240,60,74,2,81,27,41,11,254,199,
    1,46,44,90,141,98,2,74,253,222,117,119,1,89,254,175,8,39,26,2,189,131,247,35,0,0,4,63,130,247,57,28,0,47,64,23,24,21,22,22,21,21,12,6,9,9,12,8,8,12,12,1,28,16,6,114,1,79,56,9,139,139,
    136,137,34,21,35,53,24,94,42,10,32,23,135,139,8,48,63,202,54,109,64,116,10,18,11,170,187,201,98,97,116,59,112,56,4,78,251,178,1,223,35,54,14,247,231,2,1,178,157,1,154,254,112,95,90,
    2,1,34,254,235,11,49,36,1,222,130,139,36,197,0,0,5,5,82,133,5,24,94,157,8,37,10,19,8,114,0,2,65,14,8,42,51,17,51,48,49,83,51,17,54,54,51,24,100,96,8,8,41,34,6,7,17,35,197,204,119,220,
    115,206,224,205,121,139,114,189,116,204,5,182,253,172,44,49,192,180,253,181,2,34,118,119,43,41,253,69,0,255,255,0,171,24,77,151,8,8,36,75,0,0,0,2,0,42,255,236,6,108,5,205,0,41,0,50,
    0,37,64,18,30,30,36,36,47,47,23,6,6,0,10,18,9,114,42,0,123,74,10,65,129,5,130,243,37,50,4,18,21,21,33,89,111,5,33,55,21,131,248,8,116,36,2,39,35,34,38,53,52,54,55,51,6,6,21,20,22,51,
    51,62,3,23,34,14,2,7,33,52,38,3,255,222,1,17,126,251,214,9,113,214,159,96,175,150,60,87,233,175,218,254,213,164,15,50,125,146,20,14,166,7,15,54,58,34,18,103,166,227,142,98,157,114,
    67,7,3,81,188,5,205,183,254,181,220,91,160,225,119,31,46,24,186,37,54,166,1,49,209,130,109,46,76,28,15,64,34,48,54,157,242,167,85,178,62,121,176,114,225,248,131,189,42,34,255,236,4,
    253,4,96,0,39,0,47,131,189,37,29,29,35,35,43,43,130,146,38,0,9,16,11,114,40,0,24,89,108,10,137,189,33,22,22,130,189,72,180,6,131,188,36,46,2,39,34,38,141,188,35,2,23,34,6,131,187,8,
    43,38,3,46,146,207,110,253,14,4,173,160,114,163,88,83,169,116,119,194,141,80,6,91,127,65,17,13,154,9,13,53,55,20,19,135,208,125,122,150,12,2,37,57,117,24,139,42,7,60,39,39,168,38,35,
    67,133,198,131,48,98,75,39,70,25,20,57,30,47,52,157,205,100,156,154,152,92,137,77,131,179,63,42,254,118,6,108,5,205,0,44,0,53,0,41,64,20,33,33,40,40,50,50,25,6,6,0,21,19,10,18,9,114,
    45,65,115,7,34,205,51,18,67,220,5,34,17,51,47,65,117,18,37,7,17,35,17,38,38,65,120,42,39,80,202,140,198,178,245,135,13,65,120,10,35,16,102,167,229,65,120,7,39,182,5,205,183,254,184,
    219,95,65,120,6,43,34,48,4,254,133,1,128,21,175,1,30,188,65,126,21,8,42,2,0,34,254,129,4,253,4,96,0,41,0,49,0,40,64,19,12,12,19,19,45,45,4,29,29,23,41,32,0,39,11,42,23,7,114,0,43,50,
    63,51,51,205,139,200,32,69,65,111,19,33,51,50,65,149,13,130,220,32,19,65,130,6,36,2,214,122,179,102,65,115,11,35,20,134,208,128,65,149,10,36,71,145,92,191,85,65,131,6,36,7,21,129,216,
    150,65,119,10,8,53,154,205,103,122,223,152,114,185,181,39,39,168,34,32,3,254,145,5,67,154,152,92,137,77,255,255,0,196,0,0,1,145,5,182,6,6,0,44,0,0,255,255,0,3,0,0,6,235,7,129,6,38,
    1,175,130,25,44,7,2,51,1,50,1,105,0,10,179,22,1,2,91,115,8,131,35,34,35,6,24,130,35,32,207,133,35,40,0,201,0,0,0,10,179,22,11,91,115,9,48,196,254,0,5,58,5,182,0,43,0,41,64,20,42,39,
    39,4,76,127,5,37,14,43,37,2,114,36,76,126,7,76,125,8,70,25,6,34,1,54,50,83,42,20,76,129,8,8,56,51,17,54,54,55,1,4,237,253,146,12,22,13,150,240,171,91,84,150,201,117,96,131,63,63,128,
    78,84,134,96,52,79,140,184,106,64,113,44,205,205,44,97,48,1,176,5,182,253,85,1,81,158,237,158,165,248,163,83,53,5,43,62,120,178,116,127,178,111,52,17,12,253,183,130,29,37,51,55,119,
    54,1,233,130,194,37,0,171,254,11,4,87,83,67,7,48,22,19,19,27,30,30,24,29,25,6,114,24,10,114,11,4,15,130,248,32,50,109,29,11,35,101,20,6,6,83,202,8,37,54,54,53,52,38,38,77,33,6,8,66,
    51,17,1,51,1,30,2,4,87,125,209,125,76,109,48,46,105,65,81,126,72,87,165,119,43,92,37,200,200,1,211,230,254,47,148,229,131,66,195,252,120,28,25,174,23,33,85,173,133,136,175,84,14,10,
    254,80,4,78,254,3,1,253,254,24,1,113,240,72,127,6,50,214,5,182,0,33,0,25,64,13,7,32,2,114,23,16,9,114,2,0,72,123,5,49,204,43,50,43,50,48,49,101,51,3,35,19,35,17,33,14,3,7,84,87,10,
    47,54,54,55,54,54,18,18,55,33,4,241,229,157,231,165,210,24,101,115,27,39,181,253,194,1,137,5,5,107,24,101,120,21,32,0,130,135,62,10,254,129,4,225,4,78,0,26,0,27,64,14,7,25,6,114,19,
    12,11,114,2,128,0,5,10,114,0,43,50,26,142,137,24,83,215,16,46,33,4,13,212,137,200,131,208,254,191,20,60,91,130,91,24,92,87,8,60,46,36,14,2,184,162,253,223,1,127,3,172,255,0,254,150,
    229,107,9,8,156,4,5,68,142,224,1,57,205,78,183,6,33,5,47,130,255,32,24,130,119,49,13,22,17,17,19,24,20,2,114,19,8,114,12,5,0,47,51,43,66,52,5,37,48,49,65,17,20,2,65,134,10,33,53,17,
    105,190,6,45,17,5,47,134,243,161,98,132,62,61,129,78,176,182,80,30,5,48,5,182,250,202,212,254,227,143,24,24,181,23,25,232,232,2,30,80,39,7,130,229,36,171,254,12,4,114,130,229,36,24,
    0,29,64,15,133,109,130,233,32,10,130,109,65,248,7,79,201,5,130,111,65,246,11,32,55,136,111,8,35,4,114,110,191,121,74,111,49,46,109,61,114,123,1,253,205,202,202,2,51,4,78,251,212,188,
    236,110,28,28,176,23,35,170,196,1,191,80,61,7,80,237,5,33,6,21,130,221,51,15,0,33,64,16,12,7,7,9,15,10,2,114,0,5,5,2,128,9,8,24,91,39,9,133,115,65,229,7,80,242,7,37,5,47,230,159,232,
    167,80,243,7,34,181,253,194,80,243,11,132,203,34,129,5,72,73,135,7,45,3,14,14,0,6,1,6,114,7,12,12,9,128,0,24,90,139,8,65,63,7,80,153,5,33,51,17,66,72,6,42,17,171,202,2,53,200,214,138,
    200,131,207,80,74,7,35,252,84,253,223,80,250,5,32,0,130,91,36,153,254,119,4,217,130,183,58,23,0,34,64,16,21,18,18,6,9,9,1,22,13,2,114,4,2,128,1,8,0,63,26,205,51,43,108,27,7,35,48,49,
    97,35,132,178,72,13,10,98,10,5,36,4,217,195,196,187,73,134,10,40,204,254,119,2,62,1,159,43,50,24,102,62,12,130,109,36,146,254,128,4,73,130,201,132,109,47,22,19,19,7,10,10,2,23,14,6,
    114,5,3,128,2,10,143,109,32,65,130,108,34,35,17,51,73,244,17,35,73,184,191,174,73,138,12,45,251,178,254,128,2,34,1,61,53,67,176,158,1,153,73,138,8,38,1,0,196,254,119,7,105,130,221,
    8,32,29,0,37,64,19,12,27,1,3,9,15,11,2,114,18,21,128,16,21,21,0,9,8,114,0,43,50,50,17,51,26,16,204,77,54,6,36,97,1,35,30,3,24,211,184,9,65,68,5,8,51,52,62,2,55,35,1,3,65,254,56,8,3,
    6,4,2,188,1,39,1,178,8,1,186,1,37,229,157,233,167,204,2,4,5,3,9,254,45,4,233,39,104,108,97,34,252,149,5,182,251,89,4,167,74,134,5,40,3,119,35,101,107,95,30,251,25,68,141,5,34,129,6,
    68,130,251,52,24,0,35,64,18,21,12,8,3,18,23,20,6,114,24,4,4,11,1,128,18,65,199,6,34,51,17,51,134,137,35,101,3,35,19,24,95,103,18,42,33,17,6,68,138,200,132,190,5,4,6,24,95,108,14,39,
    1,14,162,253,223,1,127,2,24,95,111,18,34,252,84,0,70,19,18,42,0,0,0,5,43,7,129,6,38,0,36,69,239,6,38,78,1,105,0,10,179,23,97,135,10,38,92,255,236,3,231,6,24,120,155,6,36,6,2,51,254,
    0,130,33,33,45,0,120,213,10,132,69,32,72,135,69,35,0,106,0,60,130,69,35,12,180,40,28,133,70,34,206,48,49,130,123,132,71,33,5,223,135,71,24,169,127,19,130,37,41,0,6,170,5,182,6,6,0,
    136,0,131,175,130,51,35,6,157,4,98,130,15,32,168,132,15,36,196,0,0,4,6,132,175,32,40,134,175,32,40,132,175,24,159,183,16,132,175,32,72,130,35,130,175,33,5,0,130,175,32,36,135,175,58,
    0,2,0,131,255,236,5,139,5,205,0,28,0,37,0,25,64,12,24,34,34,9,29,19,9,114,0,123,12,6,33,50,18,76,5,5,41,34,6,6,7,53,62,2,51,50,4,100,209,8,8,87,36,2,53,53,33,46,2,3,50,54,54,55,33,
    20,22,22,2,198,100,178,150,60,59,140,177,114,173,1,8,179,91,85,169,247,162,223,254,237,127,4,49,9,114,212,113,131,193,114,9,252,168,80,180,5,28,31,47,24,184,25,43,27,106,199,254,234,
    172,172,254,236,197,105,184,1,84,233,69,158,225,119,251,128,111,210,151,149,212,111,131,235,36,102,255,236,4,33,131,235,33,3,115,132,235,132,165,37,7,72,6,38,2,206,130,199,36,7,0,106,
    0,143,130,235,35,12,180,59,47,132,167,104,229,6,132,53,35,5,223,6,38,131,53,39,1,6,0,106,227,0,0,12,24,169,1,13,71,249,6,130,73,71,249,5,35,0,106,1,33,132,73,33,39,27,71,250,5,65,159,
    5,36,3,0,0,6,35,131,73,33,1,207,134,111,33,184,0,131,75,33,39,27,71,252,5,133,37,32,86,130,165,32,98,131,149,33,1,176,134,37,32,3,132,75,37,68,56,43,3,114,0,105,123,7,36,70,255,236,
    3,169,132,75,32,208,130,37,130,149,32,153,131,73,33,69,57,65,132,5,130,73,35,0,1,0,69,130,73,32,63,67,151,6,49,18,1,28,28,29,26,2,2,25,24,24,9,29,2,114,16,9,9,80,103,11,34,17,51,17,
    113,50,5,37,4,4,21,20,6,4,71,160,15,8,59,35,53,1,33,53,4,4,254,26,1,3,1,30,128,254,250,199,121,215,93,96,226,103,132,170,81,98,194,147,132,1,198,253,119,5,182,150,254,37,9,208,201,
    128,199,112,38,42,183,46,51,67,125,87,86,113,56,159,1,189,178,130,135,49,36,254,20,3,212,4,78,0,30,0,35,64,17,1,29,29,30,27,130,135,38,25,10,30,6,114,17,10,70,39,5,34,18,57,47,130,
    129,136,133,124,147,9,84,168,5,72,38,5,132,133,8,83,3,162,254,56,156,227,123,131,247,174,121,197,74,76,204,112,112,156,83,97,186,138,115,1,189,253,148,4,78,141,254,23,11,116,203,142,
    144,222,126,39,34,181,36,52,81,148,97,106,141,70,141,1,224,164,0,255,255,0,198,0,0,5,113,6,221,6,38,1,177,0,0,1,7,1,76,1,168,1,105,0,10,179,24,0,73,157,9,38,171,0,0,4,148,5,116,130,
    35,32,209,134,35,39,35,0,0,0,10,179,21,18,73,157,6,135,71,33,7,72,135,71,35,0,106,0,215,130,71,35,12,180,43,31,133,72,65,197,5,133,73,32,223,134,73,41,6,0,106,83,0,0,12,180,40,28,133,
    72,133,35,36,124,255,236,5,199,131,73,33,0,50,65,233,6,32,201,132,73,34,54,42,14,65,233,10,37,109,255,236,4,110,5,130,73,33,0,82,65,233,5,33,21,0,130,73,35,42,54,14,7,66,13,9,132,73,
    38,5,205,6,6,2,111,0,24,123,79,12,33,2,112,132,15,132,31,35,7,49,6,38,131,31,33,1,7,130,179,34,202,1,82,130,69,33,56,44,148,105,131,53,32,1,130,179,32,20,131,105,34,53,41,11,138,105,
    32,65,130,141,32,159,131,73,33,1,198,130,141,130,73,33,255,230,132,73,34,57,45,7,138,179,36,67,255,236,3,146,131,179,33,1,230,134,37,33,106,0,133,75,32,24,138,75,32,22,130,255,38,8,
    6,221,6,38,1,188,65,109,6,32,21,130,255,35,10,179,32,17,65,145,9,42,2,254,19,4,37,5,116,6,38,0,92,133,35,33,0,139,130,73,130,35,24,146,107,10,132,71,33,7,72,135,71,130,221,32,69,130,
    71,35,12,180,51,39,133,72,24,168,171,10,131,147,132,73,130,221,32,186,131,145,33,51,39,133,72,133,35,133,73,32,138,136,145,34,82,1,70,132,73,33,37,50,144,73,33,6,33,136,147,33,82,0,
    130,191,36,0,12,180,37,50,139,75,36,153,0,0,4,217,132,149,32,192,65,3,5,33,0,94,132,75,34,41,29,9,132,222,133,113,32,146,130,37,32,73,131,149,33,1,224,130,37,130,149,32,19,131,149,
    35,41,29,19,6,65,221,6,72,245,5,42,4,29,5,182,0,9,0,21,64,10,2,130,61,34,5,128,3,92,29,5,38,26,204,43,50,48,49,65,84,221,8,47,4,29,253,116,202,195,212,5,182,178,251,177,253,194,1,137,
    130,52,72,217,5,35,3,94,4,78,135,63,33,6,114,131,63,74,135,8,139,63,38,3,94,254,23,188,193,197,85,25,8,130,52,70,211,5,33,6,29,132,201,32,196,133,201,33,1,29,132,201,33,47,35,68,221,
    11,36,171,0,0,5,164,132,201,32,228,133,37,34,0,204,0,131,203,130,37,135,203,130,75,42,46,254,90,4,28,5,182,6,38,2,136,130,29,36,7,3,108,0,169,130,7,38,1,0,13,254,95,3,103,130,110,57,
    31,0,34,64,16,17,14,14,22,25,25,13,21,18,6,114,7,0,26,13,10,0,63,51,204,50,73,33,10,24,156,242,9,34,54,53,53,96,203,5,32,33,96,135,6,52,51,17,20,6,6,1,42,39,67,24,18,53,29,32,44,23,
    201,153,153,2,193,96,148,5,8,54,176,54,108,254,95,16,9,157,7,12,23,50,39,142,1,210,156,1,224,168,254,200,156,254,199,254,224,93,125,64,0,1,0,5,254,90,5,9,5,182,0,29,0,36,64,19,20,23,
    17,14,4,15,21,18,2,130,130,36,13,24,13,13,15,94,81,8,130,137,93,142,5,24,79,96,8,131,133,32,1,84,202,11,130,131,43,4,6,41,72,23,21,52,32,32,46,25,112,84,214,17,46,117,181,55,114,254,
    90,16,8,164,7,12,25,55,45,128,84,223,14,37,188,254,201,88,130,71,130,145,38,32,254,95,4,101,4,78,130,145,38,35,64,18,14,17,23,20,131,145,32,6,135,145,39,10,0,63,51,17,51,16,204,24,
    104,51,8,156,144,43,3,112,40,67,23,18,52,30,32,44,23,110,84,251,17,33,31,165,65,29,13,85,4,13,32,101,65,34,5,130,145,44,4,0,0,4,200,5,182,0,17,0,33,64,16,130,12,45,13,9,17,17,2,12,
    14,8,114,5,2,2,114,0,91,20,8,130,151,38,48,49,83,33,1,51,1,130,2,37,33,21,33,1,35,1,130,2,8,49,33,125,1,49,254,120,227,1,97,1,95,220,254,119,1,53,254,195,1,180,234,254,131,254,126,
    219,1,176,254,201,3,85,2,97,253,198,2,58,253,159,176,253,91,2,109,253,147,2,165,0,130,113,32,32,130,113,34,44,4,78,130,113,41,37,64,18,7,1,1,10,4,2,13,132,115,32,10,130,115,32,6,136,
    115,33,57,18,86,201,6,145,117,51,114,1,0,254,192,229,1,15,1,15,227,254,189,1,5,254,250,1,88,229,65,1,5,48,84,254,254,2,130,1,204,254,106,1,150,254,52,154,254,24,1,130,246,38,1,232,
    0,0,2,0,118,130,117,32,59,130,231,48,11,0,21,0,23,64,11,14,8,8,0,9,2,114,12,0,8,71,54,8,41,48,49,97,32,36,53,52,54,54,51,130,236,36,17,37,51,17,35,89,133,5,8,35,2,156,254,218,255,0,
    112,247,200,201,205,254,122,185,180,124,165,82,176,217,200,123,199,119,2,92,250,74,173,1,254,55,114,88,134,119,69,163,7,39,68,6,20,6,6,0,71,0,131,113,36,117,255,236,6,128,130,113,41,
    30,0,43,0,35,64,18,31,29,8,130,1,42,19,30,2,114,40,19,11,114,4,13,11,132,120,33,50,43,95,170,5,46,51,48,49,65,17,22,22,51,50,54,53,17,51,17,20,24,76,210,12,33,62,2,130,145,135,141,
    132,34,8,73,4,14,1,110,104,99,110,202,212,200,120,168,43,48,168,130,150,204,104,65,139,217,151,145,135,121,167,84,59,117,85,127,119,5,182,251,184,110,102,119,117,1,202,254,30,179,207,
    87,77,73,90,99,191,139,99,163,119,64,2,95,252,241,53,120,98,85,113,56,120,87,0,2,0,107,130,163,59,149,6,20,0,38,0,51,0,43,64,23,38,0,114,33,39,39,29,8,8,23,29,7,114,19,45,45,23,130,
    168,32,15,24,68,105,8,130,170,32,47,130,156,33,51,43,131,171,32,20,136,171,38,14,2,35,34,38,38,39,131,6,49,2,17,16,18,51,50,22,22,23,51,38,38,53,17,1,34,6,21,132,38,8,96,55,53,52,38,
    4,39,91,122,108,100,201,54,104,152,97,98,126,79,28,38,105,141,91,214,251,239,194,83,123,88,30,12,4,9,254,232,136,134,132,140,156,127,2,123,6,20,251,125,121,132,126,131,1,39,254,188,
    101,149,97,47,44,84,57,52,85,50,1,30,1,24,1,28,1,36,46,76,48,34,113,38,1,163,253,167,212,200,199,200,186,185,33,197,210,0,1,0,63,130,197,54,132,5,203,0,53,0,35,64,17,7,8,38,38,37,21,
    37,21,0,17,26,9,114,46,84,172,9,38,57,47,47,51,18,57,57,130,187,130,160,42,21,20,6,7,21,30,2,23,20,30,2,65,114,10,38,46,2,39,52,46,2,35,119,80,13,8,89,62,2,2,23,149,209,112,183,140,
    114,157,82,1,24,50,83,60,112,103,200,220,195,98,157,111,59,1,52,104,154,103,193,189,133,171,83,146,124,116,173,72,103,56,144,173,5,203,90,161,107,152,176,27,7,17,90,144,100,65,97,63,
    31,119,130,1,187,254,40,201,195,50,102,159,109,70,104,68,33,158,66,118,77,106,115,71,48,137,42,68,40,130,185,44,74,255,236,5,233,4,98,0,51,0,37,64,18,132,185,40,20,37,20,37,0,16,27,
    11,114,76,46,5,36,50,43,50,17,57,130,186,32,17,145,187,137,186,35,14,2,35,34,130,187,135,186,34,53,52,38,112,90,6,8,84,1,203,118,183,105,107,91,69,101,58,4,3,44,91,73,104,101,198,51,
    102,150,98,97,151,104,56,4,3,67,127,97,146,126,135,155,118,114,79,149,79,63,84,181,4,98,66,129,95,99,118,26,9,13,62,98,70,58,84,46,126,130,1,39,254,187,100,149,98,48,41,80,120,79,69,
    87,41,155,73,88,75,74,37,34,153,38,131,183,56,63,254,119,4,252,5,203,0,40,0,32,64,15,8,7,26,26,23,23,0,15,12,17,8,33,65,111,5,34,63,51,206,24,138,22,20,35,21,51,17,35,130,1,65,100,
    20,60,40,152,215,115,188,145,177,189,209,196,214,56,113,168,111,199,204,141,182,87,154,132,122,184,75,103,57,148,179,65,87,5,54,175,27,7,25,177,146,235,253,194,1,137,1,160,64,98,65,
    32,158,66,118,77,104,117,65,77,8,41,79,254,129,4,95,4,96,0,38,0,130,149,35,7,8,25,25,130,149,38,16,13,18,10,32,0,7,84,117,5,146,149,34,30,2,21,135,149,33,38,38,65,250,5,65,63,10,8,
    89,221,121,187,107,110,92,66,107,64,192,191,195,68,138,106,155,133,144,167,125,117,83,162,76,68,88,198,4,96,66,129,96,98,116,26,10,18,64,107,82,148,253,225,1,127,1,54,62,83,41,156,
    72,87,74,77,39,34,153,38,40,0,0,1,0,3,255,234,7,60,5,182,0,46,0,29,64,15,8,8,13,19,46,2,114,37,30,9,114,4,13,75,174,5,71,4,5,67,251,17,37,38,53,17,33,14,5,82,205,19,62,4,203,1,103,
    108,106,105,202,223,190,122,188,106,254,103,9,19,21,21,23,22,11,27,80,134,108,36,70,30,26,54,24,125,97,8,8,44,20,5,182,251,200,119,107,121,128,1,187,254,40,201,195,79,174,143,3,140,
    72,154,160,157,149,133,55,136,190,100,13,10,171,11,13,87,151,93,66,201,1,3,1,41,160,130,169,40,10,255,236,6,89,4,78,0,40,131,169,44,7,7,14,21,39,6,114,33,26,11,114,3,14,67,255,6,36,
    43,50,18,57,47,130,169,67,248,13,33,46,2,130,169,82,237,17,48,3,248,99,110,101,99,200,52,102,149,96,97,152,106,56,254,211,82,245,17,52,163,1,125,118,114,126,129,1,40,254,188,101,149,
    98,48,47,97,150,102,2,52,83,2,17,52,1,0,196,255,236,7,111,5,182,0,26,0,37,64,20,26,2,114,24,19,8,130,1,37,21,22,2,114,21,8,65,72,8,69,70,7,68,155,14,37,6,35,34,38,38,39,82,157,8,56,
    5,6,102,107,106,101,201,219,189,122,186,104,1,253,87,205,205,2,169,5,182,251,203,120,109,67,166,7,36,79,175,143,1,42,83,15,11,39,255,236,6,186,4,78,0,29,131,123,34,1,26,12,130,1,50,
    28,29,6,114,28,10,114,8,19,11,114,3,6,114,0,43,43,50,43,69,195,11,35,33,17,51,17,65,34,16,57,53,33,17,35,17,1,117,2,29,201,101,108,100,100,198,51,102,148,97,95,152,107,56,253,227,202,
    98,167,5,37,253,48,119,114,126,130,68,242,5,40,98,48,48,97,149,102,111,254,25,130,118,57,0,1,0,124,255,236,5,175,5,203,0,35,0,25,64,12,35,0,0,7,22,15,3,114,30,7,66,62,6,79,119,6,36,
    33,21,20,2,6,130,235,32,36,97,100,13,34,6,2,21,116,119,5,8,69,54,53,33,3,85,2,90,70,151,239,169,222,254,198,166,172,1,78,239,131,234,93,75,77,205,116,169,231,118,109,219,166,153,179,
    78,254,125,3,3,98,160,255,0,181,96,180,1,80,236,226,1,81,188,51,43,172,35,55,144,254,253,174,173,254,254,144,116,199,124,0,130,141,40,109,255,236,4,190,4,98,0,34,131,141,32,34,132,
    141,35,7,114,29,7,66,34,6,137,141,37,14,2,35,34,36,38,97,241,12,90,4,5,33,50,54,130,139,8,59,2,168,2,22,60,127,202,142,185,254,255,132,141,1,17,197,120,199,81,66,63,175,98,137,179,
    87,174,192,117,145,67,254,182,2,88,76,125,201,142,76,135,254,178,178,1,2,139,45,39,157,29,48,100,185,127,182,222,75,134,87,131,133,56,22,255,236,5,12,5,182,0,22,0,29,64,14,2,21,21,
    22,10,10,15,22,2,114,6,15,79,3,8,78,121,7,32,33,65,139,10,8,35,6,35,34,38,38,53,17,33,53,4,84,254,67,106,108,107,106,202,224,191,122,190,107,254,76,5,182,179,252,124,120,109,121,129,
    1,188,67,42,5,35,142,3,140,179,131,103,32,43,130,237,36,187,4,78,0,25,132,103,42,24,24,25,10,10,17,25,6,114,6,17,133,239,130,238,147,103,69,141,5,131,104,61,3,184,254,157,102,109,102,
    102,199,53,104,149,97,95,153,107,57,254,159,4,78,160,253,209,120,114,120,132,1,43,66,247,7,40,149,103,2,54,160,0,1,0,105,130,109,52,119,5,203,0,47,0,31,64,15,40,39,15,15,18,18,0,24,
    32,9,114,8,69,83,5,35,43,50,17,57,70,193,10,115,166,5,98,86,5,33,51,21,24,84,231,8,32,54,91,124,5,130,232,32,52,24,138,30,8,8,91,2,143,108,173,143,64,102,84,183,122,139,158,88,186,
    147,187,189,215,245,193,181,83,160,145,60,89,228,142,187,255,130,202,181,101,152,85,121,225,5,203,35,63,43,149,53,64,112,107,78,111,60,166,128,133,133,129,22,43,28,187,39,40,104,190,
    128,150,188,23,6,16,88,144,99,107,165,95,255,255,0,86,255,236,3,178,4,98,6,6,1,129,0,0,75,101,6,34,173,5,182,130,183,54,28,64,14,15,40,2,114,31,24,9,114,7,0,128,42,13,8,0,63,51,26,
    204,50,87,164,5,75,227,14,87,172,23,40,17,51,17,20,6,6,4,169,40,75,108,7,32,205,87,183,27,34,188,55,115,75,117,10,87,192,24,33,250,252,75,128,5,130,173,41,10,254,95,4,190,4,78,0,40,
    0,131,173,37,33,6,114,27,20,11,131,173,34,35,13,10,155,173,68,226,17,133,168,43,3,203,40,67,24,19,51,30,32,44,23,204,24,116,51,20,33,176,53,76,164,11,34,3,172,255,24,116,65,15,33,252,
    75,75,139,5,37,255,255,0,0,254,141,24,76,235,11,43,4,14,5,4,0,0,255,255,0,92,254,141,24,76,235,9,37,0,7,4,14,4,174,132,23,38,0,0,0,5,43,7,232,84,139,8,40,88,5,4,1,82,0,10,179,24,84,
    139,16,32,150,24,77,39,8,34,88,4,170,130,54,34,10,179,46,84,141,16,32,209,135,71,35,3,99,4,243,130,71,45,13,183,3,2,27,5,1,1,111,86,0,43,52,52,132,135,37,255,236,4,110,6,127,135,75,
    130,39,32,158,130,75,131,39,36,49,0,1,1,127,136,39,130,20,130,151,137,79,34,100,4,238,134,79,32,31,140,79,36,23,255,236,3,231,138,79,34,100,4,159,134,79,32,53,145,79,33,8,73,136,159,
    34,101,4,237,134,79,32,32,140,79,32,92,130,159,34,61,6,247,136,159,34,101,4,164,134,79,32,54,146,79,32,101,136,79,34,102,4,242,134,79,32,25,143,79,35,3,231,7,19,136,79,34,102,4,157,
    134,79,32,47,141,79,33,254,141,130,239,32,138,133,79,33,0,39,65,183,5,43,1,7,1,74,0,222,1,105,0,10,179,43,65,143,11,33,254,141,130,243,32,32,133,83,33,0,39,130,35,40,142,0,0,1,7,4,
    14,4,160,130,91,34,10,179,54,65,151,15,33,8,20,136,167,34,103,4,251,134,167,32,22,145,167,33,6,194,136,167,34,103,4,168,134,167,32,44,141,167,35,0,0,5,43,138,79,34,104,4,249,164,79,
    34,104,4,166,154,79,32,88,136,159,32,105,155,79,33,7,6,136,159,34,105,4,171,154,79,32,97,136,79,34,106,4,245,153,159,33,7,15,136,79,32,106,151,159,36,254,141,5,43,7,134,79,41,0,39,
    1,77,0,255,1,105,1,7,65,159,5,87,179,15,37,254,141,3,231,5,248,65,151,8,34,77,0,175,65,151,6,32,143,130,171,33,10,179,87,189,11,32,196,130,43,34,249,5,182,130,43,32,40,130,27,130,79,
    45,4,207,0,0,255,255,0,109,254,141,4,39,4,98,130,23,32,72,134,23,32,204,132,23,38,196,0,0,3,249,7,232,133,47,37,1,7,2,88,4,202,130,215,35,10,179,17,2,83,23,9,38,109,255,236,4,39,6,
    150,133,59,133,35,132,119,32,37,66,15,10,133,71,32,83,135,71,41,1,81,0,154,1,105,0,10,179,12,143,71,33,5,233,134,71,36,6,1,81,120,0,130,33,75,1,5,82,161,5,130,69,35,4,141,7,209,135,
    69,35,3,99,4,189,130,141,35,12,180,40,20,133,70,133,37,131,143,34,131,6,127,134,71,32,7,130,37,32,179,130,143,35,12,180,60,40,133,144,133,37,32,56,132,217,137,75,34,100,4,192,130,75,
    40,13,183,2,1,24,2,1,1,117,68,17,8,32,47,132,221,137,77,34,100,4,183,66,1,10,32,125,136,39,131,155,34,85,8,73,136,155,34,101,4,188,134,79,32,25,140,79,131,157,34,70,6,247,136,157,34,
    101,4,173,134,79,35,45,0,1,1,140,79,35,3,249,8,101,136,79,34,102,4,186,134,79,32,18,144,79,34,39,7,19,136,79,34,102,4,175,134,79,32,38,141,79,37,254,141,3,249,7,138,133,79,51,0,39,
    4,14,4,207,0,0,1,7,1,74,0,184,1,105,0,10,179,37,65,137,11,37,254,141,4,39,6,32,133,83,33,0,39,130,35,32,149,66,81,6,32,204,66,81,16,38,143,0,0,2,20,7,232,130,43,32,44,131,35,35,2,88,
    3,149,130,167,34,10,179,9,96,105,10,32,119,130,25,37,251,6,150,6,38,3,130,155,33,1,7,130,35,32,125,132,71,33,9,2,86,203,9,38,181,254,141,1,159,5,182,133,71,43,0,7,4,14,3,147,0,0,255,
    255,0,157,130,23,34,135,5,239,130,23,32,76,130,51,131,23,32,124,132,23,38,124,254,141,5,199,5,205,130,23,32,50,133,23,33,5,140,66,201,8,34,110,4,98,130,23,32,82,133,23,33,4,210,132,
    23,86,119,5,131,167,130,47,131,131,33,5,139,132,167,32,38,24,175,239,15,131,167,32,0,130,59,131,35,33,4,213,132,167,32,38,86,224,5,36,48,49,255,255,0,133,71,32,209,133,119,37,1,7,3,
    99,5,125,130,71,35,12,180,61,41,86,191,15,34,151,6,127,133,133,131,37,33,4,199,130,73,132,37,87,43,15,32,7,137,75,32,100,132,75,40,13,183,3,2,45,14,1,1,94,66,91,8,32,64,70,189,8,132,
    151,35,3,100,4,200,65,227,6,131,39,66,51,9,132,155,33,8,73,136,155,34,101,5,123,130,155,131,79,32,46,140,79,32,109,132,79,32,247,136,157,32,101,132,157,135,39,143,79,32,101,136,79,
    32,102,136,79,32,39,145,79,33,7,19,136,79,32,102,136,79,131,39,138,79,33,254,141,24,202,5,9,51,0,39,4,14,5,140,0,0,1,7,1,74,1,107,1,105,0,10,179,58,65,139,11,37,254,141,4,110,6,32,
    133,83,131,43,33,4,210,133,43,33,0,182,130,251,131,43,32,7,98,201,8,130,247,39,6,127,7,138,6,38,2,84,131,35,35,0,118,2,75,132,79,32,51,65,219,13,35,5,69,6,33,130,35,32,85,133,35,33,
    1,151,132,71,33,49,11,153,71,34,67,1,167,132,71,32,50,154,71,34,67,0,242,132,71,32,48,144,71,32,232,135,143,41,2,88,5,145,1,82,0,10,179,49,144,71,32,150,135,143,35,2,88,4,216,132,71,
    32,47,144,71,32,83,135,71,35,1,81,1,77,132,143,32,61,143,71,33,5,233,135,71,35,1,81,0,153,132,71,32,59,139,71,37,254,141,6,127,6,20,130,35,32,84,67,19,5,45,5,136,0,0,255,255,0,109,
    254,141,5,69,4,246,133,59,37,0,7,4,14,4,213,132,23,32,182,130,23,38,43,5,182,6,38,0,56,134,47,32,86,132,23,38,161,254,141,4,83,4,78,130,23,32,88,133,23,33,4,205,132,23,34,182,255,236,
    73,205,6,130,47,33,1,7,130,239,32,83,132,239,33,25,9,70,37,9,37,161,255,236,4,83,6,130,239,32,0,130,59,131,35,33,4,220,132,167,33,29,13,68,3,9,130,71,35,6,164,7,138,130,143,130,111,
    37,1,7,0,118,2,28,132,239,33,39,21,140,71,35,5,200,6,33,130,35,32,87,65,199,6,32,169,132,71,33,42,19,153,71,34,67,1,119,132,71,32,38,154,71,34,67,1,4,132,71,32,41,144,71,32,232,135,
    143,130,215,32,92,132,215,32,37,144,71,130,215,32,2,132,143,35,2,88,4,226,132,71,32,40,144,71,32,83,135,71,35,1,81,1,29,132,143,32,49,143,71,33,5,233,135,215,34,1,81,0,74,237,5,32,
    52,139,71,37,254,141,6,164,6,20,133,71,49,0,7,4,14,5,93,0,0,255,255,0,161,254,141,5,200,4,247,133,59,131,23,33,4,204,132,23,42,0,254,141,4,154,5,182,6,38,0,60,130,147,130,47,33,4,180,
    132,23,38,2,254,19,4,37,4,78,130,23,32,92,133,23,35,5,148,255,234,110,119,7,32,7,130,239,131,47,33,1,7,130,203,32,180,132,239,33,14,7,65,127,9,132,59,32,6,130,239,32,0,130,59,132,35,
    32,116,132,239,32,35,91,5,10,130,16,34,4,154,7,130,239,133,71,130,203,32,120,132,239,32,26,143,71,131,239,132,71,40,6,1,81,61,0,0,10,179,47,138,69,36,109,254,201,4,223,131,237,41,0,
    211,0,0,1,7,0,66,0,240,130,88,131,35,32,11,67,249,5,8,32,0,2,252,95,4,217,255,208,6,127,0,18,0,28,0,35,64,17,24,19,22,128,28,64,4,9,18,3,13,128,1,15,7,130,52,50,47,93,51,26,205,23,
    57,26,220,26,204,57,57,48,49,65,35,38,38,24,169,97,8,8,75,51,30,2,23,39,54,54,55,51,21,6,6,7,35,254,228,111,51,109,51,53,107,51,112,35,78,75,31,208,31,75,77,35,81,41,59,32,185,43,110,
    54,110,4,217,35,85,49,49,85,35,25,38,88,94,46,46,94,88,38,194,48,91,64,21,58,105,43,0,0,2,251,120,4,217,254,233,136,131,55,16,22,24,128,27,19,64,15,1,10,3,5,0,128,12,12,0,0,47,50,47,
    26,16,204,130,131,36,222,50,26,205,50,130,131,35,53,62,2,55,131,123,32,21,134,140,32,39,130,147,8,43,53,51,22,22,23,21,252,99,34,77,77,31,209,32,75,76,35,112,52,106,53,52,109,51,136,
    57,109,44,184,32,60,40,4,217,23,39,88,94,45,45,94,88,39,23,133,139,40,195,43,105,58,21,64,91,50,22,130,131,8,53,252,95,4,217,255,153,6,247,0,18,0,40,0,45,64,20,37,34,34,28,25,38,19,
    192,27,64,9,18,6,128,4,14,14,12,12,6,0,47,51,47,51,17,51,26,16,205,57,26,220,26,204,50,57,57,112,215,5,34,30,2,23,135,136,32,35,131,153,37,37,50,22,21,20,6,24,87,155,15,37,254,10,31,
    75,77,35,65,33,11,56,1,156,91,104,73,55,6,91,10,63,60,51,45,22,39,12,13,44,5,252,46,94,88,38,25,65,42,10,59,251,69,74,58,62,12,76,126,7,34,33,32,29,5,3,91,4,3,0,2,252,94,4,217,254,
    243,7,19,130,171,52,43,0,37,64,16,43,43,28,35,23,40,31,31,23,9,18,12,128,6,6,12,131,167,47,26,16,221,57,198,50,47,50,16,205,50,50,47,48,49,65,143,163,32,1,80,169,5,8,94,35,34,6,7,35,
    54,54,51,50,30,2,51,50,54,55,254,7,32,77,80,36,104,55,110,53,52,113,53,105,35,81,78,31,1,175,6,48,83,58,39,73,67,63,29,41,44,13,97,10,100,85,41,75,67,62,29,41,43,12,5,244,45,91,85,
    39,23,34,81,49,49,81,34,23,38,85,92,45,1,31,64,101,57,29,38,29,47,51,96,127,29,39,29,49,49,0,130,171,32,100,130,171,53,216,6,194,0,13,0,24,0,29,64,11,15,24,21,20,20,13,13,7,128,10,
    3,130,166,33,26,204,130,162,37,124,47,51,24,204,50,130,163,102,85,5,34,51,22,22,131,138,39,55,21,14,2,7,35,53,54,130,148,8,42,216,11,158,148,151,152,8,114,8,103,88,80,110,10,32,29,
    67,72,36,102,36,61,29,5,244,129,154,150,133,86,60,65,81,206,21,36,76,67,28,23,48,96,61,143,113,39,27,64,10,23,14,17,19,19,138,112,32,50,131,111,32,205,144,111,42,37,22,22,23,21,35,
    46,2,39,53,254,132,111,33,151,9,131,111,44,81,109,10,254,253,29,59,37,101,35,73,67,28,135,112,43,64,82,206,61,96,48,23,28,67,76,36,21,135,225,33,7,6,130,225,44,35,0,31,64,12,23,20,
    29,14,22,22,10,10,130,114,130,226,44,26,204,50,51,17,51,124,47,24,204,50,57,57,143,227,32,3,66,40,21,141,127,48,239,89,98,71,50,6,86,9,58,55,49,40,25,40,13,14,46,137,134,50,1,18,68,
    70,58,60,12,44,98,10,30,33,31,26,5,2,83,4,4,130,143,38,92,4,217,254,242,7,15,130,143,32,38,130,143,40,13,38,23,30,128,35,26,18,64,65,2,8,40,220,50,26,222,50,50,26,205,50,65,3,16,32,
    19,66,20,21,50,216,10,155,150,150,153,10,115,8,104,87,82,108,10,140,7,47,83,59,66,16,5,8,74,43,14,97,10,100,86,41,74,68,62,28,42,43,11,5,231,124,146,145,125,80,53,53,80,1,40,64,99,
    57,29,38,30,47,52,96,124,29,38,29,49,49,0,1,0,39,254,52,1,123,0,2,0,19,0,12,179,17,10,10,4,0,47,51,47,51,48,49,87,52,38,39,55,22,22,21,90,245,5,98,33,5,63,222,76,69,133,64,105,112,
    102,35,64,27,17,51,27,38,50,237,56,117,64,2,46,127,86,94,109,11,7,125,5,8,48,130,77,38,26,254,90,1,165,0,178,130,77,38,14,180,7,0,192,13,14,66,76,5,32,48,24,103,111,10,53,54,53,53,
    35,53,51,17,20,6,6,161,40,72,23,20,54,31,32,46,26,23,209,82,51,12,32,178,82,25,5,8,33,255,255,0,24,254,20,4,93,5,182,6,38,0,55,0,0,0,7,0,122,1,100,0,0,255,255,0,36,254,20,2,206,5,72,
    130,23,32,87,133,23,33,0,234,132,23,34,124,254,52,75,59,11,35,1,80,2,44,132,23,35,109,254,52,4,75,59,10,35,1,80,1,120,137,47,33,6,221,74,7,5,41,0,39,1,76,1,154,1,105,1,7,130,55,32,
    65,130,87,34,10,179,35,72,143,11,131,67,33,5,116,73,183,7,38,1,76,0,229,0,0,1,130,43,131,75,32,0,131,43,73,111,6,35,0,2,0,102,101,5,5,24,79,91,7,40,13,25,25,0,29,8,11,114,17,110,233,
    11,90,245,7,32,2,85,4,5,43,53,33,38,38,35,34,6,7,53,54,54,1,111,180,5,8,59,2,8,161,242,134,123,222,150,143,207,110,2,236,4,175,160,104,164,89,83,169,1,184,253,227,1,55,114,88,123,148,
    4,98,128,251,184,182,254,254,139,123,224,152,114,176,189,38,40,169,38,35,253,88,90,138,78,159,0,255,255,24,92,165,7,35,6,6,2,88,132,233,44,1,255,236,7,35,5,205,4,39,0,50,1,93,131,183,
    8,66,3,118,254,169,255,153,0,16,181,3,2,50,14,2,0,184,255,252,176,86,0,43,52,52,0,2,1,88,4,204,3,146,6,49,0,10,0,28,0,29,64,12,28,11,11,0,25,17,17,6,4,128,1,0,0,47,50,26,204,50,50,
    47,51,17,51,47,51,130,193,130,173,8,100,55,51,21,14,2,7,7,38,38,53,52,54,51,50,22,21,20,14,2,35,20,22,23,2,108,22,53,15,204,20,65,75,37,143,117,113,59,52,45,59,15,28,38,23,56,63,4,
    222,25,58,166,71,21,42,104,107,46,18,10,122,100,55,70,47,48,24,35,23,12,35,53,5,0,0,2,0,41,3,67,2,174,6,211,0,11,0,23,0,14,181,12,0,120,18,6,119,0,63,51,228,50,130,106,24,96,21,23,
    8,34,106,160,161,154,167,161,163,154,170,82,76,76,82,79,75,74,3,67,236,221,219,236,234,221,217,240,142,155,160,159,154,154,161,158,155,130,89,56,44,3,68,2,176,6,210,0,30,0,44,0,25,
    64,11,31,12,11,15,15,39,22,120,7,0,130,95,38,50,228,50,57,47,51,51,131,207,44,50,22,23,21,38,38,35,34,6,6,7,51,54,133,206,32,6,65,159,5,35,52,62,2,19,130,22,8,82,21,20,22,22,51,50,
    54,53,52,38,1,235,30,71,24,23,66,35,111,127,56,6,8,29,108,83,121,147,76,141,95,96,150,86,40,99,174,19,54,81,44,39,78,57,73,91,80,6,210,8,6,133,9,11,80,138,87,42,61,144,131,93,138,75,
    87,175,132,101,186,144,85,254,66,41,64,36,48,91,58,91,88,74,85,130,157,32,37,130,157,52,170,6,213,0,29,0,43,0,27,64,12,20,21,36,36,24,24,16,9,120,30,137,158,32,17,134,159,65,96,5,96,
    133,9,33,55,35,132,160,35,53,52,54,23,93,219,7,131,157,8,111,38,1,93,95,151,87,40,97,173,133,33,73,23,22,63,43,111,125,56,4,9,28,103,81,128,148,170,145,70,92,78,80,54,80,45,38,77,6,
    213,86,173,134,101,187,147,85,8,6,134,9,12,84,140,84,40,64,146,133,133,170,127,88,86,74,89,40,64,35,55,89,54,255,255,255,225,255,236,2,180,7,156,6,38,1,133,0,0,1,7,3,136,255,123,0,
    0,0,18,64,10,4,3,2,1,33,16,1,1,141,86,0,43,52,130,0,130,43,32,226,142,43,34,135,255,124,137,43,32,27,141,43,32,209,132,43,32,153,136,87,34,134,255,122,137,43,32,37,141,43,32,210,142,
    43,32,133,139,87,32,31,140,43,37,0,158,255,237,4,130,132,175,32,145,133,175,33,0,220,137,87,33,40,5,139,175,144,43,34,135,0,221,137,43,32,34,147,43,131,175,133,87,32,134,139,87,32,
    44,157,43,32,133,139,87,32,38,130,43,32,100,65,51,6,8,36,0,1,0,195,254,113,5,27,5,204,0,38,0,28,64,14,25,24,15,15,29,3,114,22,2,114,20,8,7,0,0,47,50,63,43,43,50,65,250,5,65,242,9,24,
    132,71,11,8,102,51,23,51,62,2,51,50,22,22,21,17,20,14,2,3,137,54,81,29,32,78,45,54,94,58,154,172,141,165,70,205,160,34,9,39,128,161,86,150,222,123,58,107,148,254,113,14,12,172,8,13,
    41,104,93,3,172,186,166,109,202,139,252,166,5,182,198,66,99,55,106,219,169,252,58,110,160,103,50,0,255,255,0,196,254,116,5,98,5,182,6,6,1,11,0,0,0,1,0,183,255,236,5,15,130,159,52,40,
    0,35,64,18,6,23,23,0,28,27,18,18,32,3,114,25,2,114,11,0,89,221,5,132,162,41,17,57,47,206,48,49,69,34,38,38,24,100,204,10,42,17,52,38,35,34,6,6,21,21,35,17,139,169,48,6,6,2,220,178,
    245,126,205,79,158,116,184,165,155,171,142,164,130,169,37,33,10,39,128,160,87,130,169,46,123,249,20,133,241,163,29,27,119,162,82,201,175,1,168,132,169,34,126,2,218,134,168,8,56,254,
    40,154,243,141,0,0,4,0,87,4,205,2,222,7,153,0,13,0,25,0,37,0,49,0,37,64,16,32,26,26,14,43,38,38,7,10,4,128,0,0,20,20,14,0,47,51,17,51,47,26,204,50,51,50,47,196,90,23,6,32,34,24,73,
    64,9,34,6,6,5,68,84,10,32,33,138,11,35,1,53,62,2,68,215,5,8,62,1,149,149,156,13,117,12,105,88,79,111,14,121,15,168,254,178,43,57,57,43,42,59,59,1,88,42,57,57,42,43,58,58,254,178,24,
    44,41,19,181,28,69,72,37,5,203,150,123,73,57,63,67,121,152,254,53,52,54,50,50,54,52,53,135,7,43,1,233,24,32,63,69,39,20,35,75,69,28,146,187,53,43,64,19,26,32,32,20,48,38,64,42,44,44,
    13,7,128,10,3,3,14,14,20,133,190,32,51,69,112,5,34,26,205,50,134,193,73,143,12,33,1,50,71,164,6,130,105,32,33,138,11,35,3,30,2,23,73,55,5,36,2,222,15,168,146,137,196,33,254,116,130,
    189,32,42,130,197,33,1,173,130,189,32,43,130,197,45,190,19,40,45,24,104,35,74,68,28,6,220,121,152,133,194,33,254,194,139,189,130,91,44,50,1,251,39,69,63,32,24,28,69,75,35,20,130,193,
    63,102,4,205,2,208,7,156,0,3,0,15,0,27,0,39,0,31,64,13,16,22,22,10,39,33,3,3,2,2,4,4,10,134,187,34,16,222,205,91,143,8,33,53,23,150,171,32,19,74,83,5,39,62,2,55,2,208,253,150,115,143,
    159,46,61,27,69,73,36,103,24,44,41,19,6,120,149,149,218,144,152,43,254,20,36,74,69,28,24,32,63,69,39,0,138,153,32,11,137,153,44,34,28,28,16,6,0,13,13,12,12,22,22,16,144,153,42,46,2,
    39,53,51,30,2,23,21,5,53,24,211,20,13,66,17,12,47,139,35,74,68,28,180,19,41,44,24,254,116,2,106,254,9,66,7,15,48,6,185,28,69,74,36,20,39,69,63,32,24,214,149,149,254,234,66,2,15,60,
    0,0,1,0,179,4,222,3,233,5,179,0,13,0,29,64,12,1,12,12,5,9,9,13,128,3,7,7,11,132,148,36,26,205,50,17,51,130,2,39,48,49,65,21,7,35,39,35,134,3,50,53,3,233,78,38,46,185,47,38,47,183,46,
    38,76,5,179,46,167,102,130,0,57,167,46,255,255,0,33,0,0,5,243,6,31,4,38,0,73,0,0,0,7,0,73,2,203,0,0,133,23,33,4,81,138,23,32,76,138,23,32,64,138,23,32,79,137,23,33,7,28,136,23,32,39,
    133,71,130,79,34,76,5,150,135,79,33,7,11,146,31,32,79,131,31,130,215,60,183,255,237,5,121,5,203,0,43,0,38,64,19,39,8,28,5,5,29,4,0,26,26,0,20,13,9,114,33,93,43,10,38,18,57,57,51,17,
    51,63,72,250,5,33,23,1,108,131,15,8,115,53,52,38,35,35,53,1,46,2,35,34,6,6,21,17,35,17,52,54,54,2,230,143,205,130,30,254,235,125,193,110,109,227,178,108,185,79,79,190,87,168,152,172,
    175,111,1,40,23,79,115,78,119,154,75,203,127,250,5,203,86,155,106,254,220,8,101,180,129,130,202,113,35,41,182,44,48,150,131,126,132,148,1,57,52,70,37,90,166,115,252,89,3,176,160,243,
    136,0,1,255,233,254,21,5,40,5,204,0,38,0,30,64,16,13,20,20,25,130,172,37,4,26,6,2,114,32,134,166,48,47,23,57,51,47,51,48,49,83,50,22,22,23,19,1,51,1,130,165,24,160,54,8,33,38,38,24,
    163,139,14,8,120,213,79,96,66,33,193,1,110,219,254,14,1,22,33,50,55,38,18,53,28,32,74,43,87,109,81,45,198,254,55,217,2,73,236,37,60,55,20,55,33,32,70,5,204,53,112,86,254,35,2,194,252,
    105,253,92,76,82,31,8,6,158,11,14,71,148,118,1,238,252,195,4,17,2,64,95,91,9,11,158,12,20,0,0,3,0,192,254,20,4,189,5,182,0,20,0,29,0,39,0,32,64,15,8,9,21,21,30,30,0,31,19,18,8,29,0,
    2,114,0,43,50,63,120,117,5,34,18,57,57,130,169,40,33,32,4,21,20,6,6,7,21,65,79,6,34,33,17,35,24,110,12,8,8,75,17,17,33,50,54,53,52,38,38,35,192,1,178,1,28,1,10,64,119,84,95,136,73,
    132,237,160,254,225,205,205,252,169,148,168,170,231,1,17,169,160,74,152,120,5,182,179,188,86,133,86,15,9,16,81,140,107,140,188,94,254,20,5,57,116,113,116,107,253,152,253,255,138,130,
    80,109,56,70,29,5,63,20,4,12,5,182,6,38,0,47,0,0,1,7,0,122,1,166,0,0,0,11,182,1,23,0,1,0,0,86,0,43,52,134,37,33,5,98,132,37,32,49,133,37,33,2,61,133,37,33,37,1,138,37,34,0,254,52,93,
    91,11,36,1,80,1,155,0,133,61,32,52,24,208,71,13,75,245,6,36,100,254,52,1,193,132,85,32,44,130,85,41,6,1,80,18,0,0,11,182,1,21,130,90,32,30,135,121,32,182,132,83,83,225,8,130,83,32,
    237,130,30,8,33,1,0,82,0,0,2,116,5,182,0,11,0,40,64,19,9,4,4,8,8,5,5,6,2,114,10,3,3,11,11,2,2,1,8,24,111,192,8,33,43,50,124,197,5,8,34,48,49,97,33,53,55,17,39,53,33,21,7,17,23,2,116,
    253,222,170,170,2,34,171,171,118,47,4,107,48,118,118,48,251,149,47,130,132,37,0,56,255,233,2,166,130,87,39,17,0,14,182,12,2,114,7,71,17,6,32,48,24,163,20,12,94,151,5,8,33,1,27,73,112,
    42,48,107,59,56,92,55,205,98,177,23,27,21,173,18,25,44,109,96,4,34,251,232,151,193,93,255,255,0,59,131,163,37,7,138,6,38,3,152,130,90,42,7,0,67,255,233,1,105,0,10,179,18,130,168,36,
    0,43,206,48,49,130,35,131,199,32,159,138,35,34,118,0,141,132,35,32,19,137,35,130,37,34,0,2,208,137,35,35,1,74,255,173,132,35,32,25,137,35,33,0,58,130,107,34,141,7,72,136,107,34,106,
    255,11,130,35,35,12,180,33,21,133,36,132,109,33,255,225,130,37,34,222,7,83,135,37,35,1,81,255,143,132,73,32,29,138,73,32,46,130,35,34,153,6,221,136,35,34,76,255,220,132,35,32,14,138,
    35,32,32,130,35,34,183,7,97,136,35,34,77,255,206,132,35,32,16,138,35,38,82,254,52,2,116,5,182,133,35,42,0,7,1,80,0,190,0,0,255,255,0,141,23,35,6,1,80,99,132,21,130,81,34,116,7,88,136,
    81,34,78,0,161,65,43,17,49,254,116,4,82,5,182,4,38,3,152,0,0,0,7,0,45,2,199,132,81,50,56,255,233,3,169,7,138,6,38,3,153,0,0,1,7,1,74,0,133,132,59,33,31,12,84,13,9,32,82,132,95,32,232,
    135,95,41,2,88,3,200,1,82,0,10,179,17,140,177,32,141,139,177,35,4,14,3,203,131,95,45,255,255,0,0,3,125,6,3,4,39,3,152,1,10,131,121,37,1,83,254,0,255,150,65,189,6,32,116,130,227,33,
    6,3,130,145,130,15,65,133,36,66,187,7,32,6,173,53,66,153,6,130,53,130,229,143,123,35,0,1,0,171,130,251,58,116,4,78,0,3,0,12,181,2,6,114,1,10,114,0,43,43,48,49,97,35,17,51,1,116,201,
    201,130,25,37,1,255,139,254,20,1,131,35,41,16,0,14,182,11,6,114,7,0,15,130,36,32,50,130,37,32,83,74,141,8,107,27,5,46,6,50,51,85,31,33,64,40,67,84,201,67,143,254,20,24,254,3,13,32,
    0,130,125,43,171,254,20,4,189,6,31,6,6,1,126,0,130,15,33,255,232,130,15,34,109,4,87,130,15,32,147,131,15,33,0,109,130,15,38,68,6,32,6,38,3,186,130,151,40,6,2,54,107,0,0,10,179,58,93,
    105,10,32,73,130,139,38,230,6,20,6,38,0,79,131,33,40,0,122,61,0,0,11,182,1,21,68,81,11,131,101,34,91,4,98,130,35,32,81,130,35,35,7,0,122,1,130,77,47,0,14,180,1,25,14,0,0,184,254,20,
    176,86,0,43,52,130,241,8,48,92,254,52,3,231,4,97,0,21,0,51,0,62,0,50,64,25,8,7,7,29,59,59,33,53,52,52,41,40,40,22,33,11,114,27,18,45,22,7,114,15,0,0,47,50,43,50,63,43,125,25,8,52,17,
    51,51,17,51,48,49,65,34,38,53,52,54,54,55,23,14,2,21,20,22,70,31,6,39,3,50,22,21,17,35,39,35,107,125,5,36,53,52,36,37,55,104,183,10,33,7,6,107,122,6,51,53,2,111,106,110,71,109,56,102,
    58,82,43,49,42,34,51,17,28,61,73,24,192,134,33,58,254,52,107,93,76,141,120,44,31,60,107,104,56,46,47,9,5,125,8,11,6,45,181,195,253,23,160,25,4,50,28,37,255,255,0,109,254,52,95,3,11,
    46,1,80,1,98,0,0,255,255,0,60,254,52,1,153,5,92,129,7,40,6,1,80,234,0,0,2,0,161,130,45,8,37,83,4,78,0,21,0,45,0,36,64,19,45,6,114,35,6,114,8,7,7,26,40,40,15,0,30,11,114,24,10,0,63,
    43,204,50,51,17,51,130,2,33,43,43,65,11,22,32,1,65,8,10,8,33,17,51,17,20,22,51,50,54,54,53,17,2,162,109,109,60,99,57,130,60,86,45,50,42,32,53,17,29,62,1,137,161,28,11,24,251,45,12,
    131,233,40,62,117,103,42,17,51,96,90,47,135,233,52,26,251,178,151,57,76,38,84,177,140,2,209,253,75,133,132,91,175,127,2,53,130,173,8,41,109,254,20,4,68,4,98,0,34,0,51,0,35,64,19,26,
    42,42,29,11,114,18,11,15,114,5,6,114,3,35,35,0,7,114,0,43,50,17,51,43,43,50,131,6,130,172,36,50,22,23,51,55,130,144,75,69,5,82,217,5,35,53,53,52,54,79,71,5,36,2,17,16,18,23,79,230,
    5,35,51,50,62,2,130,24,8,89,38,38,2,43,111,170,59,10,23,164,109,224,172,119,198,82,83,210,117,143,151,5,2,9,55,170,113,208,235,235,252,91,125,65,143,140,79,115,72,35,64,133,4,98,85,
    84,149,251,161,155,212,108,34,36,176,40,46,160,142,42,30,87,24,88,85,1,43,1,12,1,10,1,53,167,95,183,131,198,206,44,90,138,94,46,140,177,84,0,255,255,66,243,14,42,7,1,74,0,175,0,0,0,
    10,179,65,66,245,10,132,223,33,5,248,67,23,6,36,7,1,77,0,209,132,35,32,56,144,35,32,239,136,35,34,78,1,163,132,35,67,61,8,8,37,0,1,255,240,255,225,3,219,6,31,0,43,0,37,64,18,17,39,
    39,19,20,20,36,6,114,32,25,1,114,7,6,13,13,0,0,47,50,16,102,21,5,32,17,74,129,5,39,69,34,38,53,52,54,55,23,66,180,8,35,17,35,53,55,130,17,32,54,24,74,46,8,8,110,6,21,21,33,21,33,17,
    20,6,6,1,37,145,164,10,9,168,4,5,65,49,58,66,188,188,85,162,114,69,115,42,52,34,84,47,87,82,1,14,254,242,70,137,31,156,127,32,57,26,44,15,31,17,64,66,80,96,2,129,97,61,76,139,169,78,
    24,15,154,11,19,112,113,77,155,253,137,111,155,81,0,2,0,107,255,236,4,108,6,30,0,33,0,47,0,25,64,13,34,25,9,3,0,41,17,11,114,1,0,1,114,0,24,164,148,8,41,65,23,6,4,6,21,20,22,22,23,
    74,62,8,67,139,5,33,46,2,131,159,33,36,3,67,146,7,8,88,53,52,38,38,4,55,23,213,254,211,158,62,131,102,128,182,97,129,232,155,148,230,131,93,171,116,65,113,70,95,200,1,64,216,81,148,
    94,161,140,148,161,73,129,6,30,173,24,40,61,57,41,58,65,51,63,148,190,130,155,216,114,109,210,154,125,188,128,35,35,75,100,73,83,116,81,54,253,62,21,90,154,119,143,168,170,152,97,136,
    100,70,127,8,69,201,7,63,0,1,0,112,255,59,2,213,2,225,0,23,0,24,64,11,5,4,17,17,8,124,23,125,13,22,123,0,63,51,237,228,79,193,5,34,83,17,20,82,85,8,33,17,35,79,25,5,32,21,130,8,8,37,
    242,4,3,8,34,117,71,126,134,130,75,78,112,88,130,2,225,254,255,31,59,19,51,53,115,127,254,82,1,159,79,80,122,115,254,175,3,166,134,99,32,211,130,99,49,18,0,25,64,14,15,14,4,5,11,8,
    6,9,124,18,125,13,17,133,102,35,23,57,48,49,135,100,59,55,55,51,1,1,35,3,7,21,35,17,241,4,2,4,15,50,20,228,151,254,225,1,51,154,242,86,129,130,97,47,44,28,70,30,19,58,21,225,254,229,
    254,134,1,45,70,231,135,95,33,0,242,130,95,38,3,0,10,179,2,125,1,130,84,42,237,48,49,87,35,17,51,242,130,130,197,135,33,57,4,122,1,220,0,38,0,37,64,17,28,27,18,18,32,9,0,0,32,32,25,
    124,5,14,14,24,130,47,37,51,17,51,237,50,47,120,102,6,38,51,48,49,65,50,22,21,139,234,80,13,6,130,18,80,13,7,63,23,51,54,54,3,137,119,122,130,70,68,98,86,130,70,69,67,81,35,130,103,
    18,8,21,65,81,45,80,111,26,9,34,130,109,43,114,126,254,79,1,162,78,78,109,105,254,152,131,7,47,54,104,76,254,172,2,149,91,34,46,23,54,55,56,53,0,65,25,6,32,213,130,151,39,20,0,25,64,
    11,18,17,9,130,149,32,15,130,147,131,145,133,143,146,139,130,129,37,54,54,1,212,124,133,24,139,19,10,35,1,220,115,128,65,116,9,44,2,149,92,52,52,0,2,0,112,254,21,2,239,130,93,53,21,
    0,34,0,35,64,16,10,9,29,29,6,19,18,22,22,0,0,16,124,14,6,130,100,33,206,228,132,244,132,247,134,105,38,20,6,35,34,38,39,35,25,2,173,8,34,54,54,23,24,223,87,8,8,57,53,52,38,1,208,130,
    157,159,132,83,105,30,8,2,6,130,107,17,6,31,104,57,103,87,2,85,108,91,91,90,1,220,171,169,170,175,58,40,23,62,25,254,242,3,187,95,44,63,99,112,111,20,118,126,135,112,112,128,131,235,
    36,66,255,47,2,72,130,141,48,41,0,24,64,13,13,0,17,38,21,34,6,31,24,124,10,3,130,136,39,51,228,50,23,57,48,49,69,87,182,11,130,109,35,38,39,46,2,85,157,5,106,162,10,8,71,23,30,2,2,
    72,153,137,75,109,44,47,125,59,85,74,29,76,69,69,96,51,149,122,66,114,53,42,47,99,51,67,71,34,78,66,65,96,52,12,96,101,21,20,106,21,32,50,42,24,38,38,24,24,50,69,56,85,91,24,22,92,
    18,25,40,35,27,36,34,23,23,49,71,131,147,56,24,255,47,1,211,2,102,0,23,0,29,64,13,11,20,20,17,13,14,14,16,17,124,0,7,131,147,34,244,205,51,75,82,7,8,59,69,50,54,55,21,6,6,35,34,38,
    53,17,35,53,55,55,51,21,51,21,35,17,20,22,1,105,28,57,21,22,74,38,88,122,99,103,47,80,207,207,58,111,9,7,92,9,13,88,119,1,117,56,44,143,150,93,254,141,56,55,131,103,40,113,0,0,4,99,
    5,12,0,35,130,251,50,12,26,11,29,8,4,0,18,9,121,114,28,0,0,47,50,43,50,18,131,251,37,115,17,52,54,54,55,130,2,34,1,51,1,132,8,72,160,5,38,7,6,6,7,1,35,1,130,5,8,78,6,21,17,114,12,31,
    30,27,93,66,254,252,222,1,179,12,21,9,48,40,199,24,28,24,95,67,1,6,223,254,76,31,77,20,23,1,201,58,100,89,44,44,62,17,1,165,253,48,6,13,9,31,100,88,1,217,254,40,82,123,50,44,75,20,
    254,86,2,212,3,48,55,59,102,254,55,0,1,0,87,130,145,8,33,30,5,32,0,29,0,20,64,9,9,16,122,114,27,1,1,0,10,0,63,50,17,51,43,50,48,49,115,53,33,17,52,38,39,87,218,8,35,51,50,22,23,68,
    185,5,8,32,17,51,21,88,2,105,98,95,25,63,40,50,144,103,72,153,82,99,136,61,61,81,25,20,21,9,147,162,2,229,109,108,16,130,170,47,8,163,8,8,17,22,22,68,52,29,68,79,46,253,21,162,130,
    111,36,54,255,246,2,187,130,111,8,38,38,0,23,64,11,31,10,33,32,17,23,122,7,0,10,114,0,43,50,63,51,57,57,63,48,49,87,34,38,39,55,22,22,51,50,54,55,54,54,84,42,6,34,7,53,54,133,121,36,
    21,17,35,39,35,130,89,8,67,6,6,184,27,65,38,24,30,61,33,75,124,39,27,30,51,75,38,80,48,94,92,75,107,35,50,41,159,27,11,20,51,23,42,115,10,8,6,181,6,7,69,63,43,111,67,1,188,81,96,10,
    11,166,21,34,31,42,141,88,252,48,202,35,67,21,41,48,0,130,141,52,44,0,0,3,243,5,12,0,17,0,16,182,11,8,8,9,121,114,0,0,47,71,104,5,34,97,17,52,131,128,36,55,33,53,33,21,65,117,5,8,39,
    2,99,24,18,20,37,25,253,77,3,199,50,80,25,43,3,111,51,86,28,31,40,15,162,146,18,48,37,59,108,252,148,0,2,0,171,0,0,4,114,130,221,34,27,0,31,131,223,44,28,10,30,30,7,7,17,122,114,0,
    10,0,63,131,86,32,47,130,223,130,88,65,80,10,130,227,133,217,8,64,23,22,22,21,17,33,17,51,17,3,167,94,95,24,73,46,68,209,155,49,94,45,79,130,53,114,147,61,61,82,23,16,13,252,58,201,
    3,135,108,107,16,7,7,12,14,164,5,7,4,5,5,22,27,27,80,62,43,91,57,252,121,3,24,252,232,131,207,130,127,33,1,117,130,207,36,3,0,11,180,1,130,204,131,118,34,48,49,115,130,85,37,171,202,
    5,12,250,244,131,35,36,75,0,0,2,37,130,35,35,22,0,17,183,134,243,134,157,131,41,137,244,34,14,2,7,131,247,8,43,6,21,17,184,34,31,24,51,28,254,235,1,218,23,40,34,13,17,22,6,4,5,2,174,
    90,158,57,46,69,24,162,148,17,51,58,30,40,95,42,27,63,35,253,82,75,149,5,37,4,114,5,32,0,29,131,255,38,18,10,28,25,25,1,7,135,255,34,50,17,51,130,255,35,115,17,54,54,141,245,32,35,
    66,98,9,44,17,171,18,61,44,91,163,71,115,148,61,62,81,130,244,55,203,101,101,22,66,42,43,115,72,5,6,2,5,4,8,7,22,25,27,79,63,42,93,58,130,242,41,135,112,111,14,4,4,5,5,251,142,131,
    213,36,162,255,236,4,165,130,119,60,54,0,21,64,11,30,37,122,114,10,121,114,18,0,11,114,0,43,50,43,43,50,48,49,69,34,38,39,38,131,2,38,53,17,51,17,20,22,23,66,96,8,66,221,18,36,22,22,
    21,20,6,133,252,8,111,2,160,70,131,53,52,89,33,42,40,203,49,49,34,108,69,73,110,35,54,37,35,48,39,116,68,19,38,20,30,59,28,71,127,53,50,78,27,32,34,25,24,32,102,69,55,134,20,27,31,
    30,92,67,82,209,134,2,128,253,128,144,205,62,44,49,53,48,69,214,119,118,205,68,55,49,4,4,164,5,5,30,34,30,93,64,76,204,129,101,171,71,90,132,41,33,33,0,1,0,158,1,204,1,105,5,12,0,4,
    0,14,181,3,130,202,32,121,131,179,36,47,51,48,49,83,130,169,42,7,158,203,136,1,204,3,64,253,105,169,130,26,47,0,36,254,20,3,103,5,31,0,32,0,12,180,9,22,122,66,173,5,34,48,49,65,65,
    64,5,35,39,38,35,34,132,183,32,53,65,97,15,8,60,2,155,28,31,15,42,25,76,125,45,82,38,25,73,26,28,70,42,44,95,51,106,182,56,29,47,18,34,33,254,20,4,190,87,137,55,29,47,17,54,8,7,4,18,
    5,169,7,13,5,6,5,69,55,28,67,39,70,166,96,251,67,130,163,36,69,255,236,3,136,130,119,40,56,0,16,183,24,37,122,114,8,65,89,6,65,88,6,32,53,95,230,5,132,117,33,53,53,65,81,8,34,6,6,7,
    140,137,34,30,2,21,65,88,8,8,80,1,144,98,162,71,36,103,118,56,62,97,33,32,47,13,14,21,42,46,42,132,81,60,122,63,15,23,6,32,58,39,44,101,56,105,183,57,28,47,19,22,29,15,27,28,25,74,
    54,66,159,20,17,16,176,10,19,12,34,28,26,72,37,38,105,58,176,101,167,55,52,51,15,16,4,6,1,169,9,11,6,5,133,176,45,47,105,115,65,175,87,148,65,57,87,31,55,36,0,130,185,50,58,0,0,3,176,
    6,27,0,10,0,20,64,9,8,3,3,5,6,121,67,141,5,8,63,205,51,17,51,48,49,97,19,19,33,17,51,17,33,21,3,3,1,205,98,184,253,83,202,2,172,184,97,1,238,2,122,1,179,254,245,157,253,127,254,14,
    0,0,2,0,166,0,0,4,109,5,32,0,17,0,32,0,15,182,29,7,122,114,18,130,81,37,63,50,43,50,48,49,66,207,17,32,37,69,51,6,69,54,6,58,166,17,60,43,92,163,72,118,151,64,56,76,24,16,15,253,3,
    2,50,31,32,23,67,43,23,68,45,66,216,6,54,3,8,8,23,29,25,73,57,38,89,54,252,100,162,2,254,59,79,25,21,24,5,4,130,172,130,193,32,100,130,123,32,105,130,123,48,53,0,31,64,16,37,36,10,
    52,19,18,46,46,25,122,114,15,134,200,38,43,50,17,51,51,51,63,130,204,41,115,19,54,54,53,52,38,39,46,2,130,121,37,39,51,22,22,23,51,66,8,15,32,33,69,207,5,33,46,2,66,41,6,8,99,3,100,
    88,2,1,7,8,2,15,14,3,4,9,4,190,18,24,5,14,13,78,39,39,127,73,71,116,43,30,47,17,30,30,254,43,1,10,30,30,17,50,68,44,61,106,33,32,50,16,89,3,171,22,34,16,32,58,35,12,46,45,9,12,24,12,
    38,69,31,20,60,20,21,33,38,33,23,56,33,57,150,91,252,193,162,2,157,73,113,36,28,41,23,36,24,24,56,32,252,51,0,130,195,44,93,254,26,1,106,5,12,0,17,0,10,179,8,69,100,5,38,48,49,83,17,
    52,38,38,135,169,8,37,30,2,21,17,160,13,24,16,3,7,4,204,4,9,5,8,22,17,254,26,5,81,67,133,123,51,11,21,11,11,30,20,25,120,146,65,250,175,131,75,47,111,0,0,2,225,5,32,0,34,0,15,182,12,
    19,122,114,70,174,5,32,43,70,172,5,70,49,5,70,175,16,8,67,22,22,21,17,20,6,6,7,7,111,1,146,11,10,22,23,17,47,31,39,83,46,39,91,53,68,96,34,32,47,14,18,19,10,15,7,9,162,50,98,48,2,99,
    45,73,24,16,19,8,8,167,8,7,26,23,22,64,37,34,87,50,253,157,51,117,105,33,52,130,118,37,0,106,255,236,4,109,130,123,42,35,0,60,0,23,64,11,13,12,49,49,130,129,32,36,24,158,108,8,37,51,
    51,48,49,69,34,68,104,6,40,52,54,54,55,55,7,53,62,2,69,224,9,34,20,6,7,130,133,38,6,39,50,54,55,62,2,67,40,11,8,139,21,20,22,23,22,22,2,106,140,101,75,111,34,25,26,35,72,53,3,146,92,
    171,159,73,81,140,57,58,88,30,30,31,20,19,32,111,77,108,148,74,110,35,33,39,16,67,56,33,93,52,47,71,37,80,76,43,45,35,109,20,55,38,134,95,71,173,102,111,181,141,51,3,7,167,5,7,5,38,
    40,36,110,73,73,183,111,87,153,65,106,150,45,62,167,54,50,45,128,149,75,176,206,52,32,34,2,4,85,240,169,139,199,59,50,54,0,1,0,63,255,200,4,133,5,12,0,25,0,24,64,13,23,1,2,5,9,20,6,
    0,14,6,65,159,5,72,141,5,34,87,53,37,130,197,46,1,51,19,19,62,2,55,19,51,3,14,2,7,6,4,130,195,8,57,63,1,3,28,57,28,254,215,199,154,112,83,129,82,12,54,194,53,8,39,63,43,81,254,236,
    192,93,170,56,164,39,5,11,8,4,97,253,191,254,31,44,143,190,114,2,55,253,208,80,144,130,57,130,167,29,14,25,0,130,119,44,84,254,20,4,78,5,32,0,47,0,23,64,10,130,249,35,22,0,7,37,68,
    203,6,36,18,57,47,51,51,68,208,8,66,164,5,65,27,5,41,23,23,7,39,34,38,39,38,38,53,70,162,5,32,54,72,17,6,8,45,23,22,22,21,17,3,130,44,43,38,114,71,70,127,37,35,39,44,43,28,70,43,38,
    33,37,92,152,52,45,51,65,54,26,75,34,57,125,68,81,139,56,50,82,30,37,39,130,121,8,42,114,142,198,57,55,47,45,45,40,101,57,73,110,32,21,28,8,6,148,1,69,62,54,149,94,93,160,54,26,51,
    16,26,26,35,36,30,94,63,78,204,126,251,142,130,171,36,106,255,236,4,98,130,171,45,74,0,25,64,12,40,41,41,0,27,57,122,114,9,65,240,7,118,157,5,38,69,34,38,38,39,39,53,70,82,7,32,55,
    67,133,5,130,19,73,54,5,33,6,6,137,193,132,20,136,196,66,24,16,8,136,6,2,92,63,121,103,36,43,94,184,92,68,105,34,38,44,8,5,4,14,18,19,65,52,30,66,37,74,111,34,43,51,44,37,31,74,42,
    38,32,37,56,98,42,39,66,24,33,33,77,64,73,188,100,86,144,57,46,77,28,37,39,47,43,36,99,56,49,119,20,4,9,5,5,161,10,10,45,38,42,130,69,47,81,50,83,129,63,65,99,28,16,16,40,30,39,116,
    63,67,111,35,27,31,1,6,150,3,1,25,23,22,61,38,50,128,75,108,172,53,63,61,39,38,30,91,58,78,204,128,143,221,77,61,86,25,26,27,0,130,249,53,4,254,20,3,151,5,12,0,21,0,23,64,12,20,19,
    1,4,7,5,0,11,2,66,28,11,36,65,17,1,51,19,74,163,14,8,46,7,17,1,22,254,238,212,251,26,50,25,70,79,202,47,46,36,121,79,109,254,20,4,29,2,219,253,72,7,14,6,16,132,109,1,156,254,94,96,
    149,52,39,62,12,26,252,94,130,103,36,76,0,0,4,48,130,103,8,40,26,0,29,64,13,8,24,0,12,5,121,114,25,3,3,1,1,0,0,47,50,17,51,17,51,43,50,18,57,57,48,49,115,53,33,23,1,1,51,1,23,130,113,
    66,137,6,8,69,6,7,6,6,7,1,21,84,2,91,129,254,216,254,68,220,1,82,56,70,82,11,25,194,24,5,12,18,12,13,37,23,28,67,39,1,15,162,3,1,167,2,198,253,210,86,74,187,114,1,13,255,0,42,77,70,
    33,40,74,37,46,82,36,254,131,118,0,2,0,171,130,158,32,163,132,127,32,30,130,233,40,10,27,26,28,28,15,12,12,13,130,232,32,43,130,126,61,51,47,47,47,48,49,97,52,52,53,52,54,55,62,2,55,
    19,33,53,33,21,3,14,2,7,14,3,21,21,1,130,158,8,50,2,167,32,20,6,21,27,12,175,252,223,3,248,185,7,25,24,7,8,20,20,12,253,60,203,8,16,8,50,161,84,24,82,93,43,2,49,162,136,253,178,20,
    84,87,28,26,86,95,84,24,32,130,112,35,235,251,21,0,74,127,6,41,113,5,32,0,25,0,12,180,7,15,71,207,8,74,34,9,34,6,7,53,66,232,12,8,44,2,167,99,97,24,56,32,41,115,121,50,91,153,63,126,
    147,62,61,82,24,15,13,3,135,110,109,16,5,5,3,7,4,164,7,7,22,27,27,79,63,43,91,57,252,121,131,97,36,87,0,0,5,124,130,221,8,33,40,0,29,64,15,29,10,10,21,19,20,18,3,5,0,1,121,114,22,0,
    0,47,50,43,18,23,57,51,17,51,48,49,115,3,65,202,6,38,55,19,51,3,6,6,7,130,2,40,34,6,35,19,33,50,54,55,54,137,18,8,83,6,6,35,196,109,194,59,59,108,36,42,43,8,30,190,30,12,62,63,56,159,
    112,2,5,1,22,1,20,130,219,72,71,16,53,191,54,10,64,50,51,133,92,69,158,86,5,12,253,51,14,50,42,50,148,85,1,72,254,186,129,189,69,64,74,17,1,254,250,139,127,131,170,2,52,253,200,114,
    199,85,85,129,42,38,32,131,165,52,48,255,250,4,137,5,32,0,47,0,21,64,9,30,10,36,36,18,122,114,6,132,159,35,50,17,51,47,76,19,5,38,53,22,51,50,54,53,17,130,152,69,82,5,34,22,23,22,131,
    2,79,159,5,72,250,5,33,17,20,130,184,130,186,8,111,155,24,52,31,43,45,69,68,36,72,35,102,179,153,65,93,129,95,42,55,72,21,14,11,201,101,99,45,75,35,83,47,16,19,27,44,38,107,6,10,10,
    152,7,88,91,3,26,2,6,2,164,8,12,6,12,27,21,28,77,60,37,89,53,252,116,3,134,112,109,16,9,2,3,252,219,58,96,40,57,35,29,29,255,255,0,87,0,0,5,124,6,16,6,38,3,226,0,0,1,7,4,35,5,33,0,
    114,0,10,179,41,1,121,110,239,8,133,35,32,13,136,35,36,36,0,166,0,111,148,35,134,71,39,0,39,4,33,3,53,255,108,132,79,32,36,132,79,32,52,151,79,138,43,34,36,0,172,132,87,139,43,38,113,
    255,31,4,99,5,12,130,123,42,201,0,0,0,7,4,28,2,103,255,232,130,183,34,113,254,85,140,23,36,29,2,102,255,238,131,23,33,0,0,140,23,36,33,1,245,254,215,133,231,35,4,30,5,32,130,71,32,
    202,132,71,36,33,1,127,0,71,130,23,77,185,6,130,23,32,203,134,23,32,4,132,23,77,67,6,130,23,32,204,134,23,32,82,132,23,77,11,6,130,23,32,205,133,23,33,2,133,131,23,37,255,188,0,0,1,
    117,132,167,32,206,130,23,35,6,4,33,1,130,93,32,255,130,107,33,2,37,132,21,32,207,133,21,33,15,72,130,115,75,213,6,130,67,32,209,134,67,32,150,132,67,32,190,75,51,5,130,23,32,210,133,
    23,35,0,3,1,56,130,47,75,31,6,130,23,32,211,136,139,131,71,74,191,6,130,23,32,212,134,23,33,94,0,131,23,74,29,6,130,23,32,213,134,23,32,64,132,187,32,100,73,115,5,130,23,32,215,133,
    23,33,2,130,132,23,72,123,6,130,23,32,217,133,23,33,1,65,132,71,72,23,6,130,23,32,218,133,23,33,2,103,132,47,70,233,6,130,23,32,220,134,23,34,68,1,7,130,167,131,47,34,98,5,32,130,23,
    32,221,134,23,34,97,0,248,130,23,69,11,6,130,23,32,223,133,23,35,1,5,255,166,88,125,6,34,163,5,12,130,23,32,224,133,23,35,2,84,0,70,130,23,36,44,0,0,3,113,132,71,32,225,65,123,12,36,
    87,0,0,5,124,132,47,32,226,133,23,35,3,53,255,108,130,47,67,103,6,130,71,32,227,133,23,35,2,186,0,68,130,23,78,55,5,32,211,130,23,46,206,0,0,1,7,4,30,1,17,255,183,0,10,179,4,66,139,
    9,55,252,4,4,217,253,196,6,33,4,7,0,67,251,178,0,0,255,255,253,120,4,217,255,56,132,17,34,118,253,38,131,17,45,254,152,4,217,1,106,6,32,4,7,1,74,254,70,131,17,39,252,10,4,219,255,6,
    5,233,130,17,34,81,251,184,132,35,38,203,4,219,1,54,5,116,130,17,34,76,254,121,132,17,32,183,130,53,34,78,5,248,130,17,34,77,254,101,131,17,39,255,141,5,0,0,118,5,239,130,17,34,78,
    255,59,132,35,38,214,5,10,1,42,5,223,130,125,34,106,253,168,132,35,38,31,4,217,0,235,6,147,130,35,34,79,254,205,132,35,32,247,130,71,32,244,131,143,35,1,82,254,165,142,143,32,75,131,
    143,8,46,0,2,251,198,4,217,254,190,6,33,0,12,0,25,0,37,64,17,4,17,17,19,11,24,24,0,13,19,128,6,6,15,19,1,19,0,47,93,51,47,26,16,205,50,50,17,51,131,1,32,48,108,227,6,41,46,3,39,53,
    35,30,2,23,21,35,131,9,46,254,10,22,62,67,29,112,34,83,81,66,16,154,22,61,131,10,48,84,80,66,17,6,33,46,111,107,39,25,29,81,89,81,28,20,138,10,8,36,255,255,255,88,3,193,0,152,5,182,
    4,7,2,5,255,61,0,0,0,1,253,34,254,141,254,12,255,124,0,11,0,8,177,0,6,0,47,89,149,8,57,51,50,22,21,20,6,253,150,48,68,68,48,50,68,68,254,141,57,62,64,56,56,64,62,57,0,130,71,44,49,
    254,20,0,206,0,0,4,7,0,122,255,37,131,227,44,255,82,254,52,0,175,0,31,4,7,1,80,255,130,88,130,89,51,60,4,210,254,60,6,64,0,18,0,12,179,14,3,128,15,0,47,26,204,131,93,32,52,104,16,5,
    8,41,14,2,21,20,22,23,21,46,2,253,60,72,61,50,57,31,40,32,59,60,79,115,62,5,184,61,75,44,42,26,27,15,18,21,27,53,17,76,16,64,92,0,130,75,32,63,130,75,32,62,134,75,35,16,128,5,4,130,
    167,37,26,204,48,49,65,20,77,220,5,33,53,52,83,153,7,8,57,254,62,62,114,79,59,60,31,41,31,58,50,60,71,5,184,58,92,64,16,76,17,53,27,21,18,15,27,26,42,44,75,0,1,252,81,4,140,255,59,
    5,185,0,20,0,15,180,13,9,9,3,0,0,47,205,51,124,16,131,78,83,83,5,130,160,33,33,54,104,181,8,8,33,253,15,6,43,47,51,43,44,52,1,202,5,46,46,50,45,19,43,35,4,234,43,51,57,51,53,46,46,
    48,52,51,36,46,22,130,83,50,89,4,224,255,75,5,232,0,22,0,18,182,17,16,16,10,0,128,7,131,238,34,50,51,47,131,241,8,70,50,30,2,21,21,35,38,38,35,34,14,2,35,35,53,51,50,62,2,254,88,53,
    89,65,36,136,2,61,47,45,96,115,143,91,18,15,78,130,116,113,5,232,24,54,87,63,36,71,50,35,46,34,139,36,47,36,0,0,2,255,188,254,11,0,78,255,169,0,11,0,23,130,175,41,12,18,18,0,6,0,124,
    47,51,50,131,88,32,87,101,107,10,32,3,138,11,40,4,35,37,37,35,36,38,38,36,134,7,60,245,40,38,39,41,41,39,38,40,255,0,41,38,38,41,41,37,39,41,0,5,254,158,254,6,1,84,255,164,130,89,54,
    22,0,34,0,46,0,58,0,32,64,13,47,53,53,6,35,41,41,23,29,29,0,17,132,104,36,51,50,17,51,51,66,132,7,32,69,138,100,33,51,34,85,11,5,35,21,20,6,7,138,22,99,237,11,32,39,138,23,33,254,230,
    134,140,43,254,72,37,36,35,38,38,180,36,37,39,34,130,163,33,1,111,132,23,32,37,130,20,131,180,35,37,37,251,41,133,172,33,41,79,132,179,37,41,255,40,39,40,39,131,182,33,40,39,130,186,
    130,190,32,255,135,32,35,0,3,254,208,130,195,32,37,130,195,52,3,0,15,0,27,0,24,64,9,4,10,10,16,16,22,22,0,1,0,124,47,138,183,35,53,33,21,19,138,140,140,152,43,208,1,105,161,35,37,38,
    34,36,39,38,37,134,7,36,214,87,87,254,220,135,119,37,255,41,39,39,40,41,130,124,138,111,50,7,0,19,0,31,0,27,64,10,8,14,14,20,20,26,26,0,128,3,130,112,34,26,24,204,65,42,8,34,67,53,
    35,130,116,33,35,21,103,27,11,139,118,39,168,136,1,103,136,1,44,36,130,248,35,36,38,38,36,134,7,38,254,96,202,87,87,202,90,137,234,132,243,51,39,41,0,0,1,255,182,255,7,0,74,255,165,
    0,10,0,9,177,0,5,130,110,35,51,48,49,71,65,132,9,40,2,72,38,34,38,38,39,249,79,130,172,33,38,38,130,49,40,2,255,32,255,14,0,213,255,172,66,59,6,36,0,6,6,12,18,65,22,6,33,48,49,66,59,
    11,65,179,11,32,139,131,137,47,35,39,39,254,185,34,37,38,33,37,37,37,242,40,38,40,130,254,33,37,41,65,15,7,38,0,0,3,255,32,254,6,130,89,32,164,132,89,45,35,0,25,64,9,0,6,24,24,30,12,
    18,18,30,131,96,42,17,51,17,51,24,16,206,50,48,49,67,65,116,10,32,55,66,37,22,32,6,134,251,32,109,143,121,33,254,6,65,7,8,33,40,39,66,12,5,33,41,39,131,221,65,15,5,43,69,255,55,0,182,
    255,142,0,3,0,9,177,65,239,5,8,34,48,49,71,53,33,21,187,1,113,201,87,87,0,0,1,255,71,254,103,0,181,255,136,0,7,0,10,178,0,128,3,0,47,26,204,130,146,65,152,6,42,45,140,1,110,140,254,
    103,201,88,88,201,131,43,47,183,5,126,0,75,6,28,0,10,0,8,177,5,0,0,47,132,188,66,225,8,47,1,72,37,36,36,39,39,5,126,78,40,40,42,38,38,40,131,49,32,187,130,38,36,78,5,158,0,11,130,49,
    32,6,133,49,32,83,138,226,36,3,35,37,37,35,131,51,32,0,130,188,46,40,41,38,38,41,0,3,255,30,253,219,0,236,255,160,65,55,6,36,22,183,24,30,30,67,210,7,35,24,205,50,47,131,2,130,196,
    138,69,32,23,150,11,40,154,35,37,39,33,36,37,37,120,131,101,39,37,38,38,122,35,38,38,35,130,7,32,254,65,173,5,38,38,40,148,41,39,38,41,130,117,33,42,147,65,51,11,38,187,2,6,0,78,2,
    163,139,229,32,83,66,66,9,33,3,72,132,177,35,2,6,78,39,130,176,8,38,37,41,0,0,1,255,208,254,46,0,46,255,112,0,3,0,10,178,0,128,1,0,47,26,205,48,49,67,17,51,17,48,94,254,46,1,66,254,
    190,130,35,65,9,7,65,59,10,145,85,33,5,0,66,153,8,38,1,255,182,5,0,0,74,65,59,13,66,42,11,32,2,130,202,36,34,38,38,39,5,65,59,9,48,1,255,80,254,197,0,154,255,181,0,7,0,18,181,6,2,2,
    67,114,11,65,213,9,61,52,124,1,74,126,254,197,158,82,82,158,0,255,255,0,41,255,239,2,174,3,127,6,7,3,119,0,0,252,172,130,17,42,80,0,0,1,251,3,109,6,7,0,123,134,17,38,51,0,0,2,134,3,
    128,130,17,32,116,134,17,36,43,255,241,2,151,131,53,33,0,117,134,17,32,19,130,35,38,197,3,115,6,7,2,55,134,17,32,68,130,89,32,155,131,71,33,2,56,134,17,38,44,255,240,2,176,3,126,130,
    107,32,120,134,17,32,58,130,53,32,163,132,35,32,57,134,17,32,50,130,35,34,166,3,125,130,71,130,27,132,143,32,37,130,17,34,170,3,129,130,53,32,121,131,35,61,0,2,0,114,255,236,4,68,5,
    205,0,17,0,31,0,16,183,28,14,5,114,21,5,13,114,0,43,50,43,50,24,126,109,14,8,86,54,51,50,22,18,5,16,18,51,50,18,17,52,2,38,35,34,6,2,4,68,54,118,186,132,126,183,121,58,96,214,178,168,
    216,106,252,248,130,156,155,132,56,126,105,106,126,54,2,221,178,254,232,194,101,101,194,1,23,179,235,1,80,181,179,254,175,236,254,219,254,221,1,34,1,38,192,1,3,131,131,254,253,0,0,
    1,0,45,130,181,49,114,5,182,0,13,0,21,64,10,10,9,9,5,11,4,114,13,12,130,128,8,48,43,50,50,47,51,48,49,97,17,52,54,54,55,6,6,7,7,39,1,51,17,1,171,2,3,3,26,58,37,167,102,1,158,167,3,
    221,53,89,81,38,27,49,31,135,132,1,66,250,74,131,77,38,77,0,0,4,27,5,203,87,137,5,40,9,17,5,114,26,1,27,27,0,131,78,34,50,17,51,89,220,6,36,1,62,2,53,52,131,190,35,7,39,62,2,130,211,
    34,22,21,20,130,94,8,55,1,21,33,21,77,1,129,110,145,74,134,110,101,162,85,108,59,137,165,101,136,199,107,93,169,116,254,226,2,208,158,1,135,111,167,157,93,114,120,73,68,134,50,81,48,
    96,176,118,117,199,196,111,254,230,9,179,131,117,36,85,255,236,4,32,130,117,50,45,0,29,64,13,4,3,29,29,26,26,11,36,43,5,18,11,13,0,25,63,99,7,42,18,57,57,48,49,65,20,6,7,21,22,132,
    111,24,86,73,17,34,51,50,54,24,123,97,13,8,47,3,244,171,136,174,177,120,249,194,118,200,90,92,212,97,127,158,73,90,177,132,137,139,123,160,78,135,124,114,169,75,99,82,230,149,225,234,
    4,101,149,178,28,7,22,178,146,127,198,24,86,96,7,62,86,86,109,52,166,65,118,79,103,113,70,51,139,62,88,198,0,0,2,0,45,0,0,4,112,5,188,0,10,0,21,131,163,52,6,3,11,11,9,16,4,1,1,4,4,
    0,12,0,63,63,57,47,18,57,51,24,103,83,9,32,1,130,10,8,86,21,35,17,3,17,52,54,54,55,35,6,6,7,1,2,220,253,81,2,171,200,208,208,196,1,4,3,8,21,59,25,254,122,1,74,159,3,211,252,59,173,
    254,182,1,247,1,190,66,105,84,34,39,93,37,253,202,0,0,1,0,120,255,236,4,31,5,182,0,33,0,33,64,15,26,25,25,22,22,31,0,0,8,30,27,4,15,8,65,17,9,130,99,37,17,51,48,49,65,50,65,16,19,8,
    70,35,34,6,7,39,19,33,21,33,3,54,54,2,60,146,217,120,130,247,176,115,198,69,74,209,97,106,157,86,170,181,62,142,45,95,56,2,232,253,198,33,36,111,3,136,101,194,139,152,220,118,40,40,
    185,43,53,66,134,103,136,148,21,11,57,2,189,179,254,110,7,16,66,223,6,54,65,5,202,0,34,0,49,0,27,64,12,18,41,41,22,22,6,35,30,13,13,6,5,65,156,8,131,135,38,83,52,62,3,51,50,22,114,
    73,6,33,2,7,111,161,7,8,128,20,6,6,35,34,46,2,1,50,54,53,52,38,39,38,6,6,21,20,30,2,114,37,89,154,234,165,46,106,35,38,93,48,184,210,92,7,13,30,93,133,89,128,190,105,116,211,144,109,
    184,136,75,1,245,124,151,134,135,92,136,76,36,74,112,2,112,131,250,218,165,94,8,10,169,12,12,150,254,253,164,49,80,47,105,200,143,152,222,120,81,161,241,254,196,161,165,135,156,1,1,
    78,118,61,63,127,107,65,0,1,0,19,0,0,3,244,5,182,0,6,0,16,182,6,12,5,2,2,3,4,130,165,8,66,17,51,63,48,49,115,1,33,53,33,21,1,209,2,75,252,247,3,225,253,183,5,3,179,144,250,218,0,3,
    0,116,255,236,4,69,5,203,0,31,0,48,0,62,0,23,64,12,40,41,8,24,49,5,56,16,5,32,0,13,0,63,50,63,51,23,57,48,111,227,5,37,52,54,54,55,46,2,98,17,6,33,22,21,130,219,34,7,30,2,131,6,32,
    39,66,124,5,35,38,39,39,14,130,15,34,22,22,19,67,35,7,8,113,21,20,22,22,2,96,157,220,115,83,138,83,71,117,69,117,200,124,127,201,114,75,125,78,89,148,88,121,218,150,95,132,69,74,128,
    85,34,84,121,64,65,131,104,67,109,64,136,112,107,137,67,114,20,93,175,123,101,149,107,38,40,102,137,90,112,155,81,80,155,115,88,133,99,39,42,108,146,100,122,179,97,157,60,109,73,69,
    104,83,34,13,36,87,111,73,69,108,61,2,206,29,71,99,69,98,103,104,97,70,97,71,0,2,0,100,130,205,32,51,65,175,17,34,5,13,6,66,58,9,39,17,51,48,49,65,20,14,3,67,68,9,33,18,55,101,192,
    8,40,54,54,51,50,30,2,1,34,6,130,186,32,23,132,214,8,105,46,2,4,51,37,89,156,236,164,43,112,35,37,98,47,186,210,91,6,12,29,93,134,92,127,188,103,116,212,144,108,184,136,75,254,11,124,
    152,132,135,94,138,75,36,74,113,3,71,131,251,218,166,93,9,10,170,13,14,150,1,3,164,48,80,48,105,200,142,154,222,120,81,161,241,1,59,160,165,135,155,1,76,117,62,62,127,107,65,255,255,
    0,41,2,57,2,174,5,201,6,7,3,119,0,0,254,246,130,17,38,80,2,74,1,251,5,183,69,241,5,132,17,38,51,2,74,2,134,5,202,69,241,5,132,17,36,43,2,59,2,151,131,53,33,0,117,134,53,32,19,130,35,
    34,197,5,189,69,241,5,132,35,32,68,130,89,32,155,131,71,33,2,56,134,35,38,44,2,58,2,176,5,200,69,241,5,132,35,32,58,130,53,32,163,132,35,32,57,134,35,32,50,130,35,34,166,5,199,69,241,
    5,132,35,32,37,130,17,34,170,5,203,69,241,5,55,254,246,0,2,0,108,255,236,4,79,4,100,0,15,0,27,0,16,183,22,8,7,114,16,90,217,12,66,31,6,38,30,2,21,16,2,39,50,68,147,6,8,57,21,20,22,
    2,90,165,220,109,115,223,162,124,185,124,62,251,247,149,141,144,148,146,144,143,20,144,1,2,172,174,255,141,81,150,210,129,254,250,254,200,166,205,203,203,200,201,202,198,210,0,0,1,
    0,34,0,0,2,136,130,105,41,13,0,18,183,11,10,10,6,12,6,91,188,5,38,50,17,51,48,49,97,35,69,217,11,58,2,136,204,2,5,2,20,62,39,200,98,1,187,171,2,128,52,100,90,36,21,55,28,148,134,1,
    68,131,73,36,84,0,0,4,10,130,73,48,29,0,21,64,9,11,18,7,114,27,2,28,28,1,0,47,51,69,213,6,34,97,33,53,69,214,5,69,215,5,33,54,54,69,215,8,8,56,5,23,33,4,10,252,74,1,139,106,127,57,
    53,104,78,87,170,82,103,104,230,140,127,182,98,71,138,101,254,247,2,2,122,159,1,19,74,100,93,62,59,84,45,67,68,138,89,81,79,147,102,93,141,123,69,187,7,130,191,57,60,254,159,3,229,
    4,106,0,45,0,29,64,13,5,4,29,29,26,26,12,36,43,7,114,19,12,130,121,33,43,50,24,81,154,9,33,20,6,69,216,18,36,53,52,38,38,35,24,83,3,13,132,151,8,107,3,184,68,132,95,172,168,131,240,
    167,125,187,87,78,197,102,169,177,91,174,125,123,125,101,160,94,143,107,101,154,87,89,90,225,133,196,239,3,11,92,142,94,22,7,20,171,153,136,192,103,42,41,176,40,55,144,133,84,107,51,
    164,52,113,92,103,109,56,59,138,72,72,185,0,0,2,0,46,254,167,4,111,4,100,0,10,0,21,0,30,64,14,17,16,7,6,114,6,11,11,9,9,4,2,1,10,0,63,205,76,148,5,48,43,50,50,48,49,101,35,17,35,17,
    33,53,1,51,17,51,33,69,216,9,8,51,4,111,217,197,253,93,2,162,198,217,254,98,2,4,4,8,17,62,42,254,154,34,254,133,1,123,132,3,190,252,95,1,132,60,107,103,51,33,96,63,253,251,0,1,0,117,
    254,158,4,30,4,78,69,215,16,35,6,114,15,8,65,17,8,69,215,41,8,57,57,146,218,121,131,248,174,117,195,72,93,187,100,108,157,86,176,164,61,139,66,87,55,2,238,253,196,36,59,109,2,33,96,
    189,141,153,211,109,43,40,179,46,52,65,131,99,136,143,18,20,50,2,194,176,254,106,12,13,130,249,38,111,255,236,4,63,5,206,130,139,50,48,0,31,64,15,16,17,40,40,21,21,5,34,29,13,114,12,
    5,5,72,191,5,34,18,57,47,70,211,5,37,83,52,18,54,54,23,69,220,31,8,104,35,38,6,6,23,20,30,2,111,92,176,254,162,46,94,47,42,96,56,151,213,117,9,11,41,107,130,73,136,191,100,112,211,
    149,126,189,126,63,1,244,130,146,136,131,80,138,85,1,39,74,109,2,109,221,1,70,213,105,1,11,10,164,11,11,128,254,253,198,62,86,44,107,203,143,150,223,123,94,172,235,254,177,168,161,
    138,155,2,74,119,68,65,128,106,64,0,0,1,0,42,254,178,4,3,4,78,69,221,5,36,5,2,2,3,6,104,232,5,35,47,48,49,83,69,221,5,40,237,2,54,253,7,3,217,253,195,130,41,40,238,174,126,250,226,
    0,3,0,95,130,233,56,49,5,203,0,31,0,47,0,61,0,26,64,14,44,24,8,55,4,0,36,16,13,114,48,0,134,234,34,17,23,57,71,74,9,69,211,6,34,35,34,38,103,99,11,34,3,20,22,105,207,5,36,52,38,38,
    39,14,69,32,6,8,57,22,23,62,2,53,52,38,2,73,127,200,115,75,127,78,90,148,88,121,218,146,158,220,115,84,138,82,71,117,69,117,202,172,65,132,98,95,131,69,66,142,112,85,120,65,1,38,108,
    137,67,114,71,67,108,65,137,5,203,69,213,13,69,241,13,54,251,172,69,108,61,60,109,73,67,101,89,46,36,87,111,3,108,104,97,70,97,71,30,69,232,5,43,0,2,0,92,254,160,4,48,4,100,0,32,130,
    209,55,27,64,12,17,39,39,21,21,5,33,28,7,114,12,5,0,47,51,43,50,17,57,47,51,69,227,5,32,2,24,95,115,12,69,228,6,70,191,5,69,227,16,8,92,48,88,173,254,254,171,57,100,43,43,102,45,156,
    223,126,9,8,39,103,136,89,198,221,113,212,147,119,189,131,69,254,8,132,145,138,131,80,139,85,37,74,111,1,217,216,254,200,200,97,11,9,166,12,13,121,254,195,65,88,43,236,212,149,221,
    123,84,165,243,1,69,170,151,141,152,1,67,118,75,65,126,103,61,0,0,3,0,96,255,236,4,50,5,205,130,9,51,20,0,36,0,26,64,14,0,1,3,2,4,9,33,17,5,114,25,9,13,66,104,6,8,33,23,57,48,49,65,
    39,1,23,19,20,2,6,6,35,34,38,2,53,52,18,54,51,50,22,18,5,20,18,22,51,50,54,18,53,75,48,6,8,66,1,46,116,2,152,116,108,54,117,187,132,166,216,106,96,215,177,168,217,105,252,248,56,126,
    104,104,127,56,56,125,106,106,125,55,1,100,117,2,150,115,254,225,178,254,232,194,101,178,1,81,238,234,1,81,181,179,254,175,236,195,254,252,129,128,1,4,196,75,59,7,35,255,255,0,69,130,
    155,40,40,4,100,4,6,4,68,217,0,130,15,36,133,0,0,2,235,132,15,33,69,99,131,15,36,93,0,0,4,18,132,15,33,70,9,131,15,43,71,254,159,3,240,4,106,4,6,4,71,12,131,15,36,14,254,167,4,79,132,
    31,33,72,224,131,15,38,98,254,158,4,11,4,78,130,31,33,73,237,131,15,32,85,130,95,34,38,5,206,130,15,33,74,231,131,15,36,67,254,178,4,28,132,31,33,75,25,131,15,32,77,130,31,34,31,5,
    203,130,31,32,76,132,47,36,68,254,160,4,25,132,79,33,77,233,131,31,38,41,254,229,2,174,2,117,71,29,5,33,251,162,130,161,38,80,254,246,1,251,2,99,71,29,5,132,17,38,51,254,246,2,134,
    2,118,71,29,5,132,17,36,43,254,231,2,151,131,53,35,0,117,0,0,132,17,32,19,130,35,34,197,2,105,71,29,5,132,17,32,68,130,89,32,155,131,71,33,2,56,134,35,38,44,254,230,2,176,2,116,130,
    107,32,120,134,17,32,58,130,53,32,163,132,35,32,57,134,17,32,50,130,35,34,166,2,115,71,29,5,132,71,32,37,130,17,34,170,2,119,71,29,5,49,251,162,0,1,0,77,4,105,2,71,5,12,0,3,0,8,177,
    1,79,103,6,42,53,33,21,77,1,250,4,105,163,163,0,130,33,40,79,1,214,1,151,6,38,0,13,130,33,8,66,11,3,0,47,196,48,49,83,52,18,55,51,6,2,21,20,18,23,35,38,2,79,95,93,140,95,97,95,97,140,
    90,98,3,254,170,1,20,106,111,254,227,156,152,254,225,113,100,1,26,255,255,0,79,254,100,1,151,2,181,6,7,4,100,0,0,252,142,130,81,32,60,130,81,32,133,134,81,8,50,4,10,0,47,198,48,49,
    65,20,2,7,35,54,18,53,52,2,39,51,22,18,1,133,97,90,142,99,96,98,96,141,92,95,4,1,170,254,234,107,112,1,34,152,156,1,27,111,106,254,230,0,130,83,32,60,130,83,32,133,132,83,32,102,134,
    83,46,72,2,136,2,115,4,186,0,11,0,18,182,7,5,4,130,86,35,1,0,47,51,75,228,6,55,65,53,35,53,51,53,51,21,51,21,35,21,1,36,220,220,115,220,220,2,136,226,110,226,130,2,38,0,0,2,0,72,2,
    247,130,59,48,73,0,3,0,7,0,12,179,4,5,1,0,0,47,50,206,50,130,231,35,53,33,21,5,130,3,45,72,2,43,253,213,2,43,3,220,109,109,229,110,110,130,127,38,72,255,26,2,115,1,76,130,211,32,104,
    130,127,32,146,132,17,36,137,2,115,0,219,130,17,32,105,134,17,32,19,130,90,39,244,5,182,6,6,0,18,0,130,10,61,0,174,0,0,5,229,5,182,0,15,0,31,0,37,64,17,16,16,14,14,25,1,4,114,8,8,18,
    18,31,31,77,179,7,49,17,51,124,47,43,50,50,17,51,125,47,48,49,115,17,33,50,22,106,156,6,37,38,35,33,17,19,51,130,16,34,54,54,53,115,170,5,8,101,35,33,174,1,237,156,200,94,181,68,130,
    95,254,226,198,182,1,24,102,138,70,182,96,209,170,254,33,5,182,118,214,142,253,153,2,101,109,143,71,250,227,4,66,252,87,72,146,108,3,215,252,39,137,216,124,0,2,0,102,2,217,5,201,5,
    198,0,20,0,61,0,51,64,28,3,31,58,34,54,51,38,15,11,9,41,7,7,14,14,0,0,28,21,41,192,4,1,1,48,41,3,114,0,130,140,40,47,51,26,16,204,50,50,47,51,130,1,69,120,5,24,176,145,19,32,5,113,
    109,9,32,52,105,203,19,109,9,6,8,63,2,210,183,197,202,177,125,5,1,8,210,103,200,8,3,3,253,243,61,114,37,44,114,60,76,85,76,83,56,99,61,137,122,59,109,44,31,39,93,53,67,69,80,86,71,
    93,46,69,129,2,229,2,209,253,206,2,50,253,47,1,152,29,95,30,131,10,8,38,34,86,25,254,95,12,22,18,108,18,31,53,52,49,51,31,22,51,81,65,93,102,24,19,99,20,25,50,48,53,47,32,25,55,75,
    58,68,94,47,88,243,6,42,116,4,78,6,6,3,175,0,0,255,255,114,43,7,130,15,8,62,176,0,0,0,1,1,111,254,59,2,128,255,131,0,11,0,14,180,1,7,5,128,11,0,47,26,205,57,57,48,49,69,21,14,2,7,35,
    53,62,2,55,2,128,12,49,64,36,112,14,32,28,5,125,18,40,109,113,48,25,35,109,115,44,130,91,36,61,254,52,1,155,130,91,32,38,131,91,36,0,6,1,80,235,130,97,37,0,162,254,141,1,139,136,21,
    40,7,4,14,3,128,0,0,0,0,5,250,48,190,161,22
};

static const char* GetDefaultCompressedFontDataTTF(int* out_size)
{
    *out_size = open_sans_medium_ttf_compressed_size;
    return (const char*)open_sans_medium_ttf_compressed_data;
}
#endif // #ifndef IMGUI_DISABLE_DEFAULT_FONT

#endif // #ifndef IMGUI_DISABLE
