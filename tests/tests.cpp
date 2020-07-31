//
// Created by gogagum on 13.07.2020.
//

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../lib/b_tree_list.hpp"

////////////////////////////////////////////////////////////////////////////////
//  Constructor tests                                                         //
////////////////////////////////////////////////////////////////////////////////

TEST(constructor_tests, no_arguments) {
  BTreeList<int> test_list;
  EXPECT_EQ(test_list.Size(), 0);
}

TEST(constrector_tests, size_argument) {
  BTreeList<int> test_list(5);
  EXPECT_EQ(test_list.Size(), 5);
}

////////////////////////////////////////////////////////////////////////////////
// Insert tests                                                               //
////////////////////////////////////////////////////////////////////////////////

TEST(simple_tests, insert_test) {
  BTreeList<int> test_list;
  test_list.Insert(0, 3);
  EXPECT_EQ(test_list.Size(), 1);
  EXPECT_EQ(test_list.Get(0), 3);
}

TEST(simple_tests, two_inserts) {
  BTreeList<int> test_list;
  test_list.Insert(0, 4);
  test_list.Insert(1, 2);
  EXPECT_EQ(test_list.Size(), 2);
  EXPECT_EQ(test_list.Get(1), 2);
}

TEST(simple_tests, insert_plus_set) {
  BTreeList<int> test_list;
  test_list.Insert(0, 3);
  test_list.Insert(0, 2);
  test_list.Insert(0, 0);
  test_list.Set(0, 1);
  EXPECT_EQ(test_list.Size(), 3);
  EXPECT_EQ(test_list.Get(0), 1);
  EXPECT_EQ(test_list.Get(1), 2);
  EXPECT_EQ(test_list.Get(2), 3);
}

TEST(not_simple_test, inserts) {
  BTreeList<int> test_list(0, 256, 2);
  //for (int i = 0; i < 16; ++i) {
  //  test_list.Insert(i, i + 1);
  //  test_list.print_all_nodes();
  //}
  //test_list.print_all_nodes();
  for (int i = 0; i < 16; ++i) {
    EXPECT_EQ(test_list.Get(i), i + 1);
  }
}

