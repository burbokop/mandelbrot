cmake_minimum_required(VERSION 3.8)

project(mandelbrot LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message("CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")


include_directories(${CMAKE_CURRENT_LIST_DIR})

add_executable(mandelbrot
    src/main.cpp
    src/test.cpp
    src/test.h
    src/fractalview.cpp
    src/fractalview.h
    )

find_package(Boost REQUIRED)
find_package(OpenCL REQUIRED)

include(${CMAKE_CURRENT_LIST_DIR}/e172/e172.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/sdl_implementation/sdl_implementation.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/console_impl/console_impl.cmake)

target_include_directories(mandelbrot PUBLIC ${Boost_INCLUDE_DIR})
target_link_libraries(mandelbrot
    e172
    sdl_impl
    console_impl
    ${Boost_LIBRARIES}
    OpenCL::OpenCL
)

