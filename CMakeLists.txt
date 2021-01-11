

cmake_minimum_required(VERSION 3.5)

project(mandelbrot LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_LIST_DIR})

add_executable(mandelbrot
    src/main.cpp
    src/test.cpp
    src/test.h
    src/fractalview.cpp
    src/fractalview.h
    )

include(${CMAKE_CURRENT_LIST_DIR}/e172/e172.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/sdl_implementation/sdl_implementation.cmake)
target_link_libraries(mandelbrot
    e172
    sdl_impl
)
