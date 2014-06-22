cmake_minimum_required(VERSION 3.0)

project(type_erasure CXX)

find_package(Boost)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER MATCHES clang OR UNIX OR MINGW OR MSYS OR CYGWIN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11")
endif ()

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(hand_rolled)
add_subdirectory(boost_type_erasure)
add_subdirectory(libclang_generator)
add_subdirectory(presentation)
