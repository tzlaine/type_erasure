cmake_minimum_required(VERSION 3.0)

file(GLOB filenames ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

foreach (filename ${filenames})
    get_filename_component(basename ${filename} NAME_WE)
    add_executable(code_sample_${basename} ${filename})
endforeach ()
