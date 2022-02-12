cmake_minimum_required(VERSION 3.5)

project(mandelbrot LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#conan section
#message("Running conan...")
#execute_process(COMMAND conan install --build=missing ${CMAKE_CURRENT_LIST_DIR} WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
#include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
#conan_basic_setup()
#conan section end

find_package(OpenCL REQUIRED)

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

target_link_libraries(mandelbrot ${CONAN_LIBS} ${OpenCL_LIBRARY})
