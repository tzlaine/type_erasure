cmake_minimum_required(VERSION 3.0)

set(clang_root
    /usr/local CACHE PATH
    "The root of the Clang installation paths; must contain /lib and /include subdirectories")

set(clang_include_dir ${clang_root}/include)
set(clang_lib_dir ${clang_root}/lib)

include_directories(${clang_include_dir})
link_directories(${clang_lib_dir})

add_executable(libclang_generator libclang_generator.cpp)
target_link_libraries(libclang_generator PRIVATE clang)
