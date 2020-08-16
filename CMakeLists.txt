cmake_minimum_required(VERSION 3.16)
project(b_tree_list)

set(CMAKE_CXX_STANDARD 20)

set(Boost_USE_STATIC_LIBS        OFF) # only find static libs

find_package(Boost 1.73.0 COMPONENTS system iostreams REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory(lib/googletest)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${gmock_SOURCE_DIR}/include ${gmock_SOURCE_DIR})

add_executable(b_tree_list tests/main.cpp tests/tests.cpp lib/b_tree_list.hpp lib/node.hpp lib/file_saving_manager.hpp lib/allocator.hpp lib/block_rw.hpp lib/data_info.hpp)
target_link_libraries(b_tree_list gtest gtest_main ${Boost_LIBRARIES})


project(b_tree_list_stress_test)

set(CMAKE_CXX_STANDARD 20)

add_executable(b_tree_list_stress_test stress_tests/main.cpp lib/b_tree_list.hpp lib/node.hpp lib/file_saving_manager.hpp lib/allocator.hpp lib/block_rw.hpp lib/data_info.hpp)
target_link_libraries(b_tree_list_stress_test ${Boost_LIBRARIES})