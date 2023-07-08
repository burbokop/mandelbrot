cmake_minimum_required(VERSION 3.8)

project(mandelbrot LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message("CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")

add_executable(
  mandelbrot
  ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/flags.h
  ${CMAKE_CURRENT_LIST_DIR}/src/flags.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/test.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/test.h
  ${CMAKE_CURRENT_LIST_DIR}/src/fractalview.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/fractalview.h)

find_package(Boost REQUIRED)
find_package(OpenCL REQUIRED)

# DEPENDENCIES_PREFIX
include(ExternalProject)

set(DEPENDENCIES_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/dependencies_prefix)

ExternalProject_Add(
  E172
  GIT_REPOSITORY https://github.com/burbokop/e172.git
  CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
             -DCMAKE_INSTALL_PREFIX=${DEPENDENCIES_PREFIX} -DENABLE_LINT=OFF
             -DENABLE_TESTS=OFF)

ExternalProject_Add(
  E172SdlImpl
  GIT_REPOSITORY https://github.com/burbokop/e172_sdl_impl.git
  CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
             -DCMAKE_INSTALL_PREFIX=${DEPENDENCIES_PREFIX})

ExternalProject_Add(
  E172ConsoleImpl
  GIT_REPOSITORY https://github.com/burbokop/e172_console_impl.git
  CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
             -DCMAKE_INSTALL_PREFIX=${DEPENDENCIES_PREFIX}
             -DENABLE_EXAMPLES=OFF)

ExternalProject_Add(
  E172VulkanImpl
  GIT_REPOSITORY https://github.com/burbokop/e172_vulkan_impl.git
  CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
             -DCMAKE_INSTALL_PREFIX=${DEPENDENCIES_PREFIX})

add_dependencies(E172SdlImpl E172)
add_dependencies(E172ConsoleImpl E172)
add_dependencies(E172VulkanImpl E172)

add_dependencies(mandelbrot E172 E172ConsoleImpl E172SdlImpl E172VulkanImpl)

target_include_directories(mandelbrot PRIVATE ${DEPENDENCIES_PREFIX}/include)
target_link_directories(mandelbrot PRIVATE ${DEPENDENCIES_PREFIX}/lib)

target_include_directories(mandelbrot PUBLIC ${Boost_INCLUDE_DIR})
target_link_libraries(
  mandelbrot
  e172
  e172_console_impl
  e172_sdl_impl
  e172_vulkan_impl
  ${Boost_LIBRARIES}
  OpenCL::OpenCL)

if(UNIX)
  target_link_libraries(mandelbrot tbb)
endif()
