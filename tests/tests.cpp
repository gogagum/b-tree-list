//
// Created by gogagum on 13.07.2020.
//

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../lib/b_tree_list.hpp"

TEST(constructor_tests, no_arguments) {
  BTreeList<int> test_list;
  EXPECT_EQ(test_list.Size(), 0);
}