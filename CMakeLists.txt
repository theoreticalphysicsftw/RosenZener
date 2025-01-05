cmake_minimum_required(VERSION 3.10) 

project(RosenZener)
enable_language(CXX)
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
