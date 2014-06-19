cmake_minimum_required(VERSION 3.0)

find_package(Boost)

if (Boost_FOUND)
   include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${Boost_INCLUDE_DIR})
   add_executable(hand_rolled_test basic_test.cpp)
   add_executable(small_buffer_test small_buffer_test.cpp)
else ()
   message("    Skipping hand-rolled tests.")
endif ()
