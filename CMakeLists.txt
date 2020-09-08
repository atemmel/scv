cmake_minimum_required(VERSION 3.10)
project(scv)

# Set a default build type if none was specified
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}")
endif()

file(GLOB_RECURSE sources RELATIVE ${CMAKE_SOURCE_DIR} "src/*.cpp")
add_executable(scv ${sources})
set_property(TARGET scv PROPERTY CXX_STANDARD 17)
include_directories(./include)

add_compile_options(-Wall -Wextra -Wpedantic)
