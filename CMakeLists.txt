cmake_minimum_required(VERSION 3.22)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl2)

project(mandelbrot C)

set(CMAKE_C_STANDARD 99)

add_executable(${PROJECT_NAME} mandelbrot.c)

# Find SDL2, SDL2_gfx libraries
find_package(SDL2 REQUIRED)
find_package(SDL2_gfx REQUIRED)

# Link SDL2::Main, SDL2::GFX to our project
target_link_libraries(${PROJECT_NAME} SDL2::Main SDL2::GFX)


