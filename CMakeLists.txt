if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the SDL source code and call cmake from there")
endif()

cmake_minimum_required(VERSION 3.10)

# set the project name
project(Game VERSION 0.1)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)


# cmake path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

# config generation
configure_file(GameConfig.h.in GameConfig.h)

# get the sources
file(GLOB SOURCES "*.cpp")


# SDL2: using the sdl2-cmake-scripts
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)

include_directories(${SDL2_INCLUDE_DIR}
                    ${SDL2_IMAGE_INCLUDE_DIR}
                    ${SDL2_TTF_INCLUDE_DIR})


add_executable(Game "${SOURCES}")


target_link_libraries(Game ${SDL2_LIBRARY}
                           ${SDL2_IMAGE_LIBRARIES}
                           ${SDL2_TTF_LIBRARIES})

# for generated files
target_include_directories(Game PUBLIC "${PROJECT_BINARY_DIR}")
