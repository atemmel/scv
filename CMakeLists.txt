cmake_minimum_required(VERSION 3.10)
project(scv LANGUAGES CXX)

# Set a default build type if none was specified
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}")
endif()

file(GLOB_RECURSE sources RELATIVE ${CMAKE_SOURCE_DIR} "src/*.cpp")
add_executable(scv ${sources})
set_property(TARGET scv PROPERTY CXX_STANDARD 17)
include_directories(include)

if(CMAKE_BUILD_TYPE EQUAL "Debug") 
	# AddressSanitizer flags
	set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
	set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
	add_compile_options(-fsanitize=address)
endif()

message(STATUS "!!!!!!!!!!!!!!!!!!!!!!!!!")
message(STATUS "Running ${CMAKE_BUILD_TYPE} build")
message(STATUS "!!!!!!!!!!!!!!!!!!!!!!!!!")

add_compile_options(-Wall -Wextra -Wpedantic -Wpadding -Werror)
