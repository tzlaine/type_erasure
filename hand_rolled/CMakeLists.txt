cmake_minimum_required(VERSION 3.0)

if (Boost_FOUND)
   include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                       ${Boost_INCLUDE_DIR})

   add_executable(hand_rolled_test basic_test.cpp)
   add_executable(hand_rolled_cow_test cow_test.cpp)
   add_executable(hand_rolled_vtable_test vtable_test.cpp)
   add_executable(hand_rolled_sbo_test sbo_test.cpp)
   add_executable(hand_rolled_sbo_cow_test sbo_cow_test.cpp)

   add_executable(coverage_hand_rolled coverage_hand_rolled.cpp)
   add_executable(coverage_hand_rolled_cow coverage_hand_rolled_cow.cpp)
   add_executable(coverage_hand_rolled_vtable coverage_hand_rolled_vtable.cpp)
   add_executable(coverage_hand_rolled_sbo coverage_hand_rolled_sbo.cpp)
   add_executable(coverage_hand_rolled_sbo_cow coverage_hand_rolled_sbo_cow.cpp)

   if (!WIN32)
      add_executable(small_buffer_test small_buffer_test.cpp)
   endif ()
   message("-- Configuring tests of hand-rolled implementations")
else ()
   message("-- Skipping tests of hand-rolled implementations (due to lack of Boost)")
endif ()
