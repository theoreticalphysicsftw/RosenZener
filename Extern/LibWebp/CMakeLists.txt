cmake_minimum_required(VERSION 3.5) 

project(LibWebp)

file(
        GLOB
        LIBWEBP_SOURCES
        "src/enc/*.c"
        "/dec/*.c"
        "src/dsp/*.c"
        "src/mux/*.c"
        "src/demux/*.c"
        "src/utils/*.c"
        "sharpyuv/*.c"
    )

add_library(${PROJECT_NAME} ${LIBWEBP_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ".")
