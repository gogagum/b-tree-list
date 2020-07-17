cmake_minimum_required(VERSION 3.16)
project(b_tree_list)

set(CMAKE_CXX_STANDARD 20)
add_subdirectory(lib/googletest)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${gmock_SOURCE_DIR}/include ${gmock_SOURCE_DIR})

add_executable(b_tree_list main.cpp tests/tests.cpp lib/b_tree_list.hpp lib/_node.hpp lib/_file_saving_manager.hpp lib/_allocator.hpp lib/_block_rw.hpp)
target_link_libraries(b_tree_list gtest gtest_main)
