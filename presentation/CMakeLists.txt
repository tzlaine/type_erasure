cmake_minimum_required(VERSION 3.0)

add_subdirectory(code_samples)
add_subdirectory(code_sampler)

file(GLOB filenames ${CMAKE_CURRENT_SOURCE_DIR}/code_samples/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/code_samples/*.hpp)

add_custom_command(
    OUTPUT
        index.html
    COMMAND
        code_sampler
        ${CMAKE_CURRENT_SOURCE_DIR}/index.html.in
        ${CMAKE_SOURCE_DIR}/index.html
        ${filenames}
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/index.html.in
        ${filenames}
    COMMENT
        "Generating index.html from index.html.in and source code samples."
)

add_custom_target(gen_index_html ALL DEPENDS index.html)
