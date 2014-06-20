cmake_minimum_required(VERSION 3.0)

if (Boost_FOUND)
   include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                       ${CMAKE_SOURCE_DIR}
                       ${Boost_INCLUDE_DIR})
   add_executable(boost_type_erasure boost_type_erasure.cpp)
   message("-- Configuring tests of Boost.TypeErasure implementation")
else ()
   message("-- Skipping tests of Boost.TypeErasure implementation (due to lack of Boost)")
endif ()
