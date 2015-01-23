set(CLANG_ROOT
    /usr/local CACHE PATH
    "The root of the Clang installation paths; must contain /lib and /include subdirectories")

set(clang_include_dir ${CLANG_ROOT}/include)
set(clang_lib_dir ${CLANG_ROOT}/lib)
set(clang_bin_dir ${CLANG_ROOT}/bin)

if (EXISTS ${clang_include_dir}/clang-c/Index.h)
    message("-- Configuring emtypen")
else ()
    message("-- Skipping emtypen build (due to lack of libclang headers)")
    return()
endif ()

include_directories(${clang_include_dir})
link_directories(${clang_lib_dir})

if (UNIX)
    set(install_bin_dir bin)
    set(install_lib_dir lib)
    set(install_data_dir share/emtypen)
    set(libclang_name clang)
    add_definitions(-DRELATIVE_DATA_DIR=\"../${install_data_dir}/\")
else ()
    set(install_bin_dir .)
    set(install_lib_dir .)
    set(install_data_dir .)
    set(libclang_name libclang)
    add_definitions(-DRELATIVE_DATA_DIR=\"\")
endif ()

add_executable(emtypen emtypen.cpp)
target_link_libraries(emtypen ${libclang_name})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/headers.hpp
               ${CMAKE_BINARY_DIR}/${install_data_dir}/headers.hpp)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/form.hpp
               ${CMAKE_BINARY_DIR}/${install_data_dir}/form.hpp)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_archetypes.hpp
               ${CMAKE_BINARY_DIR})

if (APPLE)
    if (true)
        set(CPACK_GENERATOR STGZ) 
    else ()
        set(CPACK_GENERATOR DragNDrop) 
        set(drag_and_drop_prefix emtypen/) 
    endif ()
elseif (CMAKE_HOST_WIN32)
    set(CPACK_GENERATOR NSIS)
else ()
    set(CPACK_GENERATOR STGZ)
endif ()

if (UNIX)
    set(clang_dll_dir ${clang_lib_dir})
else ()
    set(clang_dll_dir ${clang_bin_dir})
endif ()

install(TARGETS emtypen
        RUNTIME DESTINATION ${drag_and_drop_prefix}${install_bin_dir})
install(FILES ${clang_dll_dir}/${CMAKE_SHARED_LIBRARY_PREFIX}${libclang_name}${CMAKE_SHARED_LIBRARY_SUFFIX}
        DESTINATION ${drag_and_drop_prefix}${install_lib_dir})
install(FILES headers.hpp form.hpp
        DESTINATION ${drag_and_drop_prefix}${install_data_dir})

set(CPACK_PACKAGE_VERSION 1.0.0) 
set(CPACK_PACKAGE_VENDOR "T. Zachary Laine")
set(CPACK_PACKAGE_CONTACT ${CPACK_PACKAGE_VENDOR})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A tool to generate type-erased types from archetypes.")
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_HOME_DIRECTORY}/COPYING)
set(CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})
set(CPACK_PACKAGE_NAME emtypen)
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}")
string(TOLOWER ${CPACK_PACKAGE_FILE_NAME} CPACK_PACKAGE_FILE_NAME)

include(CPack)
