file(
        GLOB
        USD_SOURCES
        "pxr/base/arch/*.cpp"
        "pxr/base/vt/*.cpp"
        "pxr/base/gf/*.cpp"
        "pxr/base/tf/*.cpp"
        "pxr/base/ts/*.cpp"
        "pxr/base/js/*.cpp"
   )

add_library(OpenUSD STATIC "${USD_SOURCES}")
target_include_directories(OpenUSD PUBLIC ".")

if (MSVC)
    target_compile_options(OpenUSD PRIVATE "/W0")
else()
    target_compile_options(OpenUSD PRIVATE "-w")
endif()

