cmake_minimum_required(VERSION 3.0)

set(CLANG_ROOT
    /usr/local CACHE PATH
    "The root of the Clang installation paths; must contain /lib and /include subdirectories")

set(clang_include_dir ${CLANG_ROOT}/include)
set(clang_lib_dir ${CLANG_ROOT}/lib)

include_directories(${clang_include_dir})
link_directories(${clang_lib_dir})

add_executable(libclang_generator libclang_generator.cpp)
target_link_libraries(libclang_generator PRIVATE clang)
