//
// Created by gogagum on 10.08.2020.
//

#include <algorithm>
#include <fstream>
#include <cstddef>
#include <chrono>
#include <string>
#include <boost/random.hpp>
#include "../lib/b_tree_list.hpp"

#ifndef EXTRACT_STRESS_CPP
#define EXTRACT_STRESS_CPP

auto TestExtracts(size_t size_of_btree, size_t extracts_cnt) {
  std::string file_name = "extracts_stress_test_data";
  auto* test_list = new BTreeList<int, 200>(file_name, size_of_btree, false);

  auto start = std::chrono::high_resolution_clock::now();
  for(unsigned i = 0; i < extracts_cnt; ++i) {
    auto pos =
        boost::minstd_rand(static_cast<unsigned>(std::time(nullptr)))() %
        test_list->Size();
    test_list->Extract(pos);
  }
  auto finish = std::chrono::high_resolution_clock::now();

  delete test_list;
  boost::filesystem::remove(file_name);

  return std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count();
}

void RunExtractsTestPack() {
  const size_t step = 10;
  const size_t max_size = 10000000;

  for (size_t size_of_btree = step; size_of_btree <= max_size; size_of_btree *= step) {
    std::string results_file_name = "extracts_test_" + std::to_string(size_of_btree) + ".txt";
    std::ofstream file;
    file.open(results_file_name, std::ios_base::trunc);

    for (unsigned i = 0; i < 50; ++i) {
      auto extracts_cnt = std::min(size_of_btree / step, static_cast<size_t>(1000));
      file << TestExtracts(size_of_btree, extracts_cnt) / extracts_cnt << std::endl;
    }
    file.close();
  }
}

#endif