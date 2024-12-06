file(
        GLOB
        IMGUI_SOURCES
        "*.cpp"
        "misc/cpp/*.cpp"
        "backends/*.cpp"
   )

add_library(ImGUI STATIC "${IMGUI_SOURCES}")
target_include_directories(ImGUI PUBLIC ".")
target_include_directories(ImGUI PUBLIC "../SDL/include")

if (MSVC)
    target_compile_options(ImGUI PRIVATE "/W0")
else()
    target_compile_options(ImGUI PRIVATE "-w")
endif()

