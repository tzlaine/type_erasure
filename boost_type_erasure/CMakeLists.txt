cmake_minimum_required(VERSION 3.0)

# As of Boost 1.55.0, Boost.TypeErasure does not support VS 2013.
if (Boost_FOUND)
   if (NOT MSVC12)
      include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                          ${Boost_INCLUDE_DIR})
      add_executable(boost_type_erasure boost_type_erasure.cpp)
      message("-- Configuring tests of Boost.TypeErasure implementation")
   else ()
      message("-- Skipping tests of Boost.TypeErasure implementation (due to lack of support for VS 2013)")
   endif ()
else ()
   message("-- Skipping tests of Boost.TypeErasure implementation (due to lack of Boost)")
endif ()
