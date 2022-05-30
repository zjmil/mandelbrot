cmake_minimum_required(VERSION 3.22)

project(mandelbrot C)

set(CMAKE_C_STANDARD 99)

add_subdirectory(vendor/sdl2)

add_executable(${PROJECT_NAME} mandelbrot.c)
include_directories(${PROJECT_SOURCE_DIR}/vendor/sdl2/include)
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY})


