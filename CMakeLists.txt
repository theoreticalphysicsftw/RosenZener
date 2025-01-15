cmake_minimum_required(VERSION 3.20) 
cmake_policy(SET CMP0091 NEW)

project(RosenZener)
enable_language(CXX)
enable_language(C)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(RZ_SOURCE_DIR "Src")
set(RZ_EXTERN_DIR "Extern")

file(
        GLOB
        RZ_SOURCES
        "${RZ_SOURCE_DIR}/*.cpp"
        "${RZ_SOURCE_DIR}/OS/*.cpp"
        "${RZ_SOURCE_DIR}/GUI/*.cpp"
        "${RZ_SOURCE_DIR}/Common/*.cpp"
    )
    
add_executable(${PROJECT_NAME} "${RZ_SOURCES}")
target_include_directories(${PROJECT_NAME} PRIVATE "${RZ_SOURCE_DIR}")
target_include_directories(${PROJECT_NAME} PRIVATE "${RZ_EXTERN_DIR}")
target_include_directories(${PROJECT_NAME} PRIVATE "${RZ_EXTERN_DIR}/SDL/include/")

target_compile_definitions(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:DEBUG>")

add_compile_definitions("NOMINMAX")

add_subdirectory("${RZ_EXTERN_DIR}/ImGUI")
add_subdirectory("${RZ_EXTERN_DIR}/SDL")
add_subdirectory("${RZ_EXTERN_DIR}/LibWebp")

target_link_libraries(${PROJECT_NAME} ImGUI)
target_link_libraries(${PROJECT_NAME} SDLStatic)
target_link_libraries(${PROJECT_NAME} LibWebp)

if (CMAKE_HOST_WIN32)
set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY  "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set_property(TARGET ImGUI PROPERTY MSVC_RUNTIME_LIBRARY  "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set_property(TARGET LibWebp PROPERTY MSVC_RUNTIME_LIBRARY  "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set_property(TARGET SDL3-static PROPERTY MSVC_RUNTIME_LIBRARY  "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()