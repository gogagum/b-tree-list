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
  auto* test_list = new BTreeList<int, 200>("no_arguments_test_data");
  EXPECT_EQ(test_list->Size(), 0);
  delete test_list;
  EXPECT_EQ(std::filesystem::remove("no_arguments_test_data"), true);
}

TEST(constrector_tests, size_argument) {
  auto test_list = new BTreeList<int, 200>("size_argument_test_data", 5);
  EXPECT_EQ(test_list->Size(), 5);

  delete test_list;
  EXPECT_EQ(std::filesystem::remove("size_argument_test_data"), true);
}

////////////////////////////////////////////////////////////////////////////////
// Insert tests                                                               //
////////////////////////////////////////////////////////////////////////////////

TEST(simple_tests, insert_test) {
  auto* test_list = new BTreeList<int>("simple_insert_test_data");
  test_list->Insert(0, 3);
  EXPECT_EQ(test_list->Size(), 1);
  EXPECT_EQ((*test_list)[0], 3);

  delete test_list;
  EXPECT_EQ(std::filesystem::remove("simple_insert_test_data"), true);
}

TEST(simple_tests, two_inserts) {
  auto* test_list = new BTreeList<int>("two_inserts_test_data");
  test_list->Insert(0, 4);
  test_list->Insert(1, 2);
  EXPECT_EQ(test_list->Size(), 2);
  EXPECT_EQ((*test_list)[1], 2);

  delete test_list;
  EXPECT_EQ(std::filesystem::remove("two_inserts_test_data"), true);
}

TEST(simple_tests, insert_plus_set) {
  auto* test_list = new BTreeList<int>("insert_plus_set_data");
  test_list->Insert(0, 3);
  test_list->Insert(0, 2);
  test_list->Insert(0, 0);
  (*test_list)[0] = 1;
  EXPECT_EQ(test_list->Size(), 3);
  EXPECT_EQ((*test_list)[0], 1);
  EXPECT_EQ((*test_list)[1], 2);
  EXPECT_EQ((*test_list)[2], 3);

  delete test_list;

  EXPECT_EQ(std::filesystem::remove("insert_plus_set_data"), true);
}

TEST(simple_tests, simple_extract) {
  auto* test_list = new BTreeList<int>("simple_extract_data");
  for (int i = 0; i < 5; ++i) {
    test_list->Insert(0, i);
  }
  EXPECT_EQ((*test_list)[0], 4);
  auto extracted = test_list->Extract(0);
  EXPECT_EQ(test_list->Size(), 4);
  EXPECT_EQ(extracted, 4);

  EXPECT_EQ(std::filesystem::remove("simple_extract_data"), true);
}

TEST(simple_tests, square_brackets) {
  auto* test_list = new BTreeList<int>("square_brackets_test_data");
  test_list->Insert(0, 21);
  EXPECT_EQ((*test_list)[0], 21);
  (*test_list)[0] = 12;
  EXPECT_EQ((*test_list)[0], 12);

  EXPECT_EQ(std::filesystem::remove("square_brackets_test_data"), true);
}

TEST(not_simple_tests, inserts) {
  auto* test_list = new BTreeList<int, 2>("inserts_test_data", 0);
  for (int i = 0; i < 16; ++i) {
    test_list->Insert(i, i + 1);
  }
  for (int i = 0; i < 16; ++i) {
    EXPECT_EQ((*test_list)[i], i + 1);
  }

  delete test_list;
  EXPECT_EQ(std::filesystem::remove("inserts_test_data"), true);
}

TEST(simple_tests, restore_from_file) {
  std::string file_name = "restore_from_file_test_data";
  auto* test_list = new BTreeList<int, 200>(file_name);
  test_list->Insert(0, 3);
  test_list->Insert(1, 4);
  delete test_list;
  test_list = new BTreeList<int, 200>(file_name);
  EXPECT_EQ(test_list->Size(), 2);
  EXPECT_EQ((*test_list)[0], 3);
  EXPECT_EQ((*test_list)[1], 4);
  delete test_list;

  EXPECT_EQ(std::filesystem::remove(file_name), true);
}

TEST(simple_tests, insert_from_iterators) {
  std::string file_name = "insert_from_iterators_data";
  auto *test_list = new BTreeList<int, 200>(file_name);
  std::vector<int> vector_to_insert = {1, 2, 3, 4, 5};
  test_list->Insert(0, vector_to_insert.begin(), vector_to_insert.end());
  EXPECT_EQ(test_list->Size(), 5);
  EXPECT_EQ((*test_list)[2], 3);

  delete test_list;

  EXPECT_EQ(std::filesystem::remove(file_name), true);
}

TEST(not_simple_tests, many_elements) {
  std::vector<int> elements = {2, 35, 567, 2, 3, 2, 5, 7, 2, 3, 56, 8, 8, 5, 3, 2, 2, 4, 6, 89, 0, 4, 3, 2, 2, 356, 678, 0, 0, 54};
  auto* test_list = new BTreeList<int, 10>("many_elements_test_data");
  for (auto i: elements) {
    test_list->Insert(test_list->Size(), i);
  }
  for (unsigned i = 0; i < elements.size(); ++i) {
    EXPECT_EQ(elements[i], (*test_list)[i]);
  }

  EXPECT_EQ(std::filesystem::remove("many_elements_test_data"), true);
}

TEST(not_simple_tests, many_extracts) {
  std::vector<int> elements;
  auto* test_list = new BTreeList<int, 20>("many_extracts_test_data");
  for (unsigned i = 0; i < 200; ++i) {
    elements.push_back(static_cast<int>(i));
    test_list->Insert(test_list->Size(), static_cast<int>(i));
  }

  for (unsigned i = 0; i < 100; ++i) {
    elements.erase(elements.begin() + i);
    test_list->Extract(i);
  }

  EXPECT_EQ(test_list->Size(), 100);

  for (unsigned i = 0; i < 100; ++i) {
    EXPECT_EQ((*test_list)[i], elements[i]);
  }

  delete test_list;
  EXPECT_EQ(std::filesystem::remove("many_extracts_test_data"), true);
}

TEST(not_simple_extracts, random_extracts) {
  std::vector<int> elements = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
  std::string filename = "random_extracts_test_data";
  auto* test_list = new BTreeList<int, 3>(filename);
  test_list->Insert(0, elements.begin(), elements.end());
  EXPECT_EQ(test_list->Extract(5), 6);  // 1, 2, 3, 4, 5, 7, 8, 9, 10, 11, 12, 13
  EXPECT_EQ(test_list->Extract(0), 1);  // 2, 3, 4, 5, 7, 8, 9, 10, 11, 12, 13
  EXPECT_EQ(test_list->Extract(6), 9);  // 2, 3, 4, 5, 7, 8, 10, 11, 12, 13
  EXPECT_EQ(test_list->Extract(0), 2);  // 3, 4, 5, 7, 8, 10, 11, 12, 13
  EXPECT_EQ(test_list->Extract(1), 4);  // 3, 5, 7, 8, 10, 11, 12, 13
  EXPECT_EQ(test_list->Extract(1), 5);  // 3, 7, 8, 10, 11, 12, 13
  EXPECT_EQ(test_list->Extract(0), 3);  // 7, 8, 10, 11, 12, 13
  EXPECT_EQ(test_list->Extract(0), 7);  // 8, 10, 11, 12, 13
  EXPECT_EQ(test_list->Extract(2), 11);  // 8, 10, 12, 13
  EXPECT_EQ(test_list->Extract(2), 12);  // 8, 10, 13
  EXPECT_EQ(test_list->Extract(0), 8);  // 10, 13
  EXPECT_EQ(test_list->Extract(0), 10);  // 10
  EXPECT_EQ(test_list->Size(), 1);

  EXPECT_EQ(std::filesystem::remove(filename), true);
}