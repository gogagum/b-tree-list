//
// Created by gogagum on 10.08.2020.
//

#include <algorithm>
#include <fstream>
#include <filesystem>
#include <cstddef>
#include <chrono>
#include <string>
#include <boost/random.hpp>
#include "../lib/b_tree_list.hpp"

#ifndef INSERT_STRESS_CPP
#define INSERT_STRESS_CPP

auto TestInserts(size_t inserts_cnt, const std::string& file_name) {
  std::string in_test_filename = file_name + "_copy";
  std::filesystem::copy(file_name, in_test_filename);
  auto* test_list = new BTreeList<int, 200>(in_test_filename, false);

  auto start = std::chrono::high_resolution_clock::now();
  for(unsigned i = 0; i < inserts_cnt; ++i) {
    auto pos = boost::minstd_rand(static_cast<unsigned>(std::time(nullptr)))() %
               (test_list->Size() + 1);
    test_list->Insert(pos, 0);
  }
  auto finish = std::chrono::high_resolution_clock::now();

  delete test_list;
  std::filesystem::remove(in_test_filename);

  return std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count();
}

void RunInsertsTestPack(size_t max_size, size_t operations_cnt) {
  const size_t step = 10;

  for (size_t size_of_btree = step; size_of_btree <= max_size; size_of_btree *= step) {
    std::string results_file_name = "inserts_test_" + std::to_string(size_of_btree) + ".txt";
    std::ofstream file;
    file.open(results_file_name, std::ios_base::trunc);

    std::string filename = "insterts_test_data";
    auto* test_list = new BTreeList<int, 200>(filename, size_of_btree, 0, false);
    delete test_list;

    for (unsigned i = 0; i < 50; ++i) {
      auto inserts_cnt = std::min(size_of_btree / step, static_cast<size_t>(operations_cnt));
      file << TestInserts(inserts_cnt, filename) / inserts_cnt << std::endl;
    }
    std::filesystem::remove(filename);
    file.close();
  }
}

#endif