cmake_minimum_required(VERSION 3.0)

message("-- Configuring libclang_generator")

set(CLANG_ROOT
    /usr/local CACHE PATH
    "The root of the Clang installation paths; must contain /lib and /include subdirectories")

set(clang_include_dir ${CLANG_ROOT}/include)
set(clang_lib_dir ${CLANG_ROOT}/lib)

include_directories(${clang_include_dir})
link_directories(${clang_lib_dir})

if (UNIX)
    set(install_bin_dir bin)
    set(install_lib_dir lib)
    set(install_data_dir share/libclang_generator)
    set(libclang_name clang)
    add_definitions(-DRELATIVE_DATA_DIR=\"../${install_data_dir}/\")
else ()
    set(install_bin_dir .)
    set(install_lib_dir .)
    set(install_data_dir .)
    set(libclang_name libclang)
    add_definitions(-DRELATIVE_DATA_DIR=\"\")
endif ()

add_executable(libclang_generator libclang_generator.cpp)
target_link_libraries(libclang_generator PRIVATE ${libclang_name})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/headers.hpp
               ${CMAKE_BINARY_DIR}/${install_data_dir}/headers.hpp)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/form.hpp
               ${CMAKE_BINARY_DIR}/${install_data_dir}/form.hpp)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_archetypes.hpp
               ${CMAKE_BINARY_DIR})

install(TARGETS libclang_generator
        RUNTIME DESTINATION ${install_bin_dir})
install(FILES ${clang_lib_dir}/${CMAKE_SHARED_LIBRARY_PREFIX}clang${CMAKE_SHARED_LIBRARY_SUFFIX}
        DESTINATION ${install_lib_dir})
install(FILES headers.hpp form.hpp
        DESTINATION ${install_data_dir})

if (CMAKE_HOST_WIN32)
    set(CPACK_PACKAGE_VERSION 1.0.0)
    set(CPACK_PACKAGE_VENDOR "T. Zachary Laine")
    set(CPACK_PACKAGE_CONTACT ${CPACK_PACKAGE_VENDOR})
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A tool to generate type-erased types from archetypes.")
    set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_HOME_DIRECTORY}/COPYING)
    set(CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})
    set(CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}")
    string(TOLOWER ${CPACK_PACKAGE_FILE_NAME} CPACK_PACKAGE_FILE_NAME)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_SOURCE_GENERATOR "ZIP")
endif ()
