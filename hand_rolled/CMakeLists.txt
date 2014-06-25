cmake_minimum_required(VERSION 3.0)

if (Boost_FOUND)
   include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                       ${Boost_INCLUDE_DIR})
   add_executable(hand_rolled_test basic_test.cpp)
   add_executable(hand_rolled_vtable_test vtable_test.cpp)
   add_executable(small_buffer_test small_buffer_test.cpp)
   message("-- Configuring tests of hand-rolled implementation")
else ()
   message("-- Skipping tests of hand-rolled implementation (due to lack of Boost)")
endif ()
